
#include "ssp21/link/LinkFormatter.h"

#include "ssp21/link/LinkConstants.h"
#include "ssp21/link/CastagnoliCRC32.h"

#include "openpal/util/Comparisons.h"
#include "openpal/serialization/BigEndian.h"



using namespace openpal;

namespace ssp21
{
	RSlice LinkFormatter::write(WSlice dest, const Message& message)
    {
		const auto start = dest.as_rslice();

        if (dest.length() < consts::min_link_frame_size)
        {
            return RSlice::empty_slice();
        }

        const uint32_t max_payload_length = min<uint32_t>(
                                                // don't let the user write anything bigger than configured maximum
                                                consts::max_config_link_payload_size,
                                                // maximum determined by the output buffer
                                                dest.length() - consts::min_link_frame_size
                                            );

        if (message.payload.length() > max_payload_length)
        {
            return RSlice::empty_slice();
        }

        // safe cast since we've already validated the payload length relative to the maximum allowed
		const auto payload_length = static_cast<uint16_t>(message.payload.length());

		BigEndian::write(
			dest,
			consts::sync1,
			consts::sync2,
			message.addresses.destination,
			message.addresses.source,
			payload_length
		);

		const auto crc_h = CastagnoliCRC32::calc(start.take(consts::link_header_fields_size));

		UInt32::write_to(dest, crc_h);

		// copy the payload
		message.payload.copy_to(dest);

        // append the body crc
        UInt32::write_to(dest, CastagnoliCRC32::calc(message.payload));

        return start.take(consts::min_link_frame_size + payload_length);
    }
}

