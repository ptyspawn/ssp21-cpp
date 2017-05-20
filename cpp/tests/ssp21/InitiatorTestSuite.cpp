
#include "catch.hpp"

#include "fixtures/CryptoLayerFixture.h"

#define SUITE(name) "InitiatorTestSuite - " name

using namespace ssp21;
using namespace openpal;

using HandshakeState = Initiator::IHandshakeState::Enum;

void test_open(InitiatorFixture& fix);
void test_request_handshake_begin(InitiatorFixture& fix);
void test_response_timeout(InitiatorFixture& fix, HandshakeState new_state);
void test_reply_handshake_begin(InitiatorFixture& fix);
void test_reply_handshake_auth(InitiatorFixture& fix);
void test_open_and_full_handshake(InitiatorFixture& fix);

// ---------- tests for initial state -----------

TEST_CASE(SUITE("construction"))
{
    InitiatorFixture fix;
    REQUIRE(CryptoFixture::instance != nullptr);
}

TEST_CASE(SUITE("transmits REQUEST_HANDSHAKE_BEGIN when opened"))
{
    InitiatorFixture fix;
    test_open(fix);
}

TEST_CASE(SUITE("goes to bad_configuration state if algorithms aren't supported"))
{
    InitiatorConfig config;
    config.suite.dh_mode = DHMode::undefined;

    InitiatorFixture fix(config);
    fix.initiator.on_lower_open();
    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::bad_configuration);
    REQUIRE(fix.exe->num_pending_timers() == 0);
    REQUIRE(fix.lower.num_tx_messages() == 0);
}

TEST_CASE(SUITE("ignores reply handshake auth while waiting for reply handshake begin"))
{
    InitiatorFixture fix;
    test_open(fix);

    REQUIRE(fix.exe->num_timer_cancel() == 0);
    fix.lower.enqueue_message(hex::reply_handshake_auth(hex::repeat(0xFF, consts::crypto::sha256_hash_output_length)));
    REQUIRE(fix.exe->num_timer_cancel() == 0);
    REQUIRE(fix.lower.num_tx_messages() == 0);
}

TEST_CASE(SUITE("stops timer when closed"))
{
    InitiatorFixture fix;
    test_open(fix);
    fix.initiator.on_lower_close();
    REQUIRE(fix.exe->num_active() == 0);
}

TEST_CASE(SUITE("cancels response timer and retries handshake when error message received"))
{
    InitiatorFixture fix;
    test_open(fix);

    fix.lower.enqueue_message(hex::reply_handshake_error(HandshakeError::bad_message_format));
    REQUIRE(fix.exe->num_timer_cancel() == 1);
    REQUIRE(fix.exe->num_pending_timers() == 1);
    REQUIRE(fix.exe->advance_to_next_timer());
    REQUIRE(fix.exe->run_many() > 0);

    test_request_handshake_begin(fix);
}

TEST_CASE(SUITE("starts retry timer when response timeout fires"))
{
    InitiatorFixture fix;
    test_open(fix);
    test_response_timeout(fix, HandshakeState::wait_for_retry);
}

// ---------- tests for WaitBeginReply -----------

TEST_CASE(SUITE("send REQUEST_HANDSHAKE_AUTH after receving REPLY_HANDSHAKE_BEGIN"))
{
    InitiatorFixture fix;
    test_open(fix);
    test_reply_handshake_begin(fix);
}

TEST_CASE(SUITE("goes to retry state when response timeout occurs while waiting for REPLY_HANDSHAKE_AUTH"))
{
    InitiatorFixture fix;
    test_open(fix);
    test_reply_handshake_begin(fix);
    test_response_timeout(fix, HandshakeState::wait_for_retry);
}

TEST_CASE(SUITE("goes to retry state when handshake reply error received while waiting for REPLY_HANDSHAKE_AUTH"))
{
    InitiatorFixture fix;
    test_open(fix);
    test_reply_handshake_begin(fix);

    REQUIRE(fix.exe->num_timer_cancel() == 1);
    fix.lower.enqueue_message(hex::reply_handshake_error(HandshakeError::no_prior_handshake_begin)); // can be any error code
    REQUIRE(fix.exe->num_timer_cancel() == 2);
    REQUIRE(fix.exe->num_pending_timers() == 1);
}

TEST_CASE(SUITE("goes to retry state when DH fails"))
{
    InitiatorFixture fix;
    test_open(fix);

    fix.set_fail_dh_x25519(true);
    fix.lower.enqueue_message(hex::reply_handshake_begin(hex::repeat(0xFF, consts::crypto::x25519_key_length)));
    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_retry);
}

TEST_CASE(SUITE("initializes session when a proper auth reply is received"))
{
    InitiatorFixture fix;
    test_open_and_full_handshake(fix);
}

TEST_CASE(SUITE("triggers session renegotiation after timeout"))
{
    InitiatorFixture fix;
    test_open_and_full_handshake(fix);

    const auto before_time = fix.exe->get_time();

    REQUIRE(fix.exe->advance_to_next_timer());
    REQUIRE(fix.exe->run_many() == 1);

    const auto actual_elapsed_ms = fix.exe->get_time().milliseconds - before_time.milliseconds;
    const auto expected_elapsed_ms = consts::crypto::initiator::default_session_time_renegotiation_trigger_ms;

    REQUIRE(actual_elapsed_ms == expected_elapsed_ms);

    test_request_handshake_begin(fix);
}

