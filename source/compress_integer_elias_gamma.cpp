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

		/*
			Zero the destinarin array
		*/
		memset(encoded, 0, encoded_buffer_length);

		/*
			encode
		*/
		uint32_t into = 0;									// bit position to write into (counted from the beginning of encoded).
		for (const integer *value = source; value < source + source_integers; value++)
			{
			uint32_t n;
			n = maths::floor_log2(*value);

			/*
				Write n 0-bits (no write necessary as the bits are already 0).
			*/
			into += n;

			/*
				Append the value in binary
			*/
			uint32_t shift = 31 - (into % 32) - n;
			uint32_t pattern = static_cast<uint64_t>(*value) << shift;
			uint32_t *address = reinterpret_cast<uint32_t *>(encoded + (into / 8));
			*address |= pattern;
			into += n + 1;
			}

		return ((into + 7) / 8);
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::DECODE()
		--------------------------------------
	*/
	void compress_integer_elias_gamma::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		integer *end = decoded + integers_to_decode;
		uint64_t bits_used = 0;
		do
			{
			const uint32_t *source = reinterpret_cast<const uint32_t *>(source_as_void) + (bits_used / 32);
			uint32_t value = *source;

			uint32_t unary = _lzcnt_u32(value);
			uint32_t binary = _bextr_u32(value, 31 - unary - unary, unary + 1);

			value <<= unary + unary + 1;
			bits_used = unary + unary + 1;

			*decoded = binary;
			decoded++;
			}
		while (decoded < end);
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_elias_gamma::unittest(void)
		{
		compress_integer_elias_gamma codec;

		std::vector<uint8_t> buffer(1024);
		std::vector<integer> sequence  {16, 15, 14, 13, 12};
		std::vector<integer> into(sequence.size());

		auto encoded_length = codec.encode(&buffer[0], buffer.size(), &sequence[0], sequence.size());
		codec.decode(&into[0], sequence.size(), &buffer[0], encoded_length);

		JASS_assert(into == sequence);
		puts("compress_integer_elias_gamma::PASSED");
		}
	}
