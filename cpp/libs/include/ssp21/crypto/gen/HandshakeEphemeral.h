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

#ifndef SSP21_HANDSHAKEEPHEMERAL_H
#define SSP21_HANDSHAKEEPHEMERAL_H

#include <cstdint>
#include "openpal/util/Uncopyable.h"

namespace ssp21 {

/**
    Specifies the meaning of the ephemeral data in RequestHandshakeBegin
*/
enum class HandshakeEphemeral : uint8_t
{
    /// x25519 algorithm public key
    x25519 = 0x0,
    /// 32-byte random nonce
    nonce = 0x1,
    /// value not defined
    undefined = 0xFF
};

struct HandshakeEphemeralSpec : private openpal::StaticOnly
{
    typedef HandshakeEphemeral enum_type_t;

    static uint8_t to_type(HandshakeEphemeral arg);
    static HandshakeEphemeral from_type(uint8_t arg);
    static const char* to_string(HandshakeEphemeral arg);
};

}

#endif