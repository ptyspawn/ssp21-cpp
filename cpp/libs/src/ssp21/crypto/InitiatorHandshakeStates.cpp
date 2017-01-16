
#include "ssp21/crypto/InitiatorHandshakeStates.h"

#include "openpal/logging/LogMacros.h"

#include "ssp21/LogLevels.h"

namespace ssp21
{

    // -------- Idle --------

    Initiator::IHandshakeState* InitiatorHandshake::Idle::on_handshake_required(Initiator& ctx, const openpal::Timestamp& now)
    {
        if (!ctx.lower->get_is_tx_ready())
        {
            return this;
        }

        const CryptoSpec crypto_spec(
            ctx.suite.nonce_mode,
            ctx.suite.dh_mode,
            ctx.suite.handshake_hash,
            ctx.suite.handshake_kdf,
            ctx.suite.handshake_mac,
            ctx.suite.session_mode
        );

        const auto err = ctx.handshake.set_algorithms(crypto_spec);
        if (any(err))
        {
            FORMAT_LOG_BLOCK(ctx.logger, levels::error, "Error configuring handshake algorithms: %s", HandshakeErrorSpec::to_string(err));
            // There's nothing we can do about a bad configuration, so just go to an error state
            return InitiatorHandshake::BadConfiguration::get();
        }

        const auto public_key = ctx.handshake.initialize();

        const RequestHandshakeBegin request(
            consts::crypto::protocol_version,
            crypto_spec,
			SessionConstraints(
				ctx.params.max_nonce_value,
				ctx.params.max_session_time_ms
			),
            CertificateMode::preshared_keys,
            public_key
        );

        const auto result = ctx.frame_writer->write(request);

        if (result.is_error())
        {
            return InitiatorHandshake::BadConfiguration::get();
        }

        ctx.handshake.set_ck(result.written);

        ctx.lower->transmit(result.frame);

        // record when we transmited the request so we can estimate the time base later
        ctx.request_handshake_begin_time_tx = ctx.executor->get_time();

        ctx.start_response_timer();

        return InitiatorHandshake::WaitForBeginReply::get();
    }

    // -------- WaitForBeginReply --------

    Initiator::IHandshakeState* InitiatorHandshake::WaitForBeginReply::on_message(Initiator& ctx, const ReplyHandshakeBegin& msg, const seq32_t& msg_bytes, const openpal::Timestamp& now)
    {
        ctx.response_and_retry_timer.cancel();

        std::error_code ec;
        ctx.handshake.derive_authentication_key(msg_bytes, ctx.local_static_key_pair->private_key, msg.ephemeral_public_key, ctx.remote_static_public_key->as_seq(), ec);

        if (ec)
        {
            FORMAT_LOG_BLOCK(ctx.logger, levels::error, "error deriving authentication key: %s", ec.message().c_str());
            ctx.start_retry_timer();
            return WaitForRetry::get();
        }

        HashOutput hash;
        ctx.handshake.calc_auth_handshake_mac(hash);

        const RequestHandshakeAuth request(hash.as_seq());

        const auto result = ctx.frame_writer->write(request);

        if (result.is_error())
        {
            return InitiatorHandshake::BadConfiguration::get();
        }

        ctx.handshake.mix_ck(result.written);

        ctx.lower->transmit(result.frame);

        ctx.start_response_timer();

        return WaitForAuthReply::get();
    }

    Initiator::IHandshakeState* InitiatorHandshake::WaitForBeginReply::on_message(Initiator& ctx, const ReplyHandshakeError& msg, const seq32_t& msg_bytes, const openpal::Timestamp& now)
    {
        ctx.response_and_retry_timer.cancel();
        FORMAT_LOG_BLOCK(ctx.logger, levels::error, "responder handshake error: %s", HandshakeErrorSpec::to_string(msg.handshake_error));
        ctx.start_retry_timer();
        return WaitForRetry::get();
    }

    Initiator::IHandshakeState* InitiatorHandshake::WaitForBeginReply::on_response_timeout(Initiator& ctx)
    {
        SIMPLE_LOG_BLOCK(ctx.logger, levels::error, "timeout while waiting for ReplyHandshakeBegin");
        ctx.start_retry_timer();
        return WaitForRetry::get();
    }

    // -------- WaitForAuthReply --------

    Initiator::IHandshakeState* InitiatorHandshake::WaitForAuthReply::on_message(Initiator& ctx, const ReplyHandshakeAuth& msg, const seq32_t& msg_bytes, const openpal::Timestamp& now)
    {
        ctx.response_and_retry_timer.cancel();

        if (!ctx.handshake.auth_handshake(msg.mac))
        {
            SIMPLE_LOG_BLOCK(ctx.logger, levels::warn, "authentication failure");
            ctx.start_retry_timer();
            return WaitForRetry::get();
        }

        ctx.handshake.mix_ck(msg_bytes);

        // TODO - be paranoid about clock rollback?
        const auto elapsed_ms = now.milliseconds - ctx.request_handshake_begin_time_tx.milliseconds;
        const auto estimated_init_time = now.milliseconds - (elapsed_ms / 2);

        ctx.handshake.initialize_session(ctx.session, openpal::Timestamp(estimated_init_time));

        return Idle::get();
    }

    Initiator::IHandshakeState* InitiatorHandshake::WaitForAuthReply::on_message(Initiator& ctx, const ReplyHandshakeError& msg, const seq32_t& msg_bytes, const openpal::Timestamp& now)
    {
        ctx.response_and_retry_timer.cancel();
        FORMAT_LOG_BLOCK(ctx.logger, levels::error, "responder handshake error: %s", HandshakeErrorSpec::to_string(msg.handshake_error));
        ctx.start_retry_timer();
        return WaitForRetry::get();
    }

    Initiator::IHandshakeState* InitiatorHandshake::WaitForAuthReply::on_response_timeout(Initiator& ctx)
    {
        SIMPLE_LOG_BLOCK(ctx.logger, levels::error, "timeout while waiting for ReplyHandshakeAuth");
        ctx.start_retry_timer();
        return WaitForRetry::get();
    }
}


