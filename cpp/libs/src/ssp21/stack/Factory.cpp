
#include "ssp21/stack/Factory.h"

#include "ssp21/stack/LinkCryptoStack.h"

namespace ssp21
{

    std::shared_ptr<IStack> Factory::responder(
        Addresses addresses,
        const ResponderConfig& config,
        openpal::Logger logger,
        const std::shared_ptr<openpal::IExecutor>& executor,
        const LocalKeys& keys,
        const std::shared_ptr<ICertificateMode>& certificate_mode)
    {
        return std::make_shared<ResponderStack>(addresses, config, logger, executor, keys, certificate_mode);
    }

    std::shared_ptr<IStack> Factory::initiator(
        Addresses addresses,
        const InitiatorConfig& config,
        openpal::Logger logger,
        const std::shared_ptr<openpal::IExecutor>& executor,
        const LocalKeys& keys,
        const std::shared_ptr<ICertificateMode>& certificate_mode)
    {
        return std::make_shared<InitiatorStack>(addresses, config, logger, executor, keys, certificate_mode);
    }

}

