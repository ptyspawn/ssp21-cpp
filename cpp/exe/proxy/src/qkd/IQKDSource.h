#ifndef SSP21PROXY_IQKDSOURCE_H
#define SSP21PROXY_IQKDSOURCE_H

#include <ssp21/crypto/IKeyLookup.h>
#include <ssp21/crypto/IKeySource.h>

#include <cstdint>
#include <memory>

class IQKDSource {
public:
    virtual ~IQKDSource() = default;

    virtual std::shared_ptr<ssp21::IKeyLookup> bind_responder_key_lookup(uint16_t subscriber_id, uint16_t max_cached_keys) = 0;

    virtual std::shared_ptr<ssp21::IKeySource> bind_initiator_key_source(uint16_t subscriber_id, uint16_t max_cached_keys) = 0;
};

#endif
