/*
	COMPRESS_INTEGER_ELIAS_DELTA.CPP
	--------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <immintrin.h>

#include "maths.h"
#include "compress_integer_elias_delta.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_ELIAS_DELTA::ENCODE()
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
				We now know the length of the integer so we can encode the actual value in binary (without zig-zagging) with the high bit turned off
			*/
			shift = into % 8;
			pattern = (*value & ~(1ULL << (n - 1))) << shift;
			address = reinterpret_cast<uint64_t *>(encoded + (into / 8));
			*address |= pattern;

			into += n - 1;
			}

		return ((into + 7) / 8);
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA::DECODE()
		--------------------------------------
	*/
	void compress_integer_elias_delta::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		const uint64_t *source = reinterpret_cast<const uint64_t *>(source_as_void);
		uint64_t value = 0;
		uint8_t unary;
		uint8_t binary;
		int8_t bits_used;		/// bits used in the second word if we cross a word boundary (and not kept from block to block)
		int8_t bits_remaining = 0;

		for (integer *end = decoded + integers_to_decode; decoded < end; decoded++)
			{
			/*
				get the width of the width
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
					The unary part splits a machine word
				*/
				value = *source++;
				bits_used = (uint8_t)_tzcnt_u64(value);
				unary = bits_remaining + bits_used;
				value >>= bits_used;
				bits_remaining = 64 - bits_used;
				}

			/*
				get the zig-zag encoded length of the integer and un-zig-zag it.
			*/
			if (bits_remaining - unary > 0)
				{
				binary = (uint8_t)((_bextr_u64(value, 0, unary + 1) >> 1)) | (1 << unary);		// un-zig-zag
				bits_remaining -= unary + 1;
				value >>= unary + 1;
				}
			else
				{
				/*
					The binary splits a machine word
				*/
				binary = (uint8_t)value;
				value = *source++;
				binary |= (uint8_t)(_bextr_u64(value, 0, unary - bits_remaining + 1) << bits_remaining);
				binary = (binary >> 1) | (1UL << unary);				// un-zig-zag
				bits_used = unary - bits_remaining + 1;
				bits_remaining = 64 - bits_used;
				value >>= bits_used;
				}

			/*
				get the binary value that is encoded using elias delta
			*/
			if (bits_remaining - binary >= 0)
				{
				*decoded = (integer)(_bextr_u64(value, 0, binary)) | (1 << (binary - 1));
				bits_remaining -= binary - 1;
				value >>= binary - 1;
				}
			else
				{
				/*
					the encoded number splits a machine word
				*/
				*decoded = (uint32_t)value | (1 << (binary - 1));
				value = *source++;
				*decoded |= (_bextr_u64(value, 0, binary - bits_remaining) << bits_remaining);
				bits_used = binary - bits_remaining - 1;
				bits_remaining = 64 - bits_used;
				value >>= bits_used;
				}
			}
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_elias_delta::unittest(void)
		{
		compress_integer_elias_delta *codec = new compress_integer_elias_delta;

		std::vector<uint8_t> buffer(1024);
		std::vector<integer> sequence = {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, };
		std::vector<integer> into(sequence.size());

		auto encoded_length = codec->encode(&buffer[0], buffer.size(), &sequence[0], sequence.size());
		codec->decode(&into[0], sequence.size(), &buffer[0], encoded_length);

		JASS_assert(into == sequence);

		compress_integer_elias_delta *compressor = new compress_integer_elias_delta;
		compress_integer::unittest(*compressor, 1);
		delete compressor;
		delete codec;
		puts("compress_integer_elias_delta::PASSED");
		}
	}