TEST_CASE(SUITE("goes to retry state if auth reply doesn't authenticate"))
{
    InitiatorFixture fix;
    test_open(fix);
    test_reply_handshake_begin(fix);

    const auto start_num_timer_cancel = fix.exe->num_timer_cancel();
    const auto start_stats = fix.initiator.get_statistics();

    // incorrect MAC
    fix.lower.enqueue_message(hex::reply_handshake_auth(hex::repeat(0xEE, consts::crypto::sha256_hash_output_length)));

    const auto end_stats = fix.initiator.get_statistics();
    const auto end_num_timer_cancel = fix.exe->num_timer_cancel();

    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_retry);
    REQUIRE(end_num_timer_cancel == (start_num_timer_cancel + 1));
    REQUIRE(end_stats.session.num_init == start_stats.session.num_init);

    // causes the master to go through key derivation
    fix.expect(
    {
        CryptoAction::hmac_sha256,  // authenticate
        CryptoAction::secure_equals // last action since it fails
    });
}


// ---------- helper implementations -----------

void test_open(InitiatorFixture& fix)
{
    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::idle);
    REQUIRE(fix.lower.num_tx_messages() == 0);
    fix.expect_empty();

    fix.initiator.on_lower_open();

    test_request_handshake_begin(fix);
}

void test_request_handshake_begin(InitiatorFixture& fix)
{
    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_begin_reply);
    REQUIRE(fix.lower.num_tx_messages() == 1);

    const auto expected = hex::request_handshake_begin(
                              0,
                              NonceMode::increment_last_rx,
                              DHMode::x25519,
                              HandshakeHash::sha256,
                              HandshakeKDF::hkdf_sha256,
                              HandshakeMAC::hmac_sha256,
                              SessionMode::hmac_sha256_16,
                              consts::crypto::initiator::default_max_nonce,
                              consts::crypto::initiator::default_max_session_time_ms,
                              CertificateMode::preshared_keys,
                              hex::repeat(0xFF, 32)
                          );

    REQUIRE(fix.lower.pop_tx_message() == expected);
    REQUIRE(fix.exe->num_pending_timers() == 1);

    fix.expect({ CryptoAction::gen_keypair_x25519, CryptoAction::hash_sha256 });
}

void test_response_timeout(InitiatorFixture& fix, HandshakeState new_state)
{
    const auto num_timer_cancel = fix.exe->num_timer_cancel();

    REQUIRE(fix.exe->next_timer_expiration_rel() == consts::crypto::initiator::default_response_timeout);
    REQUIRE(fix.exe->advance_to_next_timer());
    REQUIRE(fix.exe->run_many() == 1);
    REQUIRE(fix.exe->num_timer_cancel() == num_timer_cancel);
    REQUIRE(fix.exe->num_pending_timers() == 1);
    REQUIRE(fix.lower.num_rx_messages() == 0);

    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_retry);

    fix.expect_empty();
}

void test_reply_handshake_begin(InitiatorFixture& fix)
{
    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_begin_reply);

    const auto num_timer_cancel = fix.exe->num_timer_cancel();

    fix.expect_empty();
    fix.lower.enqueue_message(hex::reply_handshake_begin(hex::repeat(0xFF, consts::crypto::x25519_key_length)));

    const auto expected = hex::request_handshake_auth(hex::repeat(0xFF, consts::crypto::sha256_hash_output_length));
    REQUIRE(fix.lower.pop_tx_message() == expected);

    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::wait_for_auth_reply);

    // causes the master to go through key derivation
    fix.expect(
    {
        CryptoAction::hash_sha256, // mix ck
        CryptoAction::dh_x25519,   // triple DH
        CryptoAction::dh_x25519,
        CryptoAction::dh_x25519,

        CryptoAction::hkdf_sha256, // KDF        ,
        CryptoAction::hmac_sha256,

        CryptoAction::hash_sha256  // mix ck
    });

    REQUIRE(fix.exe->num_timer_cancel() == (num_timer_cancel + 1));
    REQUIRE(fix.exe->num_pending_timers() == 1);
    REQUIRE(fix.exe->next_timer_expiration_rel() == consts::crypto::initiator::default_response_timeout);

}

void test_reply_handshake_auth(InitiatorFixture& fix)
{
    const auto start_num_timer_cancel = fix.exe->num_timer_cancel();
    const auto start_stats = fix.initiator.get_statistics();

    fix.lower.enqueue_message(hex::reply_handshake_auth(hex::repeat(0xFF, consts::crypto::sha256_hash_output_length)));

    const auto end_stats = fix.initiator.get_statistics();
    const auto end_num_timer_cancel = fix.exe->num_timer_cancel();

    REQUIRE(fix.initiator.get_state_enum() == HandshakeState::idle);
    REQUIRE(end_num_timer_cancel == (start_num_timer_cancel + 1));
    REQUIRE(end_stats.session.num_init == (start_stats.session.num_init + 1));

    REQUIRE(fix.exe->num_pending_timers() == 1); // the session timeout timer should be the only timer active

    // causes the master to go through key derivation
    fix.expect(
    {
        CryptoAction::hmac_sha256, // authenticate
        CryptoAction::secure_equals,
        CryptoAction::hash_sha256, // mix ck
        CryptoAction::hkdf_sha256
    });

    REQUIRE(fix.upper.get_is_open());
}

void test_open_and_full_handshake(InitiatorFixture& fix)
{
    test_open(fix);
    test_reply_handshake_begin(fix);
    test_reply_handshake_auth(fix);
}