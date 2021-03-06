
#include "crypto/QKDResponderHandshake.h"

#include "ssp21/util/Exception.h"

#include "crypto/Algorithms.h"
#include "crypto/HandshakeHasher.h"
#include "crypto/ProtocolVersion.h"
#include "crypto/gen/ReplyHandshakeBegin.h"

#include "log4cpp/LogMacros.h"
#include "ssp21/stack/LogLevels.h"

namespace ssp21 {

QKDResponderHandshake::QKDResponderHandshake(const log4cpp::Logger& logger, const std::shared_ptr<IKeyLookup>& key_lookup)
    : logger(logger)
    , key_lookup(key_lookup)
{
}

IResponderHandshake::Result QKDResponderHandshake::process(const RequestHandshakeBegin& msg, const seq32_t& msg_bytes, const exe4cpp::steady_time_t& now, IFrameWriter& writer, Session& session)
{
    // mode ephemeral must be empty
    if (msg.mode_ephemeral.is_not_empty()) {
        return Result::failure(HandshakeError::bad_message_format);
    }

    Algorithms::Common algorithms;

    {
        const auto err = algorithms.configure(msg.spec);
        if (any(err)) {
            return IResponderHandshake::Result::failure(err);
        }
    }

    // deserialize the key identifier from the handshake data
    uint64_t key_id;
    {
        auto key_id_data = msg.mode_data;
        if (!ser4cpp::BigEndian::read(key_id_data, key_id) || key_id_data.is_not_empty()) {
            return Result::failure(HandshakeError::bad_message_format);
        }
    }

    // look-up the request shared secret, this also validates the handshake data field (empty or key id)
    const auto shared_secret = this->key_lookup->find_and_consume_key(key_id);
    if (!shared_secret) {
        // the requested key was not found
        return Result::failure(HandshakeError::key_not_found);
    }

    {
        const auto err = algorithms.configure(msg.spec);
        if (any(err))
            return Result::failure(err);
    }

    // prepare the response
    const ReplyHandshakeBegin reply(
        version::get(),
        seq32_t::empty(),
        seq32_t::empty());

    const auto result = writer.write(reply);
    if (any(result.err)) {
        FORMAT_LOG_BLOCK(this->logger, levels::error, "Error writing handshake reply: %s", FormatErrorSpec::to_string(result.err));
        return Result::failure(HandshakeError::unknown);
    }

    HandshakeHasher hasher;
    const auto handshake_hash = hasher.compute(algorithms.handshake.hash, msg_bytes, result.written);

    SessionKeys session_keys;

    algorithms.handshake.kdf(
        handshake_hash,
        { shared_secret->as_seq() },
        session_keys.rx_key,
        session_keys.tx_key);

    session.initialize(
        algorithms.session,
        Session::Param(
            now,
            msg.constraints.max_nonce,
            std::chrono::milliseconds(msg.constraints.max_session_duration)),
        session_keys);

    return Result::success(result.frame);
}

}
