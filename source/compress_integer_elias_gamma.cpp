/*
	COMPRESS_INTEGER_ELIAS_GAMMA.CPP
	--------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <immintrin.h>

#include "maths.h"
#include "compress_integer_elias_gamma.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::ENCODE()
		--------------------------------------
	*/
	size_t compress_integer_elias_gamma::encode(void *encoded_as_void, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		uint8_t *encoded = static_cast<uint8_t *>(encoded_as_void);
		uint32_t into = 0;
		auto end = source + source_integers;

		for (const integer *value = source; value < end; value++)
			{
			uint32_t n;
			n = maths::floor_log2(*value);

			/*
				Write n 0-bits

				WRONGLY ASSUMES ENCODED == 0!  FIX THIS
			*/
			into += n;

			/*
				Append the value in binary
			*/
			uint32_t pattern = *value << (into % 8);
			uint32_t *address = reinterpret_cast<uint32_t *>(encoded + (into / 8));
			*address |= pattern;
			}

		return ((into + 7) / 8);
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::DECODE()
		--------------------------------------
	*/
	void compress_integer_elias_gamma::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		const uint8_t *source = reinterpret_cast<const uint8_t *>(source_as_void);
		uint32_t unary = _lzcnt_u32(*source);
		uint32_t binary = _bextr_u32(*source, 0, unary);
		*decoded = binary;
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_elias_gamma::unittest(void)
		{
		compress_integer_elias_gamma codec;

		uint8_t buffer[1024];
		uint32_t into[1024];
		integer sequence = 45;

		auto encoded_length = codec.encode(buffer, sizeof(buffer), &sequence, 1);
		codec.decode(into, 1, buffer, encoded_length);

		JASS_assert(into[0] == sequence);
		puts("compress_integer_elias_gamma::PASSED");
		}
	}
