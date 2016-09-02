
#ifndef SSP21_IMESSAGE_H
#define SSP21_IMESSAGE_H

#include "ssp21/crypto/IMessagePrinter.h"
#include "ssp21/crypto/FormatResult.h"

#include "ssp21/gen/ParseError.h"

namespace ssp21
{
    /**
    * Messages can be read, written, and printed
    */
    class IMessage
    {

    public:

        virtual ParseError read(openpal::RSlice input) = 0;

        virtual FormatResult write(openpal::WSlice output) const = 0;

        virtual void print(IMessagePrinter& printer) const = 0;

    };

}

#endif
