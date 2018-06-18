/*
	COMPRESS_INTEGER_BITPACK_64.CPP
	-------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_bitpack_64.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_BITPACK_64::DECODE()
		-------------------------------------
	*/
	void compress_integer_bitpack_64::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint64_t *into = (uint64_t *)decoded;
		const uint8_t *source = (uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		static const uint64_t mask_16 = 0x0000ffff0000ffff;								///< AND mask for 16-bit integers
		static const uint64_t mask_10 = 0x000003ff000003ff;								///< AND mask for 10-bit integers
		static const uint64_t mask_8  = 0x000000ff000000ff;								///< AND mask for 8-bit integers
		static const uint64_t mask_6  = 0x0000003f0000003f;								///< AND mask for 6-bit integers
		static const uint64_t mask_5  = 0x0000001f0000001f;								///< AND mask for 5-bit integers
		static const uint64_t mask_4  = 0x0000000f0000000f;								///< AND mask for 4-bit integers
		static const uint64_t mask_3  = 0x0000000700000007;								///< AND mask for 3-bit integers
		static const uint64_t mask_2  = 0x0000000300000003;								///< AND mask for 2-bit integers
		static const uint64_t mask_1  = 0x0000000100000001;								///< AND mask for 1-bit integers

		while (source < end_of_source)
			{
			uint64_t width = *source;
			source++;
			uint64_t data = *((uint64_t *)source);
			switch (width)
				{
				case 0:
					*(into + 0) = data & mask_1;
					data >>= 1;
					*(into + 1) = data & mask_1;
					data >>= 1;
					*(into + 2) = data & mask_1;
					data >>= 1;
					*(into + 3) = data & mask_1;
					data >>= 1;
					*(into + 4) = data & mask_1;
					data >>= 1;
					*(into + 5) = data & mask_1;
					data >>= 1;
					*(into + 6) = data & mask_1;
					data >>= 1;
					*(into + 7) = data & mask_1;
					data >>= 1;
					*(into + 8) = data & mask_1;
					data >>= 1;
					*(into + 9) = data & mask_1;
					data >>= 1;
					*(into + 10) = data & mask_1;
					data >>= 1;
					*(into + 11) = data & mask_1;
					data >>= 1;
					*(into + 12) = data & mask_1;
					data >>= 1;
					*(into + 13) = data & mask_1;
					data >>= 1;
					*(into + 14) = data & mask_1;
					data >>= 1;
					*(into + 15) = data & mask_1;
					data >>= 1;
					*(into + 16) = data & mask_1;
					data >>= 1;
					*(into + 17) = data & mask_1;
					data >>= 1;
					*(into + 18) = data & mask_1;
					data >>= 1;
					*(into + 19) = data & mask_1;
					data >>= 1;
					*(into + 20) = data & mask_1;
					data >>= 1;
					*(into + 21) = data & mask_1;
					data >>= 1;
					*(into + 22) = data & mask_1;
					data >>= 1;
					*(into + 23) = data & mask_1;
					data >>= 1;
					*(into + 24) = data & mask_1;
					data >>= 1;
					*(into + 25) = data & mask_1;
					data >>= 1;
					*(into + 26) = data & mask_1;
					data >>= 1;
					*(into + 27) = data & mask_1;
					data >>= 1;
					*(into + 28) = data & mask_1;
					data >>= 1;
					*(into + 29) = data & mask_1;
					data >>= 1;
					*(into + 30) = data & mask_1;
					data >>= 1;
					*(into + 31) = data & mask_1;

					into += 32;
					break;
				case 1:
					*(into + 0) = data & mask_2;
					data >>= 2;
					*(into + 1) = data & mask_2;
					data >>= 2;
					*(into + 2) = data & mask_2;
					data >>= 2;
					*(into + 3) = data & mask_2;
					data >>= 2;
					*(into + 4) = data & mask_2;
					data >>= 2;
					*(into + 5) = data & mask_2;
					data >>= 2;
					*(into + 6) = data & mask_2;
					data >>= 2;
					*(into + 7) = data & mask_2;
					data >>= 2;
					*(into + 8) = data & mask_2;
					data >>= 2;
					*(into + 9) = data & mask_2;
					data >>= 2;
					*(into + 10) = data & mask_2;
					data >>= 2;
					*(into + 11) = data & mask_2;
					data >>= 2;
					*(into + 12) = data & mask_2;
					data >>= 2;
					*(into + 13) = data & mask_2;
					data >>= 2;
					*(into + 14) = data & mask_2;
					data >>= 2;
					*(into + 15) = data & mask_2;

					into += 16;
					break;
				case 2:
					*(into + 0) = data & mask_3;
					data >>= 3;
					*(into + 1) = data & mask_3;
					data >>= 3;
					*(into + 2) = data & mask_3;
					data >>= 3;
					*(into + 3) = data & mask_3;
					data >>= 3;
					*(into + 4) = data & mask_3;
					data >>= 3;
					*(into + 5) = data & mask_3;
					data >>= 3;
					*(into + 6) = data & mask_3;
					data >>= 3;
					*(into + 7) = data & mask_3;
					data >>= 3;
					*(into + 8) = data & mask_3;
					data >>= 3;
					*(into + 9) = data & mask_3;

					into += 10;
					break;
				case 3:
					*(into + 0) = data & mask_4;
					data >>= 4;
					*(into + 1) = data & mask_4;
					data >>= 4;
					*(into + 2) = data & mask_4;
					data >>= 4;
					*(into + 3) = data & mask_4;
					data >>= 4;
					*(into + 4) = data & mask_4;
					data >>= 4;
					*(into + 5) = data & mask_4;
					data >>= 4;
					*(into + 6) = data & mask_4;
					data >>= 4;
					*(into + 7) = data & mask_4;

					into += 8;
					break;
				case 4:
					*(into + 0) = data & mask_5;
					data >>= 5;
					*(into + 1) = data & mask_5;
					data >>= 5;
					*(into + 2) = data & mask_5;
					data >>= 5;
					*(into + 3) = data & mask_5;
					data >>= 5;
					*(into + 4) = data & mask_5;
					data >>= 5;
					*(into + 5) = data & mask_5;

					into += 6;
					break;
				case 5:
					*(into + 0) = data & mask_6;
					data >>= 6;
					*(into + 1) = data & mask_6;
					data >>= 6;
					*(into + 2) = data & mask_6;
					data >>= 6;
					*(into + 3) = data & mask_6;
					data >>= 6;
					*(into + 4) = data & mask_6;

					into += 5;
					break;
				case 6:
					*(into + 0) = data & mask_8;
					data >>= 8;
					*(into + 1) = data & mask_8;
					data >>= 8;
					*(into + 2) = data & mask_8;
					data >>= 8;
					*(into + 3) = data & mask_8;

					into += 4;
					break;
				case 7:
					*(into + 0) = data & mask_10;
					data >>= 10;
					*(into + 1) = data & mask_10;
					data >>= 10;
					*(into + 2) = data & mask_10;

					into += 3;
					break;
				case 8:
					*(into + 0) = data & mask_16;
					data >>= 16;
					*(into + 1) = data & mask_16;

					into += 2;
					break;
				case 9:
					*into = data;

					into++;
					break;
				}
			source += sizeof(uint64_t);
			}
		}
	}




