
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "mocks/MockCryptoBackend.h"
#include "ssp21/crypto/Crypto.h"

int main(int argc, char* argv[])
{
    // global setup...
    ssp21::MockCryptoBackend::initialize();

    return Catch::Session().run(argc, argv);
}
