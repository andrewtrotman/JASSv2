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
			zero the destination array
		*/
		memset(encoded, 0, encoded_buffer_length);

		/*
			encode
		*/
		uint64_t into = 0;									// bit position to write into (counted from the beginning of encoded).
		for (const integer *value = source; value < source + source_integers; value++)
			{
			/*
				Get the unary part
			*/
			uint32_t n = maths::floor_log2(*value);

			/*
				Write n 0-bits (no write necessary as the bits are already 0).
			*/
			into += n;

			/*
				Move the high bit of the integer to the low bit so that we can use that bit as the end of the unary
				This is because we rely on storing the data from the low end of the integer to the high end so that we can dip in at any byte
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
		const uint64_t *source = reinterpret_cast<const uint64_t *>(source_as_void);
		uint64_t bits_used = 0;
		uint64_t bits_remaining = 0;
		uint64_t value = 0;
		uint8_t unary;

		for (integer *end = decoded + integers_to_decode; decoded < end; decoded++)
			{
			/*
				get the width
			*/
			if (value != 0)
				{
				unary = (uint8_t)_tzcnt_u64(value);
				value >>= unary;
				bits_remaining -= unary;
				}
			else
				{
				/*
					the length splits a machine word
				*/
				unary = (uint8_t)bits_remaining;
				value = *source++;
				bits_used = (uint8_t)_tzcnt_u64(value);
				unary += (uint8_t)bits_used;
				value >>= bits_used;
				bits_remaining = 64 - bits_used;
				}

			/*
				get the zig-zag encoded binary, unzig-zag it and store it
			*/
			if (bits_remaining > unary)
				{
				*decoded = (integer)((_bextr_u64(value, 0, unary + 1) >> 1)) | (1UL << unary);
				bits_remaining -= unary + 1;
				value >>= unary + 1;
				}
			else
				{
				/*
					the encoded number splits a machine word
				*/
				*decoded = (integer)value;
				value = *source++;
				*decoded |= (integer)((_bextr_u64(value, 0, (uint32_t)(unary - bits_remaining + 1))) << bits_remaining);
				bits_used = unary - bits_remaining + 1;
				bits_remaining = 64 - bits_used;
				value >>= bits_used;
				}
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

		compress_integer::unittest(compress_integer_elias_gamma(), 1);
		puts("compress_integer_elias_gamma::PASSED");
		}
	}
