//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
//
// This file is auto-generated. Do not edit manually
//
// Licensed under the terms of the BSDv3 license
//

#ifndef SSP21_PARSEERROR_H
#define SSP21_PARSEERROR_H

#include <cstdint>
#include "ser4cpp/util/Uncopyable.h"

namespace ssp21 {

/**
    The result of a message parse operation
*/
enum class ParseError : uint8_t
{
    /// message was parsed successfully
    ok = 0x0,
    /// parser ran out of bytes before completion
    insufficient_bytes = 0x1,
    /// an enumeration value was undefined
    undefined_enum = 0x2,
    /// the message function was not the expected value
    unexpected_function = 0x3,
    /// parser completed with bytes still remaining
    too_many_bytes = 0x4,
    /// reached an implementation specific capacity limit
    impl_capacity_limit = 0x5,
    /// the bit-field contains a reserved bit(s) that is set
    reserved_bit = 0x6,
    /// a variable length (vlength) was not encoded correctly
    bad_vlength = 0x7
};

inline bool any(ParseError value)
{
    return value != ParseError::ok;
}

struct ParseErrorSpec : private ser4cpp::StaticOnly
{
    using enum_type_t = ParseError;

    static const char* to_string(ParseError arg);
};

}

#endif
