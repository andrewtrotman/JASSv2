/*
	COMPRESS_INTEGER_SIMPLE_9.CPP
	-----------------------------
*/
#include <stdio.h>
#include "compress_integer_simple_9.h"
#include "maths.h"

namespace JASS
	{
	/*
		compress_integer_simple_9::simple9_table[]
		-------------------------------------
		The simple-9 selector table (top 4 bits)
	*/
	compress_integer_simple_9::lookup compress_integer_simple_9::simple9_table[] =
		{
		{1, 28, 0xFFFFFFF},		// integers, bits, bit-mask
		{2, 14, 0x3FFF},
		{3,  9, 0x1FF},
		{4,  7, 0x7F},
		{5,  5, 0x1F},
		{7,  4, 0xF},
		{9,  3, 0x7},
		{14, 2, 0x3},
		{28, 1, 0x1}
		};

	/*
		compress_integer_simple_9::bits_to_use[]
		-----------------------------------
		The number of bits that simple-9 will be used to store an integer of the given the number of bits in length
	*/
	size_t compress_integer_simple_9::bits_to_use[] =
	{
	 1,  1,  2,  3,  4,  5,  7,  7, 
	 9,  9, 14, 14, 14, 14, 14, 28, 
	28, 28, 28, 28, 28, 28, 28, 28, 
	28, 28, 28, 28, 28, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64
	};

	/*
		compress_integer_simple_9::table_row[]
		---------------------------------
		The row of the table to use given the number of integers we can pack into the word
	*/
	size_t compress_integer_simple_9::table_row[] =
	{
	0, 1, 2, 3, 4, 4, 5, 5, 
	6, 6, 6, 6, 6, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 8, 8
	};

	/*
		compress_integer_simple_9::SIMPLE9_SHIFT_TABLE[]
		---------------------------------------------
		Number of bits to shift across when packing -- is sum of prior packed ints
	*/
	size_t compress_integer_simple_9::simple9_shift_table[] =
	{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	0, 4, 8, 12, 16, 20, 24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	0, 5, 10, 15, 20, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	0, 7, 14, 21, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	0, 9, 18, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	0, 14, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	0, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
	};

	/*
		compress_integer_simple_9::INTS_PACKED_TABLE[]
		--------------------------------------------
		Number of integers packed into a 32-bit word, given its mask type
	*/
	size_t compress_integer_simple_9::ints_packed_table[] = {28, 14, 9, 7, 5, 4, 3, 2, 1};

	/*
		compress_integer_simple_9::CAN_PACK_TABLE[]
		---------------------------------------------
		Bitmask map for valid masks at an offset (column) for some num_bits_needed (row).
	*/
	size_t compress_integer_simple_9::can_pack_table[] =
	{
	0x01ff, 0x00ff, 0x007f, 0x003f, 0x001f, 0x000f, 0x000f, 0x0007, 0x0007, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
	0x01fe, 0x00fe, 0x007e, 0x003e, 0x001e, 0x000e, 0x000e, 0x0006, 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x01fc, 0x00fc, 0x007c, 0x003c, 0x001c, 0x000c, 0x000c, 0x0004, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x01f8, 0x00f8, 0x0078, 0x0038, 0x0018, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x01f0, 0x00f0, 0x0070, 0x0030, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x01e0, 0x00e0, 0x0060, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x01c0, 0x00c0, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0180, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};

	/*
		compress_integer_simple_9::INVALID_MASKS_FOR_OFFSET[]
		-----------------------------------
		We AND out masks for offsets where we don't know if we can fully pack for that offset
	*/
	size_t compress_integer_simple_9::invalid_masks_for_offset[] =
	{
	0x0000, 0x0100, 0x0180, 0x01c0, 0x01e0, 0x01f0, 0x01f0, 0x01f8, 0x01f8, 0x01fc, 0x01fc, 0x01fc, 0x01fc, 0x01fc, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01ff
	};

	/*
		compress_integer_simple_9::ROW_FOR_BITS_NEEDED[]
		-----------------------------------
		Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
	*/
	size_t compress_integer_simple_9::row_for_bits_needed[] =
	{
	0, 0, 28, 56, 84, 112, 140, 140, 168, 168, 196, 196, 196, 196, 196, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	/*
		compress_integer_simple_9::COMPRESS()
		--------------------------------
	*/
	size_t compress_integer_simple_9::compress(uint8_t *destination, size_t destination_length, integer *source, size_t source_integers)
	{
	long long words_in_compressed_string, pos;
	uint32_t mask_type, num_to_pack;
	uint32_t *into, *end;
	uint32_t remaining;
	uint32_t offset, mask_type_offset;
	uint16_t last_bitmask, bitmask;
	into = (uint32_t *)destination;
	end = (uint32_t *)(destination + destination_length);
	pos = 0;
	for (words_in_compressed_string = 0; pos < source_integers; words_in_compressed_string++)
		{
		remaining = (pos + 28 < source_integers) ? 28 : source_integers - pos;
		last_bitmask = 0x0000;
		bitmask = 0xFFFF;
		/* constrain last_bitmask to contain only bits for masks we can pack with */
		for (offset = 0; offset < remaining && bitmask; offset++)
			{
			bitmask &= can_pack_table[row_for_bits_needed[maths::ceiling_log2(source[pos + offset])] + offset];
			last_bitmask |= (bitmask & invalid_masks_for_offset[offset + 1]);
			}
		/* ensure valid input */
		if (last_bitmask == 0)
			return 0;
		/* get position of lowest set bit */
		mask_type = maths::find_first_set_bit((uint32_t)last_bitmask);
		num_to_pack = ints_packed_table[mask_type];
		/* pack the word */
		*into = 0;
		mask_type_offset = 28 * mask_type;
		for (offset = 0; offset < num_to_pack; offset++)
			*into |= ((source[pos + offset]) << simple9_shift_table[mask_type_offset + offset]);
		*into = (*into << 4) | mask_type;
		pos += num_to_pack;
		into++;
		if (into > end)
			return 0;
		}
	return words_in_compressed_string * sizeof(*into); //stores the length of n[]
	}

	/*
		compress_integer_simple_9::DECOMPRESS()
		----------------------------------
	*/
	void compress_integer_simple_9::decompress(integer *destination, uint8_t *source, size_t destination_integers)
	{
	uint32_t *compressed_sequence = (uint32_t *)source;
	uint32_t value, mask_type;
	integer *end = destination + destination_integers;

	while (destination < end)
		{
		value = *compressed_sequence++;
		mask_type = value & 0xF;
		value >>= 4;

		// Unrolled loop to enable pipelining
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
				*destination++ = (value >> 0xE) & 0x3;
				*destination++ = (value >> 0x10) & 0x3;
				*destination++ = (value >> 0x12) & 0x3;
				*destination++ = (value >> 0x14) & 0x3;
				*destination++ = (value >> 0x16) & 0x3;
				*destination++ = (value >> 0x18) & 0x3;
				*destination++ = (value >> 0x1A) & 0x3;
				break;
			case 0x2:
				*destination++ = value & 0x7;
				*destination++ = (value >> 0x3) & 0x7;
				*destination++ = (value >> 0x6) & 0x7;
				*destination++ = (value >> 0x9) & 0x7;
				*destination++ = (value >> 0xC) & 0x7;
				*destination++ = (value >> 0xF) & 0x7;
				*destination++ = (value >> 0x12) & 0x7;
				*destination++ = (value >> 0x15) & 0x7;
				*destination++ = (value >> 0x18) & 0x7;
				break;
			case 0x3:
				*destination++ = value & 0xF;
				*destination++ = (value >> 0x4) & 0xF;
				*destination++ = (value >> 0x8) & 0xF;
				*destination++ = (value >> 0xC) & 0xF;
				*destination++ = (value >> 0x10) & 0xF;
				*destination++ = (value >> 0x14) & 0xF;
				*destination++ = (value >> 0x18) & 0xF;
				break;
			case 0x4:
				*destination++ = value & 0x1F;
				*destination++ = (value >> 0x5) & 0x1F;
				*destination++ = (value >> 0xA) & 0x1F;
				*destination++ = (value >> 0xF) & 0x1F;
				*destination++ = (value >> 0x14) & 0x1F;
				break;
			case 0x5:
				*destination++ = value & 0x7F;
				*destination++ = (value >> 0x7) & 0x7F;
				*destination++ = (value >> 0xE) & 0x7F;
				*destination++ = (value >> 0x15) & 0x7F;
				break;
			case 0x6:
				*destination++ = value & 0x1FF;
				*destination++ = (value >> 0x9) & 0x1FF;
				*destination++ = (value >> 0x12) & 0x1FF;
				break;
			case 0x7:
				*destination++ = value & 0x3FFF;
				*destination++ = (value >> 0xE) & 0x3FFF;
				break;
			case 0x8:
				*destination++ = value & 0xFFFFFFF;
				break;
			}
		}
	}
	}
