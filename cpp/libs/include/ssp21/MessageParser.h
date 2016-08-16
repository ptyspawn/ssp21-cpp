#ifndef SSP21_MESSAGE_PARSER_H
#define SSP21_MESSAGE_PARSER_H

#include "ssp21/gen/ParseError.h"

#include "ssp21/gen/Function.h"
#include "ssp21/gen/CertificateMode.h"
#include "ssp21/gen/DHMode.h"
#include "ssp21/gen/HandshakeError.h"
#include "ssp21/gen/NonceMode.h"
#include "ssp21/gen/SessionMode.h"
#include "ssp21/gen/HashMode.h"

#include "ssp21/SequenceTypes.h"

#include "openpal/container/RSlice.h"
#include "openpal/util/Uncopyable.h"

namespace ssp21 {

	class MessageParser : private openpal::StaticOnly
	{
		
		template <Function expected, typename T, typename... Args>
		static ParseError read_message(openpal::RSlice& input, T& value, Args& ... args)
		{
			Function func;
			auto result = read(input, func);
			if (ret != ParseResult::ok) return ret;
			if (func != expected) return ParseResult::unexpected_function;

			return read_fields(input, value, args...)
		}

	private:

		template <typename T, typename... Args>
		static ParseError read_fields(openpal::RSlice& input, T& value, Args& ... args)
		{
			auto ret = read(input, value);
			if (ret != ParseResult::ok) return ret;
			return read_fields(input, args...);
		}

		// when we reach the base case of the recursion there shouldn't be any remaining data
		static ParseError read_fields(openpal::RSlice& input) {
			return input.is_empty() ? ParseError::ok : ParseError::too_many_bytes;
		}
		

		// integers
		static ParseError read(openpal::RSlice& input, uint8_t& value);
		static ParseError read(openpal::RSlice& input, uint16_t& value);
		static ParseError read(openpal::RSlice& input, uint32_t& value);

		// enums
		static ParseError read(openpal::RSlice& input, CertificateMode& value);
		static ParseError read(openpal::RSlice& input, DHMode& value);
		static ParseError read(openpal::RSlice& input, HandshakeError& value);
		static ParseError read(openpal::RSlice& input, NonceMode& value);
		static ParseError read(openpal::RSlice& input, SessionMode& value);
		static ParseError read(openpal::RSlice& input, HashMode& value);

		// sequences
		static ParseError read(openpal::RSlice& input, Seq8& value);
		static ParseError read(openpal::RSlice& input, Seq16& value);
		static ParseError read(openpal::RSlice& input, Seq8Seq16& value);
	};
}

#endif