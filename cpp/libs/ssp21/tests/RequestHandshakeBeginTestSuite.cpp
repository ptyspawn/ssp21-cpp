

#include "catch.hpp"

#include "crypto/LogMessagePrinter.h"
#include "crypto/gen/RequestHandshakeBegin.h"
#include "ssp21/crypto/Constants.h"

#include "ser4cpp/container/StaticBuffer.h"
#include "ser4cpp/util/HexConversions.h"

#include "log4cpp/MockLogHandler.h"
#include "mocks/HexSequences.h"
#include "ssp21/stack/LogLevels.h"

#define SUITE(name) "RequestHandshakeBeginTestSuite - " name

using namespace ssp21;
using namespace ser4cpp;

TEST_CASE(SUITE("returns error on empty message"))
{
    RequestHandshakeBegin msg;
    auto input = seq32_t::empty();
    REQUIRE(msg.read(input) == ParseError::insufficient_bytes);
}

TEST_CASE(SUITE("returns error on undefined enum"))
{
    RequestHandshakeBegin msg;

    auto input = HexConversions::from_hex("DD");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(err == ParseError::undefined_enum);
}

TEST_CASE(SUITE("returns error on unexpected function"))
{
    RequestHandshakeBegin msg;

    auto input = HexConversions::from_hex("03");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(err == ParseError::unexpected_function);
}

TEST_CASE(SUITE("returns error if too little data"))
{
    RequestHandshakeBegin msg;

    auto input = HexConversions::from_hex("00");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(err == ParseError::insufficient_bytes);
}

TEST_CASE(SUITE("successfully parses message"))
{
    RequestHandshakeBegin msg;

    auto input = HexConversions::from_hex("00 D1 D2 A3 A4 00 00 00 00 00 FF FF CA FE BA BE 00 03 AA AA AA 00");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(!any(err));
    REQUIRE(msg.version.major_version == 0xD1D2);
    REQUIRE(msg.version.minor_version == 0xA3A4);
    REQUIRE(msg.spec.session_nonce_mode == SessionNonceMode::strict_increment);
    REQUIRE(msg.spec.handshake_ephemeral == HandshakeEphemeral::x25519);
    REQUIRE(msg.spec.handshake_hash == HandshakeHash::sha256);
    REQUIRE(msg.spec.handshake_kdf == HandshakeKDF::hkdf_sha256);
    REQUIRE(msg.spec.session_crypto_mode == SessionCryptoMode::hmac_sha256_16);

    REQUIRE(msg.constraints.max_nonce == 0xFFFF);
    REQUIRE(msg.constraints.max_session_duration == 0xCAFEBABE);
    REQUIRE(msg.handshake_mode == HandshakeMode::shared_secret);
    REQUIRE(HexConversions::to_hex(msg.mode_ephemeral) == "AA AA AA");

    REQUIRE(msg.mode_data.is_empty());
}

TEST_CASE(SUITE("pretty prints message"))
{

    HexSeq public_key("CA FE");

    RequestHandshakeBegin msg(
        Version(7, 12),
        CryptoSpec(
            HandshakeEphemeral::x25519,
            HandshakeHash::sha256,
            HandshakeKDF::hkdf_sha256,
            SessionNonceMode::greater_than_last_rx,
            SessionCryptoMode::hmac_sha256_16),
        SessionConstraints(
            32768,
            0xCAFEBABE),
        HandshakeMode::public_keys,
        public_key,
        seq32_t::empty());

    log4cpp::MockLogHandler log("log");
    LogMessagePrinter printer(log.logger, ssp21::levels::info, 16);

    msg.print(printer);

    log.expect(
        "major_version: 7",
        "minor_version: 12",
        "handshake_ephemeral: x25519",
        "handshake_hash: sha256",
        "handshake_kdf: hkdf_sha256",
        "session_nonce_mode: greater_than_last_rx",
        "session_crypto_mode: hmac_sha256_16",
        "max_nonce: 32768",
        "max_session_duration: 3405691582",
        "handshake_mode: public_keys",
        "mode_ephemeral (length = 2)",
        "CA:FE",
        "mode_data (length = 0)");
}

TEST_CASE(SUITE("rejects unknown enum"))
{
    RequestHandshakeBegin msg;

    //                                    -------------VV-------------------------------------
    auto input = HexConversions::from_hex("00 D1 D2 00 CC 00 00 00 03 AA AA AA 01 00 02 BB BB");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(err == ParseError::undefined_enum);
}

TEST_CASE(SUITE("rejects trailing data"))
{
    RequestHandshakeBegin msg;

    //                                     ---------------------------------------------------------------VV VV------ zero certificate data
    auto input = HexConversions::from_hex("00 D1 D2 A1 A2 00 00 00 00 00 FF FF CA FE BA BE 00 03 AA AA AA 00 00 02");
    auto slice = input->as_rslice();

    auto err = msg.read(slice);
    REQUIRE(err == ParseError::too_many_bytes);
}

TEST_CASE(SUITE("formats default value"))
{
    ser4cpp::StaticBuffer<uint32_t, RequestHandshakeBegin::min_size_bytes> buffer;
    RequestHandshakeBegin msg;
    auto dest = buffer.as_wseq();
    auto res = msg.write(dest);

    REQUIRE(!res.is_error());
    REQUIRE(res.written.length() == msg.size());
    REQUIRE(HexConversions::to_hex(res.written) == "00 00 00 00 00 FF FF FF FF FF 00 00 00 00 00 00 FF 00 00");
}

TEST_CASE(SUITE("returns error if insufficient buffer space"))
{
    ser4cpp::StaticBuffer<uint32_t, RequestHandshakeBegin::min_size_bytes - 1> buffer;
    RequestHandshakeBegin msg;
    auto dest = buffer.as_wseq();
    REQUIRE(msg.write(dest).err == FormatError::insufficient_space);
}
