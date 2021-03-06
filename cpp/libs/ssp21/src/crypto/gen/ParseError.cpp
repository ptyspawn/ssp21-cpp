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

#include "ssp21/crypto/gen/ParseError.h"

namespace ssp21 {

const char* ParseErrorSpec::to_string(ParseError arg)
{
    switch(arg)
    {
        case(ParseError::ok):
            return "ok";
        case(ParseError::insufficient_bytes):
            return "insufficient_bytes";
        case(ParseError::undefined_enum):
            return "undefined_enum";
        case(ParseError::unexpected_function):
            return "unexpected_function";
        case(ParseError::too_many_bytes):
            return "too_many_bytes";
        case(ParseError::impl_capacity_limit):
            return "impl_capacity_limit";
        case(ParseError::reserved_bit):
            return "reserved_bit";
        case(ParseError::bad_vlength):
            return "bad_vlength";
        default:
            return "undefined";
    }
}

}
