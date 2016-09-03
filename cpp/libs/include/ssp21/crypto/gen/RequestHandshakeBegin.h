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

#ifndef SSP21_REQUESTHANDSHAKEBEGIN_H
#define SSP21_REQUESTHANDSHAKEBEGIN_H

#include <cstdint>
#include "ssp21/crypto/gen/DHMode.h"
#include "ssp21/crypto/gen/HashMode.h"
#include "ssp21/crypto/gen/Function.h"
#include "ssp21/crypto/gen/NonceMode.h"
#include "ssp21/crypto/gen/SessionMode.h"
#include "ssp21/crypto/gen/CertificateMode.h"
#include "ssp21/crypto/IMessage.h"
#include "ssp21/crypto/SequenceTypes.h"

namespace ssp21 {

struct RequestHandshakeBegin : public IMessage, private openpal::Uncopyable
{
    RequestHandshakeBegin();

    RequestHandshakeBegin(
        uint16_t version,
        NonceMode nonce_mode,
        DHMode dh_mode,
        HashMode hash_mode,
        SessionMode session_mode,
        CertificateMode certificate_mode,
        const Seq8& ephemeral_public_key
    );

    virtual ParseError read(openpal::RSlice input) override;
    virtual FormatResult write(openpal::WSlice output) const override;
    virtual void print(IMessagePrinter& printer) const override;

    static const uint32_t min_size_bytes = 10;
    static const Function function = Function::request_handshake_begin;

    uint16_t version;
    NonceMode nonce_mode;
    DHMode dh_mode;
    HashMode hash_mode;
    SessionMode session_mode;
    CertificateMode certificate_mode;
    Seq8 ephemeral_public_key;
    Seq8Seq16 certificates;

};

}

#endif
