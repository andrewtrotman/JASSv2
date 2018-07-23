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
				Write n 0-bits
			*/
			into += n;

			/*
				zig-zag the value - rotate the bits, placing the high bit at the low end (technically, a left circular shift of the low n bits).  We do this so that the unary ends with a 1, and that we don't store that 1 more than once.
			*/
			uint32_t zig_zag = ((*value & ~(1UL << n)) << 1) + 1;

			/*
				Append the value in binary
			*/
			uint32_t pattern = zig_zag << (into % 8);
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
		const uint32_t *source = reinterpret_cast<const uint32_t *>(source_as_void);
		uint64_t value = *source;

		do
			{
			uint32_t unary = _tzcnt_u64(value);
			uint32_t binary = _bextr_u32(value, unary + 1, unary);
			value >>= unary + unary + 1;

			*decoded = binary + (1 << unary);
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
		std::vector<integer> sequence  {1, 2, 3, 4, 5};
		std::vector<integer> into(sequence.size());

		auto encoded_length = codec.encode(&buffer[0], buffer.size(), &sequence[0], sequence.size());
		codec.decode(&into[0], sequence.size(), &buffer[0], encoded_length);

		JASS_assert(into == sequence);
		puts("compress_integer_elias_gamma::PASSED");
		}
	}
