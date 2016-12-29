
#ifndef SSP21_LINKFORMATTER_H
#define SSP21_LINKFORMATTER_H

#include "openpal/util/Uncopyable.h"

#include "openpal/container/RSlice.h"
#include "openpal/container/WSlice.h"

#include "ssp21/link/Addresses.h"

#include "ssp21/crypto/SequenceTypes.h"

namespace ssp21
{
    class LinkFormatter : private openpal::StaticOnly
    {
    public:

        // returns an empty slice if there wasn't sufficient space to write the frame
        static openpal::RSlice write(openpal::WSlice dest, const Addresses& addr, const Seq32& payload);
    };
}

#endif
