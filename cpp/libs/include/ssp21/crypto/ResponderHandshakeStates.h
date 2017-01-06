
#ifndef SSP21_RESPONDERHANDSHAKESTATES_H
#define SSP21_RESPONDERHANDSHAKESTATES_H

#include "ssp21/crypto/Responder.h"

namespace ssp21
{

    class ResponderHandshakeIdle final : public Responder::IHandshakeState
    {
        ResponderHandshakeIdle() {}

    public:

        virtual Responder::IHandshakeState& on_message(Responder& ctx, const seq32_t& msg_bytes, const RequestHandshakeBegin& msg) override;

        virtual Responder::IHandshakeState& on_message(Responder& ctx, const seq32_t& msg_bytes, const RequestHandshakeAuth& msg) override;

        static Responder::IHandshakeState& get()
        {
            static ResponderHandshakeIdle instance;
            return instance;
        }

    };

    class ResponderHandshakeWaitForAuth final : public Responder::IHandshakeState
    {
        ResponderHandshakeWaitForAuth() {}

    public:

        virtual Responder::IHandshakeState& on_message(Responder& ctx, const seq32_t& msg_bytes, const RequestHandshakeBegin& msg) override;

        virtual Responder::IHandshakeState& on_message(Responder& ctx, const seq32_t& msg_bytes, const RequestHandshakeAuth& msg) override;

        static Responder::IHandshakeState& get()
        {
            static ResponderHandshakeWaitForAuth instance;
            return instance;
        }

    };

}

#endif
