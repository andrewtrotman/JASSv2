/*
	COMPRESS_INTEGER_BITPACK_32_REDUCED.CPP
	---------------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_bitpack_32_reduced.h"

namespace JASS
	{
		/*
		COMPRESS_INTEGER_BITPACK_32_REDUCED::BITS_TO_USE_COMPLETE[]
		-----------------------------------------------------------
		Given the number of bits needed to store the integers, return the actual width to use.  This
		happens when, for example, you can get away with 9 bits, but since 9 * 3 = 27 and 10 * 3 = 30, you
		may as well use 10 bits.
	*/
	const uint32_t compress_integer_bitpack_32_reduced::bits_to_use_complete[] =			///< bits to use (column 1 ) for bits in integer (right column)
		{
		1, //0
		1, //1
		2, //2
		3, //3
		4, //4
		5, //5
		8, //6
		8, //7
		8, //8
		16, //9
		16, //10
		16, //11
		16, //12
		16, //13
		16, //14
		16, //15
		16, //16
		32, //17
		32, //18
		32, //19
		32, //20
		32, //21
		32, //22
		32, //23
		32, //24
		32, //25
		32, //26
		32, //27
		32, //28
		32, //29
		32, //30
		32, //31
		32  //32
		};

	/*
		COMPRESS_INTEGER_BITPACK_32_REDUCED::SELECTOR_TO_USE_COMPLETE[]
		---------------------------------------------------------------
		Given the width in bits, which selector should be used?  This is used to ensure
		a switch() statement has all the entries 0..n with no gaps.
	*/
	const uint32_t compress_integer_bitpack_32_reduced::selector_to_use_complete[] =			///< selector to use (column 1 ) for bits in integer (right column)
		{
		0, //0
		0, //1
		1, //2
		2, //3
		3, //4
		4, //5
		5, //6
		5, //7
		5, //8
		6, //9
		6, //10
		6, //11
		6, //12
		6, //13
		6, //14
		6, //15
		6, //16
		7, //17
		7, //18
		7, //19
		7, //20
		7, //21
		7, //22
		7, //23
		7, //24
		7, //25
		7, //26
		7, //27
		7, //28
		7, //29
		7, //30
		7, //31
		7  //32
		};
	/*
		COMPRESS_INTEGER_BITPACK_32_REDUCED::DECODE()
		---------------------------------------------
	*/
	void compress_integer_bitpack_32_reduced::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint32_t *into = (uint32_t *)decoded;
		const uint8_t *source = (uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		static const uint32_t mask_16 = 0x0000ffff;								///< AND mask for 16-bit integers
		static const uint32_t mask_8  = 0x000000ff;								///< AND mask for 8-bit integers
		static const uint32_t mask_5  = 0x0000001f;								///< AND mask for 5-bit integers
		static const uint32_t mask_4  = 0x0000000f;								///< AND mask for 4-bit integers
		static const uint32_t mask_3  = 0x00000007;								///< AND mask for 3-bit integers
		static const uint32_t mask_2  = 0x00000003;								///< AND mask for 2-bit integers
		static const uint32_t mask_1  = 0x00000001;								///< AND mask for 1-bit integers

		while (source < end_of_source)
			{
			uint32_t width = *source;
			source++;
			uint32_t data = *((uint32_t *)source);
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
					*(into + 0) = data & mask_8;
					data >>= 8;
					*(into + 1) = data & mask_8;
					data >>= 8;
					*(into + 2) = data & mask_8;
					data >>= 8;
					*(into + 3) = data & mask_8;

					into += 4;
					break;
				case 6:
					*(into + 0) = data & mask_16;
					data >>= 16;
					*(into + 1) = data & mask_16;

					into += 2;
					break;
				case 7:
					*into = data;

					into++;
					break;
				}
			source += sizeof(uint32_t);
			}
		}
	}




