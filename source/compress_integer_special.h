/*
	COMPRESS_INTEGER_SPECIAL.H
	--------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The null compressor for integer sequences (i.e. just copy, don't compress).
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	class compress_integer_special : public compress_integer
		{
		public:
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
				{
				size_t used = sizeof(*source) * source_integers;
				if (used > encoded_buffer_length)
					return 0;

				::memcpy(encoded, source, used);

				return used;
				}

			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length)
				{
				::memcpy(decoded, source, source_length);
				}

			template <typename WRITER>
			void decode_with_writer(WRITER &writer, size_t integers_to_decode, const void *source, size_t source_length)
				{
				integer cumulative_sum = 0;
				integer *current = (integer *)source;
				integer *end = current + integers_to_decode;

				while (current < end)
					{
					cumulative_sum += *current;
					current++;
					writer.push_back(cumulative_sum);
					}
				}
		} ;
}
