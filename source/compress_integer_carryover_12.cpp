/*
	COMPRESS_INTEGER_CARRYOVER_12.H
	-------------------------------
	Functions for Vo Ngoc Anh and Alistair Moffat's Carryover-12 compression scheme
	This is a port of Anh and Moffat's code to ATIRE then ported to JASS.

	Originally (http://www.cs.mu.oz.au/~alistair/carry/)
		Copyright (C) 2003  Authors: Vo Ngoc Anh & Alistair Moffat

		This program is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful, 
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

	These Changes
		Copyright (C) 2006, 2009 Authors: Andrew Trotman
		22 January 2010, Vo Ngoc Anh and Alistair Moffat gave permission to BSD this code and derivitaves of it:

		> From: "Vo Ngoc ANH" <vo@csse.unimelb.edu.au>
		> To: "Andrew Trotman" <andrew@cs.otago.ac.nz>
		> Cc: "Alistair Moffat" <alistair@csse.unimelb.edu.au>; "andrew Trotman" <andrew.trotman@otago.ac.nz>
		> Sent: Friday, January 22, 2010 1:11 PM
		> Subject: Re: Carryover 12
		>
		>
		>
		> Hi Andrew, 
		> Thank you for your interest. To tell the truth, I don't have enough 
		> knowledge on the license matter. From my side, the answer is Yes. Please 
		> let me know if I need to do anything for that.
		>
		> Cheers, 
		> Anh.
		>
		> On Fri, 22 Jan 2010, Andrew Trotman wrote:
		>
		>> Hi, 
		>>
		>> At ADCS I spoke biefly to Alistair about licenses for your source code to 
		>> the Carryover 12 compression scheme.  I have a copy I downloaded shortly 
		>> after you released it, and I've hacked it quite a bit.  The problem I 
		>> have is that I want to release my hacked version using a BSD license and 
		>> your code is GPL. The two licenses are not compatible.  I want to use the 
		>> BSD license because my code includes other stuff that is already BSD 
		>> (such as a hashing algorithm). As far as I know the only GPL code I'm 
		>> using is yours.
		>>
		>> Could I please have a "special" license to release my derivitave of your 
		>> code (and derivitaves of my derivitaves) under the BSD license.
		>>
		>> Thanks
		>> Andrew.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "asserts.h"
#include "compress_integer_carryover_12.h"

namespace JASS
	{
	#define TRANS_TABLE_STARTER	33

	#define GET_NEW_WORD														\
		__wval= *__wpos++
	  
	#define WORD_DECODE(x, b)													\
		do 																		\
			{																	\
			if (__wremaining < (b))												\
				{  																\
				GET_NEW_WORD;													\
				__wremaining = 32;												\
				}																\
			(x) = (__wval & __mask[b]) + 1;										\
			__wval >>= (b);														\
			__wremaining -= (b);												\
			} 																	\
		while (0)

	#define CARRY_BLOCK_DECODE_START											\
	do  																		\
		{																		\
		int32_t tmp;																\
		WORD_DECODE(tmp, 1);													\
		__pc30 = tmp == 1 ? trans_B1_30_small : trans_B1_30_big;				\
		__pc32 = tmp == 1 ? trans_B1_32_small : trans_B1_32_big;				\
		__pcbase = __pc30;														\
		CARRY_DECODE_GET_SELECTOR												\
		} 																		\
	while (0)

	#define CARRY_DECODE_GET_SELECTOR											\
		if (__wremaining >= 2)													\
			{																	\
			__pcbase = __pc32;													\
			__wbits = __pcbase[(__wbits << 2) + (__wval & 3)];					\
			__wval >>= 2;														\
			__wremaining -= 2;													\
			if (__wremaining < __wbits)											\
				{																\
				GET_NEW_WORD;													\
				__wremaining = 32;												\
				}																\
			}																	\
		else																	\
			{																	\
			__pcbase = __pc30;													\
			GET_NEW_WORD;														\
			__wbits = __pcbase[(__wbits << 2) + (__wval & 3)];					\
			__wval >>= 2;														\
			__wremaining = 30; 													\
			}

	#define CARRY_DECODE(x)													\
		do 																		\
			{																	\
			if (__wremaining < __wbits)											\
				{																\
				CARRY_DECODE_GET_SELECTOR											\
				}																		\
			x = (__wval & __mask[__wbits]);										\
			__wval >>= __wbits;													\
			__wremaining -= __wbits;											\
			}																	\
		while (0)


	#define MAX_ELEM_PER_WORD		64


	/* ========================================================
	 Coding variables:
	   trans_B1_30_big[], trans_B1_32_big are left and right transition
		 tables (see the paper) for the case when the largest elements
		 occupies more than 16 bits.
	   trans_B1_30_small[], trans_B1_32_small are for the otherwise case

	   __pc30, __pc32 is points to the left, right tables currently used
	   __pcbase points to either __pc30 or __pc32 and represents the
		 current transition table used for coding
	   ========================================================
	*/

	const unsigned char *__pc30, *__pc32;	/* point to transition table, 30 and 32 data bits */
	const unsigned char *__pcbase;    /* point to current transition table */
	/*
		big is transition table for the cases when number of bits
		needed to code the maximal value exceeds 16.
		_small are used otherwise.
	*/
	static const unsigned char trans_B1_30_big[]=
		{
		0, 0, 0, 0, 1, 2, 3, 28, 1, 2, 3, 28, 2, 3, 4, 28, 3, 4, 5, 28, 4, 5, 6, 28, 
		5, 6, 7, 28, 6, 7, 8, 28, 6, 7, 10, 28, 8, 10, 15, 28, 9, 10, 14, 28, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 15, 16, 28, 10, 14, 15, 28, 
		7, 10, 15, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		6, 10, 16, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 9, 15, 28
		};

	static const unsigned char trans_B1_32_big[]=
		{
		0, 0, 0, 0, 1, 2, 3, 28, 1, 2, 3, 28, 2, 3, 4, 28, 3, 4, 5, 28, 4, 5, 6, 28, 
		5, 6, 7, 28, 6, 7, 8, 28, 7, 9, 10, 28, 7, 10, 15, 28, 8, 10, 15, 28, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 10, 15, 28, 10, 15, 16, 28, 
		10, 14, 15, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		6, 10, 16, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 10, 16, 28
		};

	static const unsigned char trans_B1_30_small[]=
		{
		0, 0, 0, 0, 1, 2, 3, 16, 1, 2, 3, 16, 2, 3, 4, 16, 3, 4, 5, 16, 4, 5, 6, 16, 
		5, 6, 7, 16, 6, 7, 8, 16, 6, 7, 10, 16, 7, 8, 10, 16, 9, 10, 14, 16, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 8, 10, 15, 16, 10, 14, 15, 16,  7, 10, 15, 16, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 7, 10, 16
		};

	static const unsigned char trans_B1_32_small[] =
		{
		0, 0, 0, 0, 1, 2, 3, 16, 1, 2, 3, 16, 2, 3, 4, 16, 3, 4, 5, 16, 4, 5, 6, 16, 
		5, 6, 7, 16, 6, 7, 8, 16, 7, 9, 10, 16, 7, 10, 15, 16, 8, 10, 15, 16, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 7, 10, 15, 16, 8, 10, 15, 16, 10, 14, 15, 16, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 3, 7, 10, 16
		};
	 
	static const unsigned char CLOG2TAB[] =
		{
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
		};

	#define GET_TRANS_TABLE(avail) avail < 2? (avail = 30, __pc30) : (avail -= 2, __pc32)

	/*
		__MASK
		------
		__mask[i] is 2^i-1
	*/
	static const unsigned __mask[33]=
		{
		0x00U, 0x01U, 0x03U, 0x07U, 0x0FU, 
		0x1FU, 0x3FU, 0x7FU, 0xFFU, 
		0x01FFU, 0x03FFU, 0x07FFU, 0x0FFFU, 
		0x1FFFU, 0x3FFFU, 0x7FFFU, 0xFFFFU, 
		0x01FFFFU, 0x03FFFFU, 0x07FFFFU, 0x0FFFFFU, 
		0x1FFFFFU, 0x3FFFFFU, 0x7FFFFFU, 0xFFFFFFU, 
		0x01FFFFFFU, 0x03FFFFFFU, 0x07FFFFFFU, 0x0FFFFFFFU, 
		0x1FFFFFFFU, 0x3FFFFFFFU, 0x7FFFFFFFU, 0xFFFFFFFFU
		};

	/*
		MACROS FOR WORD ENCODING
		========================
	*/
	#define WORD_ENCODE_WRITE													\
		do																		\
			{																	\
			uint32_t word;														\
			word = __values[--__pvalue];										\
			for (--__pvalue; __pvalue >= 0; __pvalue--)							\
				{																\
				word <<= __bits[__pvalue];										\
				word |= __values[__pvalue];										\
				}																\
			*((uint32_t *)destination) = word;									\
			destination = (uint8_t*)destination + sizeof(word);										\
			if (destination >= destination_end)									\
				return 0;	 /* overflow */										\
			__wremaining = 32;													\
			__pvalue = 0;														\
			}																	\
		while(0)

	#define WORD_ENCODE(x, b)													\
		do 																		\
			{																	\
			if (__wremaining < (b))												\
				WORD_ENCODE_WRITE;												\
			__values[__pvalue] = ((uint32_t)x) - 1;								\
			__bits[__pvalue++] = (b);											\
			__wremaining -= (b);												\
			} 																	\
		while (0)

	#define CARRY_BLOCK_ENCODE_START(n, max_bits)								\
		do 																		\
			{																	\
			__pc30 = max_bits <= 16 ? trans_B1_30_small : trans_B1_30_big;		\
			__pc32 = max_bits <= 16 ? trans_B1_32_small : trans_B1_32_big;		\
			__pcbase = __pc30;													\
			WORD_ENCODE((max_bits <= 16 ? 1 : 2), 1);							\
			} 																	\
		while(0)

	/*
		QCEILLOG_2()
		------------
	*/
	static inline int32_t qceillog_2(compress_integer::integer x)
		{
		compress_integer::integer _B_x  = x - 1;

		return (_B_x >> 16) ?
		((_B_x >> 24) ? 24 + CLOG2TAB[_B_x >> 24] : 16 | CLOG2TAB[_B_x >> 16]) :
		((_B_x >> 8) ? 8 + CLOG2TAB[_B_x >> 8] : CLOG2TAB[_B_x]);
		}

	/*
		CALC_MIN_BITS()
		---------------
		Calculate the number of bits needed to store the largest integer in the list
	*/
	static inline int32_t calc_min_bits(const compress_integer::integer *gaps, size_t n)
		{
		int64_t i;
		int32_t max = 0;

		for (i = 0; i < n; i++)
			{
			int32_t bits;
			if (max < (bits = qceillog_2(gaps[i] + 1)))
				max = bits;
			}

		return max > 28 ? -1 : max;
		}

	/*
		ELEMS_CODED()
		-------------
		given codeleng of "len" bits, and "avail" bits available for coding, 
		Return number_of_elems_coded (possible) if "avail" bits can be used to
		code the number of elems  with the remaining < "len"
		Returns 0 (impossible) otherwise
	*/
	static int64_t elems_coded(int32_t avail, int32_t len, const compress_integer::integer *gaps, int64_t start, size_t end)
		{
		int64_t i;

		if (len)
			{
			int64_t max = avail/len;
			int64_t real_end = start + max - 1 <= end ? start + max: end + 1;
			for (i = start; i < real_end && qceillog_2(gaps[i] + 1) <= len; i++);			// empty loop
			if (i < real_end)
				return 0;
			return real_end - start;
			}
		else
			{
			for (i = start; i < start + MAX_ELEM_PER_WORD && i <= end && qceillog_2(gaps[i] + 1) <= len; i++);			// empty loop

			if (i - start < 2)
				return 0;
			return i - start;
			}
		}

		/*
			COMPRESS_INTEGER_CARRYOVER_12::ENCODE()
			---------------------------------------
			Parameters
			a - (source)
			n - length of a (in integers)
			destination - where the compressed sequence is put
			destination_length - length of destination (in bytes)

			returns length of destination (length in bytes)
		*/
		size_t compress_integer_carryover_12::encode(void *destination, size_t destination_length, const compress_integer::integer *a, size_t n)
			{
			if (n == 0)
				return 0;
				
			int32_t max_bits;
			uint32_t __values[32];			// can't compress integers larger than 2^28 so they will all fit in a uint32_t
			uint32_t __bits[32];
			int64_t i;
			int32_t j, __wremaining = 32, __pvalue = 0, size;
			int64_t elems = 0;
			const unsigned char *table, *base;
			unsigned char *destination_end, *original_destination = static_cast<uint8_t *>(destination);

			destination_end = static_cast<uint8_t *>(destination) + destination_length;

			size = TRANS_TABLE_STARTER;

			if ((max_bits = calc_min_bits(a, n)) < 0)
				return 0;

			CARRY_BLOCK_ENCODE_START(n, max_bits);

			for (i = 0; i < n; )
				{
				int32_t avail = __wremaining;
				table = GET_TRANS_TABLE(avail);
				base = table + (size << 2);       /* row in trans table */

				/* 1. Modeling: Find j= the first-fit column in base */
				for (j = 0; j < 4; j++)
					{
					size = base[j];
					if (size > avail) 		/* must use next word for data  */
						{
						avail = 32;
						j = -1;
						continue;
						}
					if ((elems = elems_coded(avail, size, a, i, n - 1)) != 0)
						break;
					}

				/* 2. Coding: Code elements using row "base" & column "j" */
				WORD_ENCODE(j + 1, 2);             /* encoding column */
				for ( ; elems ; elems--, i++)   /* encoding d-gaps */
					WORD_ENCODE(a[i] + 1, size);
				}

			if (__pvalue)
				WORD_ENCODE_WRITE;

			return static_cast<uint8_t *>(destination) - original_destination;
			}

	/*
		COMPRESS_INTEGER_CARRYOVER_12::DECODE()
		---------------------------------------
		__wpos is the compressed string
		destination is the destination
		n is the number of integers in __wpos
	*/
	void compress_integer_carryover_12::decode(compress_integer::integer *destination, size_t n, const void *compressed, size_t source_length)
		{
		int64_t i;
		int32_t __wbits = TRANS_TABLE_STARTER;
		int32_t __wremaining = -1;
		uint32_t *__wpos = (uint32_t *)compressed, __wval = 0;

		CARRY_BLOCK_DECODE_START;
		for (i = 0; i < n; i++)
			{
			CARRY_DECODE(*destination++);
			}
		}

	/*
		COMPRESS_INTEGER_CARRYOVER_12::UNITTEST()
		-----------------------------------------
	*/
	void compress_integer_carryover_12::unittest(void)
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

		compress_integer_carryover_12 compressor;
		std::vector<uint32_t>compressed(every_case.size() * 2);
		std::vector<uint32_t>decompressed(every_case.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor.decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);
		
		/*
			Try the error cases
			(1) no integers
			(2) Integer overflow
			(3) buffer overflow
		*/
		integer one = 1;
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &one, 0);
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		for (instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);
		size_once_compressed = compressor.encode(&compressed[0], 1, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		puts("compress_integer_carryover_12::PASSED");
		}
	}
