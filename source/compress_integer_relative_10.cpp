/*
	COMPRESS_INTEGER_RELATIVE_10.CPP
	--------------------------------
	Copyright (c) 2007-2017 Vikram Subramanya, Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Anh and Moffat's Relative-10 Compression scheme from:
	V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166

	This code was originally written by Vikram Subramanya while working on:
	A. Trotman, V. Subramanya (2007), Sigma encoded inverted files, Proceedings of CIKM 2007, pp 983-986

	Substantially re-written and converted for use in ATIRE by Andrew Trotman (2009)
	and re-written with loop unroling for JASS by Andrew Trotman.
	Released undeer BSD license (see compress_sigma.c)
*/


#include <vector>

#include "maths.h"
#include "compress_integer_relative_10.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_RELATIVE10::RELATIVE_10_TABLE
		-----------------------------------------------
		This is the Relative-10 selector table.  The first 3 columns are obvious.
		The last 4 columns are the 2-bit selector - that is, given the previous
		row and a 2 bit selector, it is the new row to use.
		The last 9 columns are points into the last 4 columns used to generate the
		2-bit selectors during compression.
	*/
	const compress_integer_relative_10::relative_10_lookup compress_integer_relative_10::relative10_table[10] =
		{    /* N Bits Mask        Transfer Table                      Decode Table */
		/*0*/	{1, 30, 0x3FFFFFFF, {3, 2, 1, 0, 0, 0, 0, 0, 0, 0},		{3, 2, 1, 0}},
		/*1*/	{2, 15, 0x7FFF,     {3, 2, 1, 0, 0, 0, 0, 0, 0, 0},		{3, 2, 1, 0}},
		/*2*/	{3, 10, 0x3FF,      {3, 2, 1, 0, 0, 0, 0, 0, 0, 0},		{3, 2, 1, 0}},
		/*3*/	{4,  7, 0x7F,       {3, 3, 2, 1, 0, 0, 0, 0, 0, 0},		{4, 3, 2, 0}},
		/*4*/	{5,  6, 0x3F,       {3, 3, 3, 2, 1, 0, 0, 0, 0, 0},		{5, 4, 3, 0}},
		/*5*/	{6,  5, 0x1F,       {3, 3, 3, 3, 2, 1, 0, 0, 0, 0},		{6, 5, 4, 0}},
		/*6*/	{7,  4, 0xF,        {3, 3, 3, 3, 3, 2, 1, 0, 0, 0},		{7, 6, 5, 0}},
		/*7*/	{10, 3, 0x7,        {3, 3, 3, 3, 3, 3, 2, 1, 0, 0},		{8, 7, 6, 0}},
		/*8*/	{15, 2, 0x3,        {3, 3, 3, 3, 3, 3, 3, 2, 1, 0},		{9, 8, 7, 0}},
		/*9*/	{30, 1, 0x1, 		  {3, 3, 3, 3, 3, 3, 3, 2, 1, 0},      {9, 8, 7, 0}}
		};

	/*
		COMPRESS_INTEGER_RELATIVE_10::BITS_TO_USE10
		-------------------------------------------
		This is the number of bits that simple-9 will be used to store an integer of the given the number of bits in length
	*/
	const uint32_t compress_integer_relative_10::bits_to_use10[] =
		{
		 1,  1,  2,  3,  4,  5,  6,  7,
		10, 10, 10, 15, 15, 15, 15, 15,
		30, 30, 30, 30, 30, 30, 30, 30,
		30, 30, 30, 30, 30, 30, 30, 64,
		64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64
		};

	/*
		COMPRESS_INTEGER_RELATIVE_10::TABLE_ROW10
		-----------------------------------------
		This is the row of the table to use given the number of integers we can pack into the word
	*/
	const uint32_t compress_integer_relative_10::table_row10[] =
		{
		0, 1, 2, 3, 4, 5, 6, 6, 6,
		7, 7, 7, 7, 7, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 9, 9, 9
		};

	/*
		COMPRESS_INTEGER_RELATIVE_10::ENCODE()
		--------------------------------------
	*/
	size_t compress_integer_relative_10::encode(void *destination, size_t destination_length, const integer *source, size_t source_integers)
		{
		if (source_integers == 0)
			return 0;
		if (destination_length < 4)
			return 0;
			
		const integer *from = source;
		uint64_t words_in_compressed_string;

		/*
			Init
		*/
		uint32_t *into = reinterpret_cast<uint32_t *>(destination);
		uint32_t *end = reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(destination) + destination_length);

		/*
			Encode the first word using Simple 9
		*/
		uint32_t needed = 0;
		uint64_t pos = 0;
		uint32_t term;
		for (term = 0; term < 28 && pos + term < source_integers; term++)
			{
			uint32_t needed_for_this_integer = bits_to_use[maths::ceiling_log2(source[pos + term])];
			if (needed_for_this_integer > 28 || needed_for_this_integer < 1)
				return 0;					// we fail because there is an integer greater then 2^28 (or 0) and so we cannot pack it
			if (needed_for_this_integer > needed)
				needed = needed_for_this_integer;
			if (needed * (term + 1) > 28)				// then we'll overflow so break out
				break;
			}

		uint32_t row = table_row[term - 1];
		pos = simple9_table[row].numbers;
		uint32_t bits_per_integer = simple9_table[row].bits;

		*into = row << 28;   //puts the row no. to the first 4 bits.
		for (term = 0; from < source + pos; term++)
			*into |= (*from++ << (term * bits_per_integer));  //left shift the bits to the correct position in n[j]
		into++;

		/*
			And the remainder in Relative 10
		*/
		for (words_in_compressed_string = 1; pos < source_integers; words_in_compressed_string++)  //outer loop: loops thru' all the elements in source[]
			{
			if (into >= end)
				return 0;
			needed = 0;
			for (term = 0; term < 30 && pos + term < source_integers; term++)
				{
				uint32_t needed_for_this_integer = bits_to_use10[maths::ceiling_log2(source[pos + term])];
				if (needed_for_this_integer > 30 || needed_for_this_integer < 1)
					return 0;					// we fail because there is an integer greater then 2^30 (or 0) and so we cannot pack it
				if (needed_for_this_integer > needed)
					needed = needed_for_this_integer;
				if (needed * (term + 1) > 30)				// then we'll overflow so break out
					break;
				}

			uint32_t r = relative10_table[row].transfer_array[table_row10[term - 1]];
			row = relative10_table[row].relative_row[r];

			pos += relative10_table[row].numbers;
			bits_per_integer = relative10_table[row].bits;

			*into = r << 30;   //puts the row no. to the first 4 bits.
			for (term = 0; from < source + pos; term++)
				*into |= (*from++ << (term * bits_per_integer));  //left shift the bits to the correct position in n[j]
			into++;
			}
		return words_in_compressed_string * sizeof(*into);
		}

	/*
		COMPRESS_INTEGER_RELATIVE_10::DECODE()
		--------------------------------------
	*/
	void compress_integer_relative_10::decode(integer *destination, size_t destination_integers, const void *source, size_t source_length)
		{
		long long numbers;
		long mask, bits;
		uint32_t *compressed_sequence = (uint32_t *)source;
		uint32_t value, row;
		integer *end = destination + destination_integers;

		/*
			The first word is encoded in Simple-9
		*/
		value = *compressed_sequence++;
		row = value >> 28;
		value &= 0x0fffffff;

		bits = simple9_table[row].bits;
		mask = simple9_table[row].mask;
		numbers = simple9_table[row].numbers;

		while (numbers-- > 0)
			{
			*destination++ = value & mask;
			value >>= bits;
			}

		if (destination >= end)
			return;			// we're done as it all fits in the first word!

		/*
			The remainder is in relative-10
		*/
		while (destination < end)
			{
			value = *compressed_sequence++;
			row = relative10_table[row].relative_row[value >> 30];

			switch (row)
				{
				case 0:
						*destination++ = value & 0X3FFFFFFF;
						break;
				case 1:
						*destination++ = value & 0x7FFF;
						*destination++ = (value >> 15) & 0x7FFF;
						break;
				case 2:
						*destination++ = value & 0x3FF;
						*destination++ = (value >> 10) & 0x3FF;
						*destination++ = (value >> 20) & 0x3FF;
						break;
				case 3:
						*destination++ = value & 0x7F;
						*destination++ = (value >> 7) & 0x7F;
						*destination++ = (value >> 14) & 0x7F;
						*destination++ = (value >> 21) & 0x7F;
						break;
				case 4:
						*destination++ = value & 0x3F;
						*destination++ = (value >> 6) & 0x3F;
						*destination++ = (value >> 12) & 0x3F;
						*destination++ = (value >> 18) & 0x3F;
						*destination++ = (value >> 24) & 0x3F;
						break;
				case 5:
						*destination++ = value & 0x1F;
						*destination++ = (value >> 5) & 0x1F;
						*destination++ = (value >> 10) & 0x1F;
						*destination++ = (value >> 15) & 0x1F;
						*destination++ = (value >> 20) & 0x1F;
						*destination++ = (value >> 25) & 0x1F;
						break;
				case 6:
						*destination++ = value & 0xF;
						*destination++ = (value >> 4) & 0xF;
						*destination++ = (value >> 8) & 0xF;
						*destination++ = (value >> 12) & 0xF;
						*destination++ = (value >> 16) & 0xF;
						*destination++ = (value >> 20) & 0xF;
						*destination++ = (value >> 24) & 0xF;
						break;
				case 7:
						*destination++ = value & 0x7;
						*destination++ = (value >> 3) & 0x7;
						*destination++ = (value >> 6) & 0x7;
						*destination++ = (value >> 8) & 0x7;
						*destination++ = (value >> 12) & 0x7;
						*destination++ = (value >> 15) & 0x7;
						*destination++ = (value >> 18) & 0x7;
						*destination++ = (value >> 21) & 0x7;
						*destination++ = (value >> 24) & 0x7;
						*destination++ = (value >> 27) & 0x7;
						break;
				case 8:
						*destination++ = value & 0x3;
						*destination++ = (value >> 2) & 0x3;
						*destination++ = (value >> 4) & 0x3;
						*destination++ = (value >> 6) & 0x3;
						*destination++ = (value >> 8) & 0x3;
						*destination++ = (value >> 10) & 0x3;
						*destination++ = (value >> 12) & 0x3;
						*destination++ = (value >> 14) & 0x3;
						*destination++ = (value >> 16) & 0x3;
						*destination++ = (value >> 18) & 0x3;
						*destination++ = (value >> 20) & 0x3;
						*destination++ = (value >> 22) & 0x3;
						*destination++ = (value >> 24) & 0x3;
						*destination++ = (value >> 26) & 0x3;
						*destination++ = (value >> 28) & 0x3;
						break;
				case 9:
						*destination++ = value & 0x1;
						*destination++ = (value >> 1) & 0x1;
						*destination++ = (value >> 2) & 0x1;
						*destination++ = (value >> 3) & 0x1;
						*destination++ = (value >> 4) & 0x1;
						*destination++ = (value >> 5) & 0x1;
						*destination++ = (value >> 6) & 0x1;
						*destination++ = (value >> 7) & 0x1;
						*destination++ = (value >> 8) & 0x1;
						*destination++ = (value >> 9) & 0x1;
						*destination++ = (value >> 10) & 0x1;
						*destination++ = (value >> 11) & 0x1;
						*destination++ = (value >> 12) & 0x1;
						*destination++ = (value >> 13) & 0x1;
						*destination++ = (value >> 14) & 0x1;
						*destination++ = (value >> 15) & 0x1;
						*destination++ = (value >> 16) & 0x1;
						*destination++ = (value >> 17) & 0x1;
						*destination++ = (value >> 18) & 0x1;
						*destination++ = (value >> 19) & 0x1;
						*destination++ = (value >> 20) & 0x1;
						*destination++ = (value >> 21) & 0x1;
						*destination++ = (value >> 22) & 0x1;
						*destination++ = (value >> 23) & 0x1;
						*destination++ = (value >> 24) & 0x1;
						*destination++ = (value >> 25) & 0x1;
						*destination++ = (value >> 26) & 0x1;
						*destination++ = (value >> 27) & 0x1;
						*destination++ = (value >> 28) & 0x1;
						*destination++ = (value >> 29) & 0x1;
						break;
				}
			}
		}

	/*
		COMPRESS_INTEGER_RELATIVE_10::UNITTEST()
		----------------------------------------
	*/
	void compress_integer_relative_10::unittest(void)
		{
		std::vector<integer> every_case;

		/*
			start with the largest possible Simple-9 value
		*/
		every_case.push_back(0x0FFFFFFF);

		/*
			now move on to Relative-10 values.  Starting at selector 0, slowly move down each selector one at a time.
		*/
		every_case.push_back(0x0FFFFFFF);
		for (size_t instance = 0; instance < 2; instance++)
			every_case.push_back(0x7FFF);
		for (size_t instance = 0; instance < 3; instance++)
			every_case.push_back(0x3FF);
		for (size_t instance = 0; instance < 4; instance++)
			every_case.push_back(0x7F);
		for (size_t instance = 0; instance < 5; instance++)
			every_case.push_back(0x3F);
		for (size_t instance = 0; instance < 6; instance++)
			every_case.push_back(0x1F);
		for (size_t instance = 0; instance < 7; instance++)
			every_case.push_back(0xF);
		for (size_t instance = 0; instance < 10; instance++)
			every_case.push_back(0x7);
		for (size_t instance = 0; instance < 15; instance++)
			every_case.push_back(0x3);
		for (size_t instance = 0; instance < 30; instance++)
			every_case.push_back(0x1);

		/*
			test that it worked
		*/
		compress_integer_relative_10 *compressor = new compress_integer_relative_10;
		std::vector<uint32_t>compressed(every_case.size() * 2);
		std::vector<uint32_t>decompressed(every_case.size() + 256);

		auto size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor->decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);

		compressor->decode(&decompressed[0], 1, &compressed[0], size_once_compressed);
		JASS_assert(decompressed[0] == every_case[0]);

		/*
			Try the error cases
			(1) 1 integer (encoded with Simple-9)
			(2) no integers
			(3) Integer overflow
			(4) Integer overflow in the Relative-10 encoder
			(5) buffer overflow on simple-9 encoder
			(6) buffer overflow on Relatice-10 encoder
		*/
		compressor->decode(&decompressed[0], 1, &compressed[0], size_once_compressed);
		JASS_assert(decompressed[0] == every_case[0]);

		integer one = 1;
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &one, 0);
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0x0FFFFFFF);
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		for (size_t instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);
		size_once_compressed = compressor->encode(&compressed[0], 1, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		for (size_t instance = 0; instance < 28; instance++)
			every_case.push_back(0xFF);
		size_once_compressed = compressor->encode(&compressed[0], 5, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		delete compressor;
		puts("compress_integer_relative_10::PASSED");
		}
	}
