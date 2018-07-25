/*
	COMPRESS_INTEGER_ELIAS_DELTA.CPP
	--------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <immintrin.h>

#include "maths.h"
#include "compress_integer_elias_delta.h"

namespace JASS
	{
	/*
		OMPRESS_INTEGER_ELIAS_DELTA::ENCODE()
		--------------------------------------
	*/
	size_t compress_integer_elias_delta::encode(void *encoded_as_void, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		uint8_t *encoded = static_cast<uint8_t *>(encoded_as_void);

		/*
			Zero the destination array
		*/
		memset(encoded, 0, encoded_buffer_length);

		/*
			encode
		*/
		uint64_t into = 0;									// bit position to write into (counted from the beginning of encoded).
		for (const integer *value = source; value < source + source_integers; value++)
			{
			/*
				Get the length
			*/
			uint32_t n = maths::floor_log2(*value) + 1;

			/*
				Get the unary part of the length.  The binary part is already stored in n
			*/
			uint32_t unary = maths::floor_log2(n);


			/*
				Write unary numner of 0-bits (no write necessary as the bits are already 0).
			*/
			into += unary;

			/*
				Move the high bit of the integer to the low bit so that we can use that bit as the end of the unary
				This is because we rely on storing the data from the low end of the integer to the high end so that we can dip in at any byte
				and have the correct sequence of bits.  It also means we can return the size used in bytes rather than as 32-bit integers (so we
				can truncate the byte sequence at any point).  It also appears to reduce the complexity of decoding.
			*/
			uint64_t zig_zag = ((n & ~(1 << unary)) << 1) + 1;

			/*
				Append the value in binary
			*/
			size_t shift = into % 8;
			uint64_t pattern = zig_zag << shift;
			uint64_t *address = reinterpret_cast<uint64_t *>(encoded + (into / 8));
			*address |= pattern;

			into += unary + 1;

			/*
				We now know the length of the integer so we can encode the actual value in binary (without zig-zagging)
			*/
			shift = into % 8;
			pattern = *value << shift;
			address = reinterpret_cast<uint64_t *>(encoded + (into / 8));
			*address |= pattern;

			into += n + 1;
			}

		return ((into + 7) / 8);
		}

	/*
		OMPRESS_INTEGER_ELIAS_DELTA::DECODE()
		--------------------------------------
	*/
	void compress_integer_elias_delta::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint64_t bits_used = 0;

		for (integer *end = decoded + integers_to_decode; decoded < end; decoded++)
			{
			/*
				Read as many bits as we can from the lowest possible address.
				With the gamma code, a 32-bit integer uses 31 bits for the unary and 32 bits for the binary and if not aligned with the
				start of a byte extra bits might be needed.  With the delta code the worst case is 32 bits for the binary, 6 bits for the unary,
				the 6 bits for the length (total is 44), which means no extra bits are ever needed if a 64-bit read is performed (even if
				the integer starts at bit position 8, because 44+8 < 64).
			*/
			const uint8_t *address = static_cast<const uint8_t *>(source_as_void) + (bits_used / 8);
			const uint64_t *source = reinterpret_cast<const uint64_t *>(address);
			/*
				dismiss the bits we're already used
			*/
			uint64_t value = *source >> (bits_used % 8);

			/*
				get the width of the width
			*/
			uint64_t unary = _tzcnt_u64(value);

			/*
				get the zig-zag encoded length of the integer
			*/
			uint64_t zig_zag = _bextr_u64(value, unary, unary + 1);

			/*
				unzig-zag it
			*/
			uint32_t hight_bit = 1UL << unary;
			uint32_t binary = (zig_zag >> 1) | hight_bit;

			uint64_t result = _bextr_u64(value, unary + unary + 1, binary);

			*decoded = result;

			/*
				Remember how much we've already used
			*/
			bits_used += unary + unary + 1 + binary;
			}
		}

	/*
		OMPRESS_INTEGER_ELIAS_DELTA::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_elias_delta::unittest(void)
		{
		compress_integer_elias_delta codec;

		std::vector<uint8_t> buffer(1024);
		std::vector<integer> sequence = {2, 3, 4, 5, 6, 7, 8, 9, 10};
		std::vector<integer> into(sequence.size());

		auto encoded_length = codec.encode(&buffer[0], buffer.size(), &sequence[0], sequence.size());
		codec.decode(&into[0], sequence.size(), &buffer[0], encoded_length);

		JASS_assert(into == sequence);

//		compress_integer::unittest(compress_integer_elias_gamma(), false);
		puts("compress_integer_elias_delta::PASSED");
		}
	}
