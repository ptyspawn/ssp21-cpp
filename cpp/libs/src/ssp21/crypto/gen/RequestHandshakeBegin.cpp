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

#include "ssp21/crypto/gen/RequestHandshakeBegin.h"

#include "ssp21/crypto/MessageParser.h"
#include "ssp21/crypto/MessagePrinting.h"
#include "ssp21/crypto/MessageFormatter.h"

namespace ssp21 {

RequestHandshakeBegin::RequestHandshakeBegin()
{}

RequestHandshakeBegin::RequestHandshakeBegin(
    uint16_t version,
    const CryptoSpec& spec,
    CertificateMode certificate_mode,
    const Seq8& ephemeral_public_key
) :
    version(version),
    spec(spec),
    certificate_mode(certificate_mode),
    ephemeral_public_key(ephemeral_public_key)
{}


ParseError RequestHandshakeBegin::read(openpal::RSlice input)
{
    auto read_fields = [this](openpal::RSlice& input) -> ParseError 
    {
        return MessageParser::read_fields(
            input,
            version,
            spec,
            certificate_mode,
            ephemeral_public_key,
            certificates
        );
    };

    return MessageParser::read_message(input, Function::request_handshake_begin, read_fields);
}

FormatResult RequestHandshakeBegin::write(openpal::WSlice output) const
{
    auto write_fields = [this](openpal::WSlice& output) -> FormatError 
    {
        return MessageFormatter::write_fields(
            output,
            version,
            spec,
            certificate_mode,
            ephemeral_public_key,
            certificates
        );
    };

    return MessageFormatter::write_message(output, Function::request_handshake_begin, write_fields);
}
void RequestHandshakeBegin::print(IMessagePrinter& printer) const
{
    MessagePrinting::print_fields(
        printer,
        "version",
        version,
        "spec",
        spec,
        "certificate_mode",
        certificate_mode,
        "ephemeral_public_key",
        ephemeral_public_key,
        "certificates",
        certificates
    );
}


}
