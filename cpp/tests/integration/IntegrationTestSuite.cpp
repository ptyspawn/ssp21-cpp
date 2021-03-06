

#include "catch.hpp"

#include "fixtures/IntegrationFixture.h"

#include "log4cpp/ConsolePrettyPrinter.h"
#include "ssp21/stack/LogLevels.h"

#define SUITE(name) "IntegrationTestSuite - " name

using namespace ssp21;

void open_and_test_handshake(IntegrationFixture& fix);
void test_bidirectional_data_transfer(IntegrationFixture& fix, const seq32_t& data);

const auto HANDSHAKE_TYPES = { HandshakeType::shared_secret, HandshakeType::qkd, HandshakeType::preshared_key, HandshakeType::certificates };
const auto SESSION_MODES = { SessionCryptoMode::hmac_sha256_16, SessionCryptoMode::aes_256_gcm };

template <class T>
void for_each_mode(const T& action)
{
    for (auto type : HANDSHAKE_TYPES) {
        for (auto mode : SESSION_MODES) {
            action(type, mode);
        }
    }
}

TEST_CASE(SUITE("fixture construction"))
{
    auto run_test = [](HandshakeType type, SessionCryptoMode mode) {
        IntegrationFixture fix(type, mode);
    };

    for_each_mode(run_test);
}

TEST_CASE(SUITE("completes handshake"))
{
    auto run_test = [](HandshakeType type, SessionCryptoMode mode) {
        IntegrationFixture fix(type, mode);
        open_and_test_handshake(fix);
    };

    for_each_mode(run_test);
}

TEST_CASE(SUITE("can transfer data bidirectionally multiple times"))
{
    auto run_test = [](HandshakeType type, SessionCryptoMode mode) {
        IntegrationFixture fix(type, mode);
        open_and_test_handshake(fix);

        const auto num_bytes_tx = 64;
        uint8_t payload[num_bytes_tx] = { 0x00 };
        for (int i = 0; i < num_bytes_tx; ++i)
            payload[i] = i % 256;
        const auto slice = seq32_t(payload, num_bytes_tx);

        for (int i = 0; i < 5; ++i) {
            test_bidirectional_data_transfer(fix, slice);
        }
    };

    for_each_mode(run_test);
}

void open_and_test_handshake(IntegrationFixture& fix)
{
    fix.stacks.responder->on_lower_open();
    fix.stacks.initiator->on_lower_open();

    REQUIRE_FALSE(fix.responder_upper.is_open());
    REQUIRE_FALSE(fix.initiator_upper.is_open());

    REQUIRE(fix.exe->run_many() > 0);

    REQUIRE(fix.responder_upper.is_open());
    REQUIRE(fix.initiator_upper.is_open());
}

void test_bidirectional_data_transfer(IntegrationFixture& fix, const seq32_t& data)
{
    REQUIRE(fix.responder_validator->is_empty());
    REQUIRE(fix.initiator_validator->is_empty());

    fix.responder_validator->expect(data);
    fix.initiator_validator->expect(data);

    REQUIRE(fix.stacks.initiator->start_tx_from_upper(data));
    REQUIRE(fix.stacks.responder->start_tx_from_upper(data));

    REQUIRE(fix.exe->run_many() > 0);

    REQUIRE(fix.responder_validator->is_empty());
    REQUIRE(fix.initiator_validator->is_empty());
}
