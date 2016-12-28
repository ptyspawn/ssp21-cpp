
#include "HexMessageBuilders.h"

#include "openpal/container/StaticBuffer.h"

#include "testlib/HexConversions.h"
#include "testlib/Hex.h"

#include "ssp21/crypto/gen/RequestHandshakeBegin.h"
#include "ssp21/crypto/gen/ReplyHandshakeBegin.h"
#include "ssp21/crypto/gen/RequestHandshakeAuth.h"
#include "ssp21/crypto/gen/ReplyHandshakeAuth.h"
#include "ssp21/crypto/gen/ReplyHandshakeError.h"
#include "ssp21/crypto/gen/SessionData.h"

#include "MakeUnique.h"

#include <vector>
#include <assert.h>

using namespace openpal;

namespace ssp21
{
    namespace hex
    {

        std::string write_message(const IMessage& msg)
        {
            openpal::StaticBuffer<1024> buffer;
            auto dest = buffer.as_wslice();
            auto result = msg.write(dest);

            assert(!result.is_error());

            return to_hex(result.written);
        }

        std::string repeat(uint8_t value, uint8_t count)
        {
            return openpal::repeat_hex(value, count);
        }

        std::string func_to_hex(Function func)
        {
            return byte_to_hex(FunctionSpec::to_type(func));
        }

        std::string request_handshake_begin(
            uint16_t version,
            NonceMode nonce_mode,
            DHMode dh_mode,
            HandshakeHash handshake_hash,
            HandshakeKDF handshake_kdf,
            HandshakeMAC handshake_mac,
            SessionMode session_mode,
            CertificateMode certificate_mode,
            const std::string& hex_ephem_pub_key,
            std::initializer_list<std::string> certificates
        )
        {
            Hex pub_key(hex_ephem_pub_key);

            RequestHandshakeBegin msg(
                version,
                CryptoSpec(
                    nonce_mode,
                    dh_mode,
                    handshake_hash,
                    handshake_kdf,
                    handshake_mac,
                    session_mode
                ),
                certificate_mode,
                Seq8(pub_key.as_rslice())
            );

            std::vector<std::unique_ptr<Hex>> certificate_slices;

            for (auto& cert : certificates)
            {
                auto hex = std::make_unique<Hex>(cert);
                msg.certificates.push(hex->as_rslice());
                certificate_slices.push_back(std::move(hex));
            }

            return write_message(msg);
        }

        std::string request_handshake_auth(const std::string& mac)
        {
            Hex mac_bytes(mac);
            RequestHandshakeAuth request(Seq8(mac_bytes.as_rslice()));
            return write_message(request);
        }

        std::string reply_handshake_auth(const std::string& mac)
        {
            Hex mac_bytes(mac);
            ReplyHandshakeAuth reply(Seq8(mac_bytes.as_rslice()));
            return write_message(reply);
        }

        std::string reply_handshake_begin(
            const std::string& hex_ephem_pub_key,
            std::initializer_list<std::string> certificates
        )
        {
            Hex pub_key(hex_ephem_pub_key);

            ReplyHandshakeBegin msg(Seq8(pub_key.as_rslice()));

            std::vector<std::unique_ptr<Hex>> certificate_slices;

            for (auto& cert : certificates)
            {
                auto hex = std::make_unique<Hex>(cert);
                msg.certificates.push(hex->as_rslice());
                certificate_slices.push_back(std::move(hex));
            }

            return write_message(msg);
        }

        std::string reply_handshake_error(HandshakeError err)
        {
            ReplyHandshakeError msg(err);
            return write_message(msg);
        }

        std::string session_data(uint16_t nonce, uint32_t valid_until, bool fir, bool fin, const std::string& user_data, const std::string& auth_tag)
        {
            Hex user_data_hex(user_data);
            Hex auth_tag_hex(auth_tag);

            SessionData msg(
                AuthMetadata(
                    nonce,
                    valid_until,
                    SessionFlags(fir, fin)
                ),
                Seq16(user_data_hex.as_rslice()),
                Seq8(auth_tag_hex.as_rslice())
            );

            return write_message(msg);
        }

    }
}

