

#include "Backend.h"

#include <sodium.h>
#include <assert.h>

namespace ssp21
{
	namespace sodium
	{
		/// assertions for SHA-256 related constants
		static_assert(crypto_hash_sha256_BYTES == crypto_auth_hmacsha256_BYTES, "SHA256 hash and HMAC length mismatch");
		static_assert(consts::SHA256_HASH_OUTPUT_LENGTH == crypto_hash_sha256_BYTES, "SHA256 length mismatch");
		static_assert(consts::SHA256_HASH_OUTPUT_LENGTH == crypto_auth_hmacsha256_BYTES, "SHA256-HMAC length mismatch");

		/// assertions for DH key lengths
		static_assert(consts::X25519_KEY_LENGTH == crypto_scalarmult_BYTES, "X25519 key length mismatch");
		
		void zero_memory(openpal::WSlice& buff)
		{
			sodium_memzero(buff, buff.Size());
		}

		bool secure_compare(const openpal::RSlice& lhs, const openpal::RSlice& rhs)
		{
			if (lhs.Size() != rhs.Size()) return false;

			return sodium_memcmp(lhs, rhs, lhs.Size()) == 0;
		}

		void calc_hash_sha256(std::initializer_list<openpal::RSlice> data, HashOutput& output)
		{			
			crypto_hash_sha256_state state;
			crypto_hash_sha256_init(&state);

			for (auto& item : data)
			{
				crypto_hash_sha256_update(&state, item, item.Size());
			}

			crypto_hash_sha256_final(&state, output.get_write_slice());

			output.set_type(HashOutputType::SHA256);
		}

		void calc_hmac_sha256(const openpal::RSlice& key, std::initializer_list<openpal::RSlice> data, HashOutput& output)
		{			
			crypto_auth_hmacsha256_state state;
			crypto_auth_hmacsha256_init(&state, key, key.Size());

			for (auto& item : data)
			{
				crypto_auth_hmacsha256_update(&state, item, item.Size());
			}


			crypto_auth_hmacsha256_final(&state, output.get_write_slice());

			output.set_type(HashOutputType::SHA256);
		}

		void gen_keypair_x25519(KeyPair& pair)
		{
			auto dest = pair.private_key.get_write_slice();
			randombytes_buf(dest, crypto_scalarmult_BYTES);
			crypto_scalarmult_base(pair.public_key.get_write_slice(), dest);

			pair.public_key.set_key_type(KeyType::X25519);
			pair.private_key.set_key_type(KeyType::X25519);
		}

		void dh_x25519(const Key& priv_key, const Key& pub_key, Key& output, std::error_code& ec)
		{
			if (crypto_scalarmult(output.get_write_slice(), priv_key.as_slice(), pub_key.as_slice()) != 0)
			{
				ec = std::error_code(1, std::generic_category()); // TODO - make actual error codes
				return;
			}

			output.set_key_type(KeyType::X25519);
		}
	}
}
