
#include "ssp21/crypto/Responder.h"

#include "openpal/logging/LogMacros.h"

#include "ssp21/LogLevels.h"

#include "ssp21/crypto/LogMessagePrinter.h"
#include "ssp21/crypto/Crypto.h"

#include "ssp21/msg/ReplyHandshakeError.h"
#include "ssp21/msg/ReplyHandshakeBegin.h"

using namespace openpal;

namespace ssp21
{
    Responder::Responder(
		const Config& config,
		std::unique_ptr<KeyPair> local_static_key_pair,
		std::unique_ptr<PublicKey> remote_static_public_key,
		openpal::IExecutor& executor,
		openpal::Logger logger,
		ILowerLayer& lower) :
        
		config_(config),
		local_static_key_pair_(std::move(local_static_key_pair)),
		remote_static_public_key_(std::move(remote_static_public_key)),
        executor_(&executor),
        logger_(logger),
        lower_(&lower),
        tx_buffer_(config.max_tx_message_size),
		handshake_state_(HandshakeState::wait_for_handshake_begin)
    {

    }

    void Responder::on_open_impl()
    {

    }

    void Responder::on_close_impl()
    {

    }

    void Responder::on_tx_ready_impl()
    {
        // if there's message to be read, read it
        // since we can now transmit responses
        if (lower_->is_rx_ready())
        {
            lower_->receive(*this);
        }
    }

    void Responder::on_rx_ready_impl()
    {
        // only read a message if the lower layer
        // can transmit a response
        if (lower_->is_tx_ready())
        {
            lower_->receive(*this);
        }
    }

    template <class MsgType>
    inline void Responder::read_any(const openpal::RSlice& data)
    {
        MsgType msg;
        auto err = msg.read_msg(data);
        if (any(err))
        {
            FORMAT_LOG_BLOCK(logger_, levels::warn, "error reading %s: %s", FunctionSpec::to_string(MsgType::function), ParseErrorSpec::to_string(err));
            this->handle_parse_error<MsgType>(err);
        }
        else
        {
            this->on_message(data, msg);
        }
    }

    void Responder::process(const Message& message)
    {
		if (message.addresses.destination != this->config_.local_address)
		{
			FORMAT_LOG_BLOCK(logger_, levels::info, "unknown destination address: %u", message.addresses.destination);
			return;
		}

		if (message.addresses.source != this->config_.remote_address)
		{
			FORMAT_LOG_BLOCK(logger_, levels::info, "unknown source address: %u", message.addresses.source);
			return;
		}

        if (message.payload.is_empty())
        {
            SIMPLE_LOG_BLOCK(logger_, levels::warn, "Received zero length message");
            return;
        }

        const auto function = message.payload[0];

        switch (FunctionSpec::from_type(function))
        {
        case(Function::request_handshake_begin) :
            this->read_any<RequestHandshakeBegin>(message.payload);
            break;

        case(Function::request_handshake_auth) :
            this->read_any<RequestHandshakeAuth>(message.payload);
            break;

        case(Function::unconfirmed_session_data) :
            this->read_any<UnconfirmedSessionData>(message.payload);
            break;

        default:
            FORMAT_LOG_BLOCK(logger_, levels::warn, "Received unknown function id: %u", function);
            break;
        }
    }

    void Responder::reply_with_handshake_error(HandshakeError err)
    {
        ReplyHandshakeError msg(err);

        auto dest = this->tx_buffer_.as_wslice();
        auto result = msg.write_msg(dest);

        if (!result.is_error())
        {
            this->lower_->transmit(Message(Addresses(), result.written));
        }
    }

    void Responder::on_message(const RSlice& msg_bytes, const RequestHandshakeBegin& msg)
    {
        FORMAT_LOG_BLOCK(logger_, levels::rx_crypto_msg, "request handshake begin (length = %u)", msg_bytes.length());

        if (logger_.is_enabled(levels::rx_crypto_msg_fields))
        {
            LogMessagePrinter printer(logger_, levels::rx_crypto_msg_fields);
            msg.print(printer);
        }

		auto err = validate_handshake_begin(msg);

		if (any(err))
		{
			FORMAT_LOG_BLOCK(logger_, levels::warn, "handshake error: %s", HandshakeErrorSpec::to_string(err));
			this->reply_with_handshake_error(err);
			return;
		}

		Seq8 public_ephem_dh_key(this->handshake_.initialize());	// generate our local ephemeral keys
		this->handshake_.set_ck(msg_bytes);							// initialize the chaining key

		// now format our response - in the future, this we'll add certificates after this call
		ReplyHandshakeBegin reply(public_ephem_dh_key);

		auto dest = this->tx_buffer_.as_wslice();
		auto result = reply.write_msg(dest);

		if (result.is_error()) {
			FORMAT_LOG_BLOCK(logger_, levels::error, "error formatting reply: %s", FormatErrorSpec::to_string(result.err));
			return;
		}
			
		std::error_code ec;

		this->handshake_.derive_authentication_key(
			result.written,
			this->local_static_key_pair_->private_key,
			msg.ephemeral_public_key,
			this->remote_static_public_key_->as_slice(),
			ec
		);

		if (ec)
		{
			FORMAT_LOG_BLOCK(logger_, levels::error, "error deriving auth key: %s", ec.message().c_str());
			this->reply_with_handshake_error(HandshakeError::internal);
		}
		
		this->lower_->transmit(Message(Addresses(), result.written)); // begin transmitting the response		
    }

	HandshakeError Responder::validate_handshake_begin(const RequestHandshakeBegin& msg)
	{		
		if (msg.version != consts::ssp21_protocol_version)
		{			
			return HandshakeError::unsupported_version;
		}

		// verify that the public key length matches the DH mode
		if (msg.ephemeral_public_key.length() != consts::x25519_key_length)
		{
			return HandshakeError::bad_message_format;
		}		

		if (msg.certificate_mode != CertificateMode::preshared_keys)
		{
			return HandshakeError::unsupported_certificate_mode;
		}

		if (msg.certificates.count() != 0)
		{
			return HandshakeError::bad_message_format;
		}		

		// last thing we should do is configure the requested algorithms
		return this->handshake_.set_algorithms(
			Algorithms::Config(
				msg.dh_mode,
				msg.hash_mode,
				msg.nonce_mode,
				msg.session_mode
			)
		);
	}	

    void Responder::on_message(const RSlice& data, const RequestHandshakeAuth& msg)
    {
        FORMAT_LOG_BLOCK(logger_, levels::rx_crypto_msg, "request handshake auth (length = %u)", data.length());

        if (logger_.is_enabled(levels::rx_crypto_msg_fields))
        {
            LogMessagePrinter printer(logger_, levels::rx_crypto_msg_fields);
            msg.print(printer);
        }
    }

    void Responder::on_message(const RSlice& data, const UnconfirmedSessionData& msg)
    {
        FORMAT_LOG_BLOCK(logger_, levels::rx_crypto_msg, "unconfirmed session data (length = %u)", data.length());

        if (logger_.is_enabled(levels::rx_crypto_msg_fields))
        {
            LogMessagePrinter printer(logger_, levels::rx_crypto_msg_fields);
            msg.print(printer);
        }
    }

}
