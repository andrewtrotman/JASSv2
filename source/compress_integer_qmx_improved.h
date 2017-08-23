/*
	COMPRESS_INTEGER_QMX_IMPROVED.H
	-------------------------------
	Copyright (c) 2014-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	Improved by:
		Not having the variable-byte crud on the end
		not requiring SIMD-word alignment.
		The removal of crud not necessary for real use (e.g. stats)
		short_end_block is always true
*/
#pragma once

#include <stdint.h>
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_QMX_IMPROVED
		-----------------------------------
	*/
	class compress_integer_qmx_improved : public compress_integer
		{
		private:
			uint8_t *length_buffer;
			uint64_t length_buffer_length;

		public:
			compress_integer_qmx_improved() :
				length_buffer(nullptr),
				length_buffer_length(0)
				{
				/* Nothing */
				}
				
			virtual ~compress_integer_qmx_improved()
				{
				delete [] length_buffer;
				}

			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);
		};
	}

