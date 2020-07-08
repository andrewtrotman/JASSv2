/*
	COMPRESS_INTEGER_SIMPLE_16.CPP
	------------------------------
	Copyright (c) 2014-2017 Blake Burgess and Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>

#include <vector>

#include "maths.h"
#include "compress_integer_simple_16.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_SIMPLE_16::SIMPLE16_SHIFT_TABLE
		------------------------------------------------
		Number of bits to shift across when packing - is sum of prior packed ints (see above)
	*/
	const size_t compress_integer_simple_16::simple16_shift_table[] =
		{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
		0, 2, 4, 6, 8, 10, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 28, 28, 28, 28, 28, 28,
		0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 15, 17, 19, 21, 22, 23, 24, 25, 26, 27, 28, 28, 28, 28, 28, 28, 28,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28, 28, 28, 28, 28, 28, 28,
		0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 4, 7, 10, 13, 16, 19, 22, 25, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 3, 7, 11, 15, 19, 22, 25, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 4, 8, 12, 16, 20, 24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 5, 10, 15, 20, 24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 4, 8, 13, 18, 23, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 6, 12, 18, 23, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 5, 10, 16, 22, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 7, 14, 21, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 10, 19, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 14, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
		0, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
		};

	/*
		COMPRESS_INTEGER_SIMPLE_16::INTS_PACKED_TABLE
		---------------------------------------------
		Number of integers packed into a word, given its mask type
	*/
	const size_t compress_integer_simple_16::ints_packed_table[] =
		{
		28, 21, 21, 21, 14, 9, 8, 7, 6, 6, 5, 5, 4, 3, 2, 1
		};

	/*
		COMPRESS_INTEGER_SIMPLE_16::CAN_PACK_TABLE
		------------------------------------------
		Bitmask map for valid masks at an offset (column) for some num_bits_needed (row).
	*/
	const size_t compress_integer_simple_16::can_pack_table[] =
		{
		0xffff, 0x7fff, 0x3fff, 0x1fff, 0x0fff, 0x03ff, 0x00ff, 0x007f, 0x003f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x000f, 0x000f, 0x000f, 0x000f, 0x000f, 0x000f, 0x000f, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
		0xfff2, 0x7ff2, 0x3ff2, 0x1ff2, 0x0ff2, 0x03f2, 0x00f2, 0x0074, 0x0034, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xffe0, 0x7fe0, 0x3fe0, 0x1fe0, 0x0fe0, 0x03e0, 0x00e0, 0x0060, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xffa0, 0x7fc0, 0x3fc0, 0x1fc0, 0x0fc0, 0x0380, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xfd00, 0x7d00, 0x3f00, 0x1f00, 0x0e00, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xf400, 0x7400, 0x3c00, 0x1800, 0x0800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xf000, 0x7000, 0x3000, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xe000, 0x6000, 0x2000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xe000, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0xc000, 0x4000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		};

	/*
		COMPRESS_INTEGER_SIMPLE_16::INVALID_MASKS_FOR_OFFSET
		----------------------------------------------------
		We AND out masks for offsets where we don't know if we can fully pack for that offset
	*/
	const size_t compress_integer_simple_16::invalid_masks_for_offset[] =
		{
		0x0000, 0x8000, 0xc000, 0xe000, 0xf000, 0xfc00, 0xff00, 0xff80, 0xffc0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xffe0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfff0, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xfffe, 0xffff
		};

	/*
		COMPRESS_INTEGER_SIMPLE_16::ROW_FOR_BITS_NEEDED
		-----------------------------------------------
		Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
	*/
	const size_t compress_integer_simple_16::row_for_bits_needed[] =
		{
		0, 0, 28, 56, 84, 112, 140, 168, 196, 196, 224, 252, 252, 252, 252, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,				// Valid
		308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308 // Overflow
		};

	/*
		COMPRESS_INTEGER_SIMPLE_16::ENCODE()
		------------------------------------
	*/
	size_t compress_integer_simple_16::encode(void *destination, size_t destination_length, const integer *source, size_t source_integers)
		{
		size_t words_in_compressed_string;

		uint32_t *into = reinterpret_cast<uint32_t *>(destination);
		uint32_t *end = reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(destination) + destination_length);
		size_t pos = 0;
		for (words_in_compressed_string = 0; pos < source_integers; words_in_compressed_string++)
			{
			/*
				Check for overflow before it happens
			*/
			if (into + 1 > end)
				return 0;

			size_t remaining = (pos + 28 < source_integers) ? 28 : source_integers - pos;
			size_t last_bitmask = 0x0000;
			size_t bitmask = 0xFFFF;
			
			/*
				constrain last_bitmask to contain only bits for masks we can pack with
			*/
			for (size_t offset = 0; offset < remaining && bitmask; offset++)
				{
				bitmask &= can_pack_table[row_for_bits_needed[maths::ceiling_log2(source[pos + offset])] + offset];
				last_bitmask |= (bitmask & invalid_masks_for_offset[offset + 1]);
				}
				
			/*
				ensure valid input
			*/
			if (last_bitmask == 0)
				return 0;
				
			/*
				get position of lowest set bit
			*/
			uint32_t mask_type = maths::find_first_set_bit((uint32_t)last_bitmask);
			size_t num_to_pack = ints_packed_table[mask_type];
			
			/*
				pack the word
			*/
			*into = 0;
			size_t mask_type_offset = 28 * mask_type;
			for (size_t offset = 0; offset < num_to_pack; offset++)
				*into |= (source[pos + offset] << simple16_shift_table[mask_type_offset + offset]);
			*into = (*into << 4) | mask_type;
			pos += num_to_pack;
			into++;
			}
		return words_in_compressed_string * sizeof(*into);
		}

	/*
		COMPRESS_INTEGER_SIMPLE_16::DECODE()
		------------------------------------
	*/
	void compress_integer_simple_16::decode(integer *destination, size_t destination_integers, const void *source, size_t source_length)
		{
		const uint32_t *compressed_sequence = reinterpret_cast<const uint32_t *>(source);
		integer *end = destination + destination_integers;

		while (destination < end)
			{
			integer value = *compressed_sequence++;
			integer mask_type = value & 0xF;
			value >>= 4;

			/*
				Unrolled loop to enable pipelining
			*/
			switch (mask_type)
				{
				case 0x0:
					*destination++ = value & 0x1;
					*destination++ = (value >> 0x1) & 0x1;
					*destination++ = (value >> 0x2) & 0x1;
					*destination++ = (value >> 0x3) & 0x1;
					*destination++ = (value >> 0x4) & 0x1;
					*destination++ = (value >> 0x5) & 0x1;
					*destination++ = (value >> 0x6) & 0x1;
					*destination++ = (value >> 0x7) & 0x1;
					*destination++ = (value >> 0x8) & 0x1;
					*destination++ = (value >> 0x9) & 0x1;
					*destination++ = (value >> 0xA) & 0x1;
					*destination++ = (value >> 0xB) & 0x1;
					*destination++ = (value >> 0xC) & 0x1;
					*destination++ = (value >> 0xD) & 0x1;
					*destination++ = (value >> 0xE) & 0x1;
					*destination++ = (value >> 0xF) & 0x1;
					*destination++ = (value >> 0x10) & 0x1;
					*destination++ = (value >> 0x11) & 0x1;
					*destination++ = (value >> 0x12) & 0x1;
					*destination++ = (value >> 0x13) & 0x1;
					*destination++ = (value >> 0x14) & 0x1;
					*destination++ = (value >> 0x15) & 0x1;
					*destination++ = (value >> 0x16) & 0x1;
					*destination++ = (value >> 0x17) & 0x1;
					*destination++ = (value >> 0x18) & 0x1;
					*destination++ = (value >> 0x19) & 0x1;
					*destination++ = (value >> 0x1A) & 0x1;
					*destination++ = (value >> 0x1B) & 0x1;
					break;
				case 0x1:
					*destination++ = value & 0x3;
					*destination++ = (value >> 0x2) & 0x3;
					*destination++ = (value >> 0x4) & 0x3;
					*destination++ = (value >> 0x6) & 0x3;
					*destination++ = (value >> 0x8) & 0x3;
					*destination++ = (value >> 0xA) & 0x3;
					*destination++ = (value >> 0xC) & 0x3;
					*destination++ = (value >> 0xE) & 0x1;
					*destination++ = (value >> 0xF) & 0x1;
					*destination++ = (value >> 0x10) & 0x1;
					*destination++ = (value >> 0x11) & 0x1;
					*destination++ = (value >> 0x12) & 0x1;
					*destination++ = (value >> 0x13) & 0x1;
					*destination++ = (value >> 0x14) & 0x1;
					*destination++ = (value >> 0x15) & 0x1;
					*destination++ = (value >> 0x16) & 0x1;
					*destination++ = (value >> 0x17) & 0x1;
					*destination++ = (value >> 0x18) & 0x1;
					*destination++ = (value >> 0x19) & 0x1;
					*destination++ = (value >> 0x1A) & 0x1;
					*destination++ = (value >> 0x1B) & 0x1;
					break;
				case 0x2:
					*destination++ = value & 0x1;
					*destination++ = (value >> 0x1) & 0x1;
					*destination++ = (value >> 0x2) & 0x1;
					*destination++ = (value >> 0x3) & 0x1;
					*destination++ = (value >> 0x4) & 0x1;
					*destination++ = (value >> 0x5) & 0x1;
					*destination++ = (value >> 0x6) & 0x1;
					*destination++ = (value >> 0x7) & 0x3;
					*destination++ = (value >> 0x9) & 0x3;
					*destination++ = (value >> 0xB) & 0x3;
					*destination++ = (value >> 0xD) & 0x3;
					*destination++ = (value >> 0xF) & 0x3;
					*destination++ = (value >> 0x11) & 0x3;
					*destination++ = (value >> 0x13) & 0x3;
					*destination++ = (value >> 0x15) & 0x1;
					*destination++ = (value >> 0x16) & 0x1;
					*destination++ = (value >> 0x17) & 0x1;
					*destination++ = (value >> 0x18) & 0x1;
					*destination++ = (value >> 0x19) & 0x1;
					*destination++ = (value >> 0x1A) & 0x1;
					*destination++ = (value >> 0x1B) & 0x1;
					break;
				case 0x3:
					*destination++ = value & 0x1;
					*destination++ = (value >> 0x1) & 0x1;
					*destination++ = (value >> 0x2) & 0x1;
					*destination++ = (value >> 0x3) & 0x1;
					*destination++ = (value >> 0x4) & 0x1;
					*destination++ = (value >> 0x5) & 0x1;
					*destination++ = (value >> 0x6) & 0x1;
					*destination++ = (value >> 0x7) & 0x1;
					*destination++ = (value >> 0x8) & 0x1;
					*destination++ = (value >> 0x9) & 0x1;
					*destination++ = (value >> 0xA) & 0x1;
					*destination++ = (value >> 0xB) & 0x1;
					*destination++ = (value >> 0xC) & 0x1;
					*destination++ = (value >> 0xD) & 0x1;
					*destination++ = (value >> 0xE) & 0x3;
					*destination++ = (value >> 0x10) & 0x3;
					*destination++ = (value >> 0x12) & 0x3;
					*destination++ = (value >> 0x14) & 0x3;
					*destination++ = (value >> 0x16) & 0x3;
					*destination++ = (value >> 0x18) & 0x3;
					*destination++ = (value >> 0x1A) & 0x3;
					break;
				case 0x4:
					*destination++ = value & 0x3;
					*destination++ = (value >> 0x2) & 0x3;
					*destination++ = (value >> 0x4) & 0x3;
					*destination++ = (value >> 0x6) & 0x3;
					*destination++ = (value >> 0x8) & 0x3;
					*destination++ = (value >> 0xA) & 0x3;
					*destination++ = (value >> 0xC) & 0x3;
					*destination++ = (value >> 0xE) & 0x3;
					*destination++ = (value >> 0x10) & 0x3;
					*destination++ = (value >> 0x12) & 0x3;
					*destination++ = (value >> 0x14) & 0x3;
					*destination++ = (value >> 0x16) & 0x3;
					*destination++ = (value >> 0x18) & 0x3;
					*destination++ = (value >> 0x1A) & 0x3;
					break;
				case 0x5:
					*destination++ = value & 0xF;
					*destination++ = (value >> 0x4) & 0x7;
					*destination++ = (value >> 0x7) & 0x7;
					*destination++ = (value >> 0xA) & 0x7;
					*destination++ = (value >> 0xD) & 0x7;
					*destination++ = (value >> 0x10) & 0x7;
					*destination++ = (value >> 0x13) & 0x7;
					*destination++ = (value >> 0x16) & 0x7;
					*destination++ = (value >> 0x19) & 0x7;
					break;
				case 0x6:
					*destination++ = value & 0x7;
					*destination++ = (value >> 0x3) & 0xF;
					*destination++ = (value >> 0x7) & 0xF;
					*destination++ = (value >> 0xB) & 0xF;
					*destination++ = (value >> 0xF) & 0xF;
					*destination++ = (value >> 0x13) & 0x7;
					*destination++ = (value >> 0x16) & 0x7;
					*destination++ = (value >> 0x19) & 0x7;
					break;
				case 0x7:
					*destination++ = value & 0xF;
					*destination++ = (value >> 0x4) & 0xF;
					*destination++ = (value >> 0x8) & 0xF;
					*destination++ = (value >> 0xC) & 0xF;
					*destination++ = (value >> 0x10) & 0xF;
					*destination++ = (value >> 0x14) & 0xF;
					*destination++ = (value >> 0x18) & 0xF;
					break;
				case 0x8:
					*destination++ = value & 0x1F;
					*destination++ = (value >> 0x5) & 0x1F;
					*destination++ = (value >> 0xA) & 0x1F;
					*destination++ = (value >> 0xF) & 0x1F;
					*destination++ = (value >> 0x14) & 0xF;
					*destination++ = (value >> 0x18) & 0xF;
					break;
				case 0x9:
					*destination++ = value & 0xF;
					*destination++ = (value >> 0x4) & 0xF;
					*destination++ = (value >> 0x8) & 0x1F;
					*destination++ = (value >> 0xD) & 0x1F;
					*destination++ = (value >> 0x12) & 0x1F;
					*destination++ = (value >> 0x17) & 0x1F;
					break;
				case 0xA:
					*destination++ = value & 0x3F;
					*destination++ = (value >> 0x6) & 0x3F;
					*destination++ = (value >> 0xC) & 0x3F;
					*destination++ = (value >> 0x12) & 0x1F;
					*destination++ = (value >> 0x17) & 0x1F;
					break;
				case 0xB:
					*destination++ = value & 0x1F;
					*destination++ = (value >> 0x5) & 0x1F;
					*destination++ = (value >> 0xA) & 0x3F;
					*destination++ = (value >> 0x10) & 0x3F;
					*destination++ = (value >> 0x16) & 0x3F;
					break;
				case 0xC:
					*destination++ = value & 0x7F;
					*destination++ = (value >> 0x7) & 0x7F;
					*destination++ = (value >> 0xE) & 0x7F;
					*destination++ = (value >> 0x15) & 0x7F;
					break;
				case 0xD:
					*destination++ = value & 0x3FF;
					*destination++ = (value >> 0xA) & 0x1FF;
					*destination++ = (value >> 0x13) & 0x1FF;
					break;
				case 0xE:
					*destination++ = value & 0x3FFF;
					*destination++ = (value >> 0xE) & 0x3FFF;
					break;
				case 0xF:
					*destination++ = value & 0xFFFFFFF;
					break;
				}
			}
		}
		
	/*
		COMPRESS_INTEGER_SIMPLE_16::UNITTEST()
		-------------------------------------
	*/
	void compress_integer_simple_16::unittest(void)
		{
		std::vector<integer> every_case;

		size_t instance;

		/*
			28 * 1-bit
		*/
		for (instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);

		/*
			7 * 2-bits and 14 * 1-bit
		*/
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x03);
		for (instance = 0; instance < 14; instance++)
			every_case.push_back(0x01);

		/*
			7 * 1-bit and 7 * 2-bits and 7 * 1-bit
		*/
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x01);
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x03);
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x01);
			
		/*
			14 * 1-bit and 7 * 2-bits
		*/
		for (instance = 0; instance < 14; instance++)
			every_case.push_back(0x01);
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x03);

		/*
			14 * 2-bits
		*/
		for (instance = 0; instance < 14; instance++)
			every_case.push_back(0x03);

		/*
			1 * 4-bit and 8 * 3-bits
		*/
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 8; instance++)
			every_case.push_back(0x07);

		/*
			1 * 3-bits and 4 * 4-bits and 3 * 3-bits
		*/
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x07);
		for (instance = 0; instance < 4; instance++)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 3; instance++)
			every_case.push_back(0x07);

		/*
			7 * 4-bits
		*/
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x0F);
 
		/*
			4 * 5 bits and 2 * 4 bits
		*/
		for (instance = 0; instance < 4; instance++)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x0F);

		/*
			2 * 4-bits and 4 * 5-bits
		*/
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 4; instance++)
			every_case.push_back(0x1F);

		/*
			3 * 6-bits and 2 * 5-bits
		*/
		for (instance = 0; instance < 3; instance++)
			every_case.push_back(0x3F);
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x1F);

		/*
			2 * 5-bits and 3 * 6 bits
		*/
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 3; instance++)
			every_case.push_back(0x3F);

		/*
			4 * 7-bits
		*/
		for (instance = 0; instance < 4; instance++)
			every_case.push_back(0x7F);

		/*
			1 * 10-bits and 2 * 9 bits
		*/
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x3FF);
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x1FF);

		/*
			2 * 14-bits
		*/
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x3FFF);

		/*
			1 * 28-bits
		*/
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x0FFFFFFF);

		compress_integer_simple_16 *compressor = new compress_integer_simple_16;
		std::vector<uint32_t>compressed(every_case.size() * 2);
		std::vector<uint32_t>decompressed(every_case.size() + 256);

		auto size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor->decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);
		
		/*
			Try the error cases
			(1) no integers
			(2) Integer overflow
			(3) buffer overflow
		*/
		integer one = 1;
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &one, 0);
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		for (instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);
		size_once_compressed = compressor->encode(&compressed[0], 1, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		delete compressor;
		puts("compress_integer_simple_16::PASSED");
		}
	}
