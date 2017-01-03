
#include "ssp21/crypto/Session.h"

#include "openpal/logging/LogMacros.h"

#include "ssp21/LogLevels.h"
#include "ssp21/crypto/gen/CryptoError.h"

#include <limits>

using namespace openpal;

namespace ssp21
{

    Session::Session(const std::shared_ptr<IFrameWriter>& frame_writer, const Config& config) :
        frame_writer(frame_writer),
        config(config),
        decrypt_scratch_buffer(calc_max_crypto_payload_length(frame_writer->get_max_payload_size())),
        encrypt_scratch_buffer(decrypt_scratch_buffer.length())
    {}

    uint32_t Session::calc_max_crypto_payload_length(uint32_t max_link_payload_size)
    {
        return (max_link_payload_size > SessionData::min_size_bytes) ? max_link_payload_size - SessionData::min_size_bytes : 0;
    }

    bool Session::initialize(const Algorithms::Session& algorithms, const openpal::Timestamp& session_start, const SessionKeys& keys, uint16_t nonce_start)
    {
        if (!keys.valid()) return false;

        this->statistics.num_init.increment();

        this->valid = true;

        this->rx_nonce.set(nonce_start);
        this->tx_nonce.set(nonce_start);
        this->algorithms = algorithms;
        this->session_start = session_start;
        this->keys.copy(keys);

        return true;
    }

    void Session::reset()
    {
        this->statistics.num_reset.increment();
        this->valid = false;
    }

    seq32_t Session::validate_message(const SessionData& message, const openpal::Timestamp& now, std::error_code& ec)
    {
        if (!this->valid)
        {
            this->statistics.num_user_data_without_session.increment();
            ec = CryptoError::no_valid_session;
            return seq32_t::empty();
        }

        const auto payload = this->algorithms.mode->read(this->keys.rx_key, message, this->decrypt_scratch_buffer.as_wslice(), ec);

        if (ec)
        {
            this->statistics.num_auth_fail.increment();
            return seq32_t::empty();
        }

        if (payload.is_empty())
        {
            ec = CryptoError::empty_user_data;
            this->statistics.num_auth_fail.increment();
            return seq32_t::empty();
        }

        const auto current_session_time = now.milliseconds - this->session_start.milliseconds;

        // the message is authentic, check the TTL
        if (current_session_time > message.metadata.valid_until_ms)
        {
            this->statistics.num_ttl_expiration.increment();
            ec = CryptoError::expired_ttl;
            return seq32_t::empty();
        }

        // check the nonce via the configured maximum
        if (message.metadata.nonce > config.max_nonce)
        {
            this->statistics.num_nonce_fail.increment();
            ec = CryptoError::nonce_config_max;
            return seq32_t::empty();
        }

        // check the nonce via the verification function
        if (!this->algorithms.verify_nonce(this->rx_nonce.get(), message.metadata.nonce))
        {
            this->statistics.num_nonce_fail.increment();
            ec = CryptoError::nonce_replay;
            return seq32_t::empty();
        }

        this->rx_nonce.set(message.metadata.nonce.value);
        this->statistics.num_success.increment();

        return payload;
    }

    seq32_t Session::format_session_message(bool fir, const openpal::Timestamp& now, seq32_t& cleartext, std::error_code& ec)
    {
        const auto ret = this->format_session_message_impl(fir, now, cleartext, ec);
        if (ec)
        {
            // any error invalidates the tx direction of the session
            this->valid = false;
        }
        return ret;
    }

    seq32_t Session::format_session_message_impl(bool fir, const openpal::Timestamp& now, seq32_t& user_data, std::error_code& ec)
    {
        if (!this->valid)
        {
            ec = CryptoError::no_valid_session;
            return seq32_t::empty();
        }

        if (this->tx_nonce.get() == this->config.max_nonce)
        {
            ec = CryptoError::nonce_config_max;
            return seq32_t::empty();
        }

        const auto session_time_long = now.milliseconds - this->session_start.milliseconds;
        if (session_time_long > std::numeric_limits<uint32_t>::max())
        {
            ec = CryptoError::ttl_overflow;
            return seq32_t::empty();
        }

        const auto session_time = static_cast<uint32_t>(session_time_long);
        const auto remainder = std::numeric_limits<uint32_t>::max() - session_time;
        if (remainder < config.ttl_pad_ms)
        {
            ec = CryptoError::ttl_overflow;
            return seq32_t::empty();
        }

        // the metadata we're encoding
        AuthMetadata metadata(
            this->tx_nonce.get() + 1,
            session_time + config.ttl_pad_ms,
            SessionFlags(fir, true) // doesn't matter what FIN is as the write function will determine how much it can transmit
        );

        const auto frame = this->algorithms.mode->write(*this->frame_writer, this->keys.tx_key, metadata, user_data, this->encrypt_scratch_buffer.as_wslice(), ec);
        if (ec)
        {
            return seq32_t::empty();
        }

        // everything succeeded, so increment the nonce and advance the input buffer
        this->tx_nonce.increment();

        return frame;
    }

}


