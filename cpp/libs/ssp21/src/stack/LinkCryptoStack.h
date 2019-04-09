
#ifndef SSP21_LINKCRYPTOSTACK_H
#define SSP21_LINKCRYPTOSTACK_H

#include "link/LinkLayer.h"
#include "crypto/Responder.h"
#include "crypto/Initiator.h"
#include "link/LinkFrameWriter.h"
#include "ssp21/stack/IStack.h"

namespace ssp21
{
    class ResponderStack final : public IStack
    {
    public:

        ResponderStack(
            Addresses addresses,
            const ResponderConfig& config,
            log4cpp::Logger logger,
            const std::shared_ptr<exe4cpp::IExecutor>& executor,
            const std::shared_ptr<IResponderHandshake>& handshake
        ) :
            link(addresses.source, addresses.destination),
            responder(
                config,
                logger,
                get_frame_writer(logger, addresses, consts::link::max_config_payload_size),
                executor,
                handshake
            )
        {

        }

        virtual void bind(ILowerLayer& lower, IUpperLayer& upper) override
        {
            this->link.bind(lower, responder);
            this->responder.bind(link, upper);
        }

        virtual ILowerLayer& get_lower() override
        {
            return this->responder;
        }

        virtual IUpperLayer& get_upper() override
        {
            return this->link;
        }

    private:

        static std::shared_ptr<IFrameWriter> get_frame_writer(log4cpp::Logger logger, Addresses addresses, uint16_t max_payload_size)
        {
            return std::make_shared<LinkFrameWriter>(logger, addresses, max_payload_size);
        }

        LinkLayer link;
        Responder responder;
    };

    class InitiatorStack final : public IStack
    {
    public:

        InitiatorStack(
            Addresses addresses,
            const InitiatorConfig& config,
            log4cpp::Logger logger,
            const std::shared_ptr<exe4cpp::IExecutor>& executor,
            const std::shared_ptr<IInitiatorHandshake>& handshake) :
            link(addresses.source, addresses.destination),
            initiator(
                config,
                logger,
                get_frame_writer(logger, addresses, consts::link::max_config_payload_size),
                executor,
                handshake
            )
        {

        }

        virtual void bind(ILowerLayer& lower, IUpperLayer& upper) override
        {
            this->link.bind(lower, initiator);
            this->initiator.bind(link, upper);
        }

        virtual ILowerLayer& get_lower() override
        {
            return this->initiator;
        }

        virtual IUpperLayer& get_upper() override
        {
            return this->link;
        }

    private:

        static std::shared_ptr<IFrameWriter> get_frame_writer(log4cpp::Logger logger, Addresses addresses, uint16_t max_payload_size)
        {
            return std::make_shared<LinkFrameWriter>(logger, addresses, max_payload_size);
        }

        LinkLayer link;
        Initiator initiator;
    };
}

#endif