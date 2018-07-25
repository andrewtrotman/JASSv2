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
		uint64_t into = 0;									// bit position to write into (counted from the beginning of encoded).
		for (const integer *value = source; value < source + source_integers; value++)
			{
			uint32_t n;
			n = maths::floor_log2(*value);

			/*
				Write n 0-bits (no write necessary as the bits are already 0).
			*/
			into += n;

			/*
				Move the high bit of the integer to the low bit so that we can use that bit as the end of the unary
				This is because we rely on storing the data from the low end of the integer to the hight end so that we can dip in at any byte
				and have the correct sequence of bits.  It also means we can return the size used in bytes rather than as 32-bit integers (so we
				can truncate the byte sequence at any point).  It also appears to reduce the complexity of decoding.
			*/
			uint64_t zig_zag = ((*value & ~(1 << n)) << 1) + 1;

			/*
				Append the value in binary
			*/
			size_t shift = into % 8;
			uint64_t pattern = zig_zag << shift;
			uint64_t *address = reinterpret_cast<uint64_t *>(encoded + (into / 8));
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
		uint64_t bits_used = 0;

		for (integer *end = decoded + integers_to_decode; decoded < end; decoded++)
			{
			/*
				read as many bits as we can from the lowest possible address
			*/
			const uint8_t *address = static_cast<const uint8_t *>(source_as_void) + (bits_used / 8);
			const uint64_t *source = reinterpret_cast<const uint64_t *>(address);
			/*
				but we might not be aligned with the start of the word so read the next word to get the top few bits
			*/
			uint64_t extra_byte = *reinterpret_cast<const uint64_t *>(address + 1);

			/*
				dismiss the bits we're already used
			*/
			uint64_t value = *source >> (bits_used % 8);

			/*
				Append the next byte because 0x80000000 uses 63 bits (31 + 31 + 1), so if we have a string of 31-bit integers than they won't all fit!
			*/
			value |= (extra_byte << (8 - (bits_used % 8))) & 0xFF00000000000000;

			/*
				get the width
			*/
			uint64_t unary = _tzcnt_u64(value);

			/*
				get the zig-zag encoded binary
			*/
			uint64_t zig_zag = _bextr_u64(value, unary, unary + 1);

			/*
				unzig-zag it and store it
			*/
			uint32_t hight_bit = 1UL << unary;
			uint32_t binary = (zig_zag >> 1) | hight_bit;
			*decoded = binary;

			/*
				Remember how much we've already used
			*/
			bits_used += unary + unary + 1;
			}
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_elias_gamma::unittest(void)
		{
		compress_integer_elias_gamma codec;

		std::vector<uint8_t> buffer(1024);
		std::vector<integer> sequence = {1073741823, 1073741823, 1073741823, 1073741823, 1073741823, 1073741823, 1073741823, 1073741823};
		std::vector<integer> into(sequence.size());

		auto encoded_length = codec.encode(&buffer[0], buffer.size(), &sequence[0], sequence.size());
		codec.decode(&into[0], sequence.size(), &buffer[0], encoded_length);

		JASS_assert(into == sequence);

		compress_integer::unittest(compress_integer_elias_gamma(), false);
		puts("compress_integer_elias_gamma::PASSED");
		}
	}
