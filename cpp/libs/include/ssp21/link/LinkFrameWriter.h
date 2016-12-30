
#ifndef SSP21_LINKFRAMEWRITER_H
#define SSP21_LINKFRAMEWRITER_H

#include "ssp21/IFrameWriter.h"

#include "openpal/container/Buffer.h"

#include "ssp21/link/LinkConstants.h"
#include "ssp21/link/Addresses.h"

namespace ssp21
{
    
    class LinkFrameWriter final : public IFrameWriter
    {

    public:        

		LinkFrameWriter(Addresses addr, uint16_t max_payload_size);

		virtual WriteResult write(IWritable& payload) override;		

		virtual uint16_t get_max_payload_size() const override
		{
			return max_payload_size;
		}

	private:		

		FormatResult write_body_and_crc(IWritable& payload);

		static constexpr uint32_t get_buffer_size(uint16_t max_payload_size)
		{
			return static_cast<uint32_t>(consts::link::min_frame_size) + static_cast<uint32_t>(max_payload_size);
		}

		seq32_t last_frame;

		Addresses addr;
		uint16_t max_payload_size;
		openpal::Buffer frame_buffer;



    };

}

#endif
