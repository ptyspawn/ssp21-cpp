
#ifndef SSP21_SEQBYTEFIELD_H
#define SSP21_SEQBYTEFIELD_H

#include "ser4cpp/serialization/BigEndian.h"

#include "ssp21/util/SequenceTypes.h"

#include "ssp21/crypto/gen/FormatError.h"
#include "ssp21/crypto/gen/ParseError.h"

#include "ssp21/crypto/IMessagePrinter.h"
#include "ssp21/crypto/IntegerField.h"

#include "ssp21/crypto/VLength.h"

namespace ssp21 {
class SeqByteField final : public seq32_t {
public:
    size_t size() const
    {
        return VLength::size(this->length()) + this->length();
    }

    SeqByteField() {}

    SeqByteField& operator=(const seq32_t& other)
    {
        seq32_t::operator=(other);
        return *this;
    }

    explicit SeqByteField(const seq32_t& value)
        : seq32_t(value)
    {
    }

    ParseError read(seq32_t& input)
    {
        uint32_t length;
        const auto err = VLength::read(length, input);
        if (any(err))
            return err;

        if (input.length() < length) {
            return ParseError::insufficient_bytes;
        }

        *this = input.take(length);
        input.advance(length);
        return ParseError::ok;
    }

    FormatError write(wseq32_t& dest) const
    {
        const auto err = VLength::write(this->length(), dest);
        if (any(err))
            return err;

        if (dest.length() < this->length())
            return FormatError::insufficient_space;

        dest.copy_from(*this);

        return FormatError::ok;
    }

    void print(const char* name, IMessagePrinter& printer) const
    {
        printer.print(name, *this);
    }
};

}

#endif
