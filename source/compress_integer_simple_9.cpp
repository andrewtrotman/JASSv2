/*
	COMPRESS_INTEGER_SIMPLE_9.CPP
	-----------------------------
	Copyright (c) 2007-2017 Vikram Subramanya, Andrew Trotman, Blake Burgess
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Anh and Moffat's Simple-9 Compression scheme from:
	V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166

	This code was originally written by Vikram Subramanya while working on:
	A. Trotman, V. Subramanya (2007), Sigma encoded inverted files, Proceedings of CIKM 2007, pp 983-986

	Substantially re-written and converted for use in ATIRE by Andrew Trotman (2009)
	but then adapted for Simple-8b and back ported by Blake Burgess.  This version has been
	refactored for JASS by Andrew Trotman.
*/
#include <stdio.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_simple_9.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_SIMPLE_9::SIMPLE9_TABLE
		----------------------------------------
		The simple-9 selector table (top 4 bits)
	*/
	const compress_integer_simple_9::lookup compress_integer_simple_9::simple9_table[] =
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
		COMPRESS_INTEGER_SIMPLE_9::BITS_TO_USE
		--------------------------------------
		The number of bits that simple-9 will be used to store an integer of the given the number of bits in length
	*/
	const uint32_t compress_integer_simple_9::bits_to_use[] =
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
		COMPRESS_INTEGER_SIMPLE_9::TABLE_ROW
		------------------------------------
		The row of the table to use given the number of integers we can pack into the word
	*/
	const uint32_t compress_integer_simple_9::table_row[] =
		{
		0, 1, 2, 3, 4, 4, 5, 5, 
		6, 6, 6, 6, 6, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 8, 8
		};

	/*
		COMPRESS_INTEGER_SIMPLE_9::SIMPLE9_SHIFT_TABLE
		----------------------------------------------
		Number of bits to shift across when packing - is sum of prior packed ints
	*/
	const uint32_t compress_integer_simple_9::simple9_shift_table[] =
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
		COMPRESS_INTEGER_SIMPLE_9::INTS_PACKED_TABLE
		--------------------------------------------
		Number of integers packed into a 32-bit word, given its mask type
	*/
	const uint32_t compress_integer_simple_9::ints_packed_table[] =
		{
		28, 14, 9, 7, 5, 4, 3, 2, 1
		};

	/*
		COMPRESS_INTEGER_SIMPLE_9::CAN_PACK_TABLE
		-----------------------------------------
		Bitmask map for valid masks at an offset (column) for some num_bits_needed (row).
	*/
	const uint32_t compress_integer_simple_9::can_pack_table[] =
		{
		0x01ff, 0x00ff, 0x007f, 0x003f, 0x001f, 0x000f, 0x000f, 0x0007, 0x0007, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
		0x01fe, 0x00fe, 0x007e, 0x003e, 0x001e, 0x000e, 0x000e, 0x0006, 0x0006, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x01fc, 0x00fc, 0x007c, 0x003c, 0x001c, 0x000c, 0x000c, 0x0004, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x01f8, 0x00f8, 0x0078, 0x0038, 0x0018, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x01f0, 0x00f0, 0x0070, 0x0030, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x01e0, 0x00e0, 0x0060, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x01c0, 0x00c0, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0180, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		};

	/*
		COMPRESS_INTEGER_SIMPLE_9::INVALID_MASKS_FOR_OFFSET
		---------------------------------------------------
		We AND out masks for offsets where we don't know if we can fully pack for that offset
	*/
	const uint32_t compress_integer_simple_9::invalid_masks_for_offset[] =
		{
		0x0000, 0x0100, 0x0180, 0x01c0, 0x01e0, 0x01f0, 0x01f0, 0x01f8, 0x01f8, 0x01fc, 0x01fc, 0x01fc, 0x01fc, 0x01fc, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01fe, 0x01ff
		};

	/*
		COMPRESS_INTEGER_SIMPLE_9::ROW_FOR_BITS_NEEDED
		----------------------------------------------
		Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
	*/
	const uint32_t compress_integer_simple_9::row_for_bits_needed[] =
		{
		0, 0, 28, 56, 84, 112, 140, 140, 168, 168, 196, 196, 196, 196, 196, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,			// Valid
		252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252		// Overflow
		};

	/*
		COMPRESS_INTEGER_SIMPLE_9::ENCODE()
		-----------------------------------
	*/
	size_t compress_integer_simple_9::encode(void *destination, size_t destination_length, const integer *source, size_t source_integers)
		{
		size_t words_in_compressed_string;
		uint32_t *into = reinterpret_cast<uint32_t *>(destination);
		uint32_t *end = reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(destination) + destination_length);
		size_t pos = 0;
		for (words_in_compressed_string = 0; pos < source_integers; words_in_compressed_string++)
			{
			/*
				Check for overflow (before we overflow)
			*/
			if (into + 1 > end)
				return 0;

			size_t remaining = (pos + 28 < source_integers) ? 28 : source_integers - pos;
			size_t last_bitmask = 0x0000;
			size_t bitmask = 0xFFFF;

			/*
				Constrain last_bitmask to contain only bits for masks we can pack with
			*/
			for (size_t offset = 0; offset < remaining && bitmask; offset++)
				{
				bitmask &= can_pack_table[row_for_bits_needed[maths::ceiling_log2(source[pos + offset])] + offset];
				last_bitmask |= (bitmask & invalid_masks_for_offset[offset + 1]);
				}

			/*
				Ensure valid input (this is triggered when and integer greater than 2^28 is in the unput stream
			*/
			if (last_bitmask == 0)
				return 0;

			/*
				Get position of lowest set bit
			*/
			uint32_t mask_type = maths::find_first_set_bit((uint32_t)last_bitmask);
			size_t num_to_pack = ints_packed_table[mask_type];

			/*
				Pack the word
			*/
			*into = 0;
			size_t mask_type_offset = 28 * mask_type;
			for (size_t offset = 0; offset < num_to_pack; offset++)
				*into |= ((source[pos + offset]) << simple9_shift_table[mask_type_offset + offset]);
			*into = (*into << 4) | mask_type;
			pos += num_to_pack;
			into++;
			}

		return words_in_compressed_string * sizeof(*into);
		}

	/*
		COMPRESS_INTEGER_SIMPLE_9::DECODE()
		-----------------------------------
	*/
	void compress_integer_simple_9::decode(integer *destination, size_t destination_integers, const void *source, size_t source_length)
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

	/*
		COMPRESS_INTEGER_SIMPLE_9::UNITTEST()
		-------------------------------------
	*/
	void compress_integer_simple_9::unittest(void)
		{
		std::vector<integer> every_case;

		size_t instance;

		for (instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);
		for (instance = 0; instance < 14; instance++)
			every_case.push_back(0x03);
		for (instance = 0; instance < 9; instance++)
			every_case.push_back(0x07);
		for (instance = 0; instance < 7; instance++)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 5; instance++)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 4; instance++)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 3; instance++)
			every_case.push_back(0x1FF);
		for (instance = 0; instance < 2; instance++)
			every_case.push_back(0x3FFF);
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x0FFFFFFF);

		compress_integer_simple_9 *compressor = new compress_integer_simple_9;
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
		puts("compress_integer_simple_9::PASSED");
		}
	}
