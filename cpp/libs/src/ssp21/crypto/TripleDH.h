
#ifndef SSP21_TRIPLEDH_H
#define SSP21_TRIPLEDH_H

#include "ssp21/crypto/BufferTypes.h"
#include "ssp21/crypto/CryptoTypedefs.h"
#include "ssp21/crypto/StaticKeys.h"
#include "openpal/util/Uncopyable.h"


namespace ssp21
{

    /**
    * Computes a "triple DH" usign the supplied hash function and keys
    */
    class TripleDH : public openpal::Uncopyable
    {

    public:

        std::initializer_list<seq32_t> compute(
            dh_func_t dh,
            const StaticKeys& static_keys,
            const KeyPair& ephemeral_keys,
            const seq32_t& remote_public_static,
            const seq32_t& remote_public_ephemeral,
            std::error_code& ec
        );

    private:

        DHOutput dh1;
        DHOutput dh2;
        DHOutput dh3;

    };



}

#endif