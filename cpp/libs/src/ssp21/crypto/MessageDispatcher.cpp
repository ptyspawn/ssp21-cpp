
#include "ssp21/crypto/MessageDispatcher.h"

#include "openpal/logging/LogMacros.h"
#include "ssp21/LogLevels.h"

namespace ssp21
{
    bool MessageDispatcher::Dispatch(openpal::Logger& logger, const openpal::RSlice& message, const openpal::Timestamp& now, IMessageHandler& handler)
    {
        if (message.is_empty())
        {
            SIMPLE_LOG_BLOCK(logger, levels::warn, "Received zero length message");
            return false;
        }

        const auto raw_function = message[0];
        const auto function = FunctionSpec::from_type(raw_function);

        if (!handler.supports(function))
        {
            FORMAT_LOG_BLOCK(logger, levels::warn, "Received unsupported function: %s(%u)", FunctionSpec::to_string(function), raw_function);
            return false;
        }

        switch (function)
        {
        case(Function::request_handshake_begin):
            return handle_message<RequestHandshakeBegin>(logger, message, now, handler);
        case(Function::request_handshake_auth):
            return handle_message<RequestHandshakeAuth>(logger, message, now, handler);
        case(Function::reply_handshake_begin):
            return handle_message<ReplyHandshakeBegin>(logger, message, now, handler);
        case(Function::reply_handshake_auth):
            return handle_message<ReplyHandshakeAuth>(logger, message, now, handler);
        case(Function::reply_handshake_error):
            return handle_message<ReplyHandshakeError>(logger, message, now, handler);
        case(Function::unconfirmed_session_data):
            return handle_message<UnconfirmedSessionData>(logger, message, now, handler);
        default:
            return false;
        }
    }

    void MessageDispatcher::log_parse_error(openpal::Logger& logger, Function function, ParseError error)
    {
        FORMAT_LOG_BLOCK(logger, levels::warn, "Error parsing function %s: %s", FunctionSpec::to_string(function), ParseErrorSpec::to_string(error));
    }
}

