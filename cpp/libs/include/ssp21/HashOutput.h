
#ifndef SSP21_HASHOUTPUT_H
#define SSP21_HASHOUTPUT_H

#include "SecureBuffer.h"
#include "Constants.h"

#include <openpal/util/Uncopyable.h>

namespace ssp21
{			
	enum class HashOutputType
	{
		EMPTY,
		SHA256
	};


	class HashOutput : private openpal::Uncopyable
	{
		public:

			HashOutput();

			openpal::RSlice as_slice() const;
			
			HashOutputType get_type() const;

			openpal::WSlice get_write_slice();
			
			void set_type(HashOutputType type);

		private:	

			static uint32_t get_length(HashOutputType);

			uint32_t length_;
			HashOutputType hash_type_;
			SecureBuffer<consts::MAX_HASH_OUTPUT_SIZE> buffer_;
	};	
}

#endif