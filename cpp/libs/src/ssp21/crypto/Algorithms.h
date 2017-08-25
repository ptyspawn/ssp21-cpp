
#ifndef SSP21_ALGORITHMS_H
#define SSP21_ALGORITHMS_H

#include "ssp21/crypto/CryptoTypedefs.h"
#include "ssp21/crypto/NonceFunctions.h"
#include "ssp21/crypto/SessionModes.h"
#include "ssp21/crypto/HandshakeAuthentication.h"

#include "ssp21/crypto/gen/CryptoSpec.h"
#include "ssp21/crypto/gen/HandshakeError.h"

#include "ssp21/crypto/Crypto.h"

namespace ssp21
{
    /**
    * Represents a complete set of algorithms for the handshake and the session
    */
    struct Algorithms
    {

    public:

        struct Session
        {
            Session() = default;

            verify_nonce_func_t verify_nonce = NonceFunctions::default_verify();
            ISessionMode* mode = &SessionModes::default_mode();
        };

        struct Handshake
        {
            Handshake() = default;

            dh_func_t dh = &Crypto::dh_x25519;
            kdf_func_t kdf = &Crypto::hkdf_sha256;
            hash_func_t hash = &Crypto::hash_sha256;            
            gen_keypair_func_t gen_keypair = &Crypto::gen_keypair_x25519;
        };

        // default constructor initializes with default algorithms
        Algorithms() = default;

        HandshakeError configure(const CryptoSpec& spec);

        // handshake algorithms
        Handshake handshake;

        // session algorithms
        Session session;
    };


}

#endif
