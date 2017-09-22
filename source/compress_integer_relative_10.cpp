/*
	COMPRESS_RELATIVE10.C
	---------------------
	Anh and Moffat's Relative-9 Compression scheme from:
	V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Alligned Binary Codes, Information Retrieval, 8(1):151-166

	This code was originally written by Vikram Subramanya while working on:
	A. Trotman, V. Subramanya (2007), Sigma encoded inverted files, Proceedings of CIKM 2007, pp 983-986

	Substantially re-written and converted for use in ANT by Andrew Trotman (2009)
	Released undeer BSD license (see compress_sigma.c)
*/

#include "compress_relative10.h"
#include "maths.h"

/*
	ANT_compress_relative10::relative10_table[]
	-------------------------------------------
	This is the Relative-10 selector table.  The first 3 columns are obvious.
	The last 4 columns are the 2-bit selector - that is, given the previous
	row and a 2 bit selector, it is the new row to use.
	The last 9 columns are points into the last 4 columns used to generate the
	2-bit selectors during compression.
*/
ANT_compress_relative10::ANT_compress_relative10_lookup ANT_compress_relative10::relative10_table[10] = 
{    /* N Bits Mask        Transfer Table                      Decode Table */
/*0*/	{1, 30, 0x3FFFFFFF, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,		3, 2, 1, 0},
/*1*/	{2, 15, 0x7FFF,     3, 2, 1, 0, 0, 0, 0, 0, 0, 0,		3, 2, 1, 0},
/*2*/	{3, 10, 0x3FF,      3, 2, 1, 0, 0, 0, 0, 0, 0, 0,		3, 2, 1, 0},
/*3*/	{4,  7, 0x7F,       3, 3, 2, 1, 0, 0, 0, 0, 0, 0,		4, 3, 2, 0},
/*4*/	{5,  6, 0x3F,       3, 3, 3, 2, 1, 0, 0, 0, 0, 0,		5, 4, 3, 0},
/*5*/	{6,  5, 0x1F,       3, 3, 3, 3, 2, 1, 0, 0, 0, 0,		6, 5, 4, 0},
/*6*/	{7,  4, 0xF,        3, 3, 3, 3, 3, 2, 1, 0, 0, 0,		7, 6, 5, 0},
/*7*/	{10, 3, 0x7,        3, 3, 3, 3, 3, 3, 2, 1, 0, 0,		8, 7, 6, 0},
/*8*/	{15, 2, 0x3,        3, 3, 3, 3, 3, 3, 3, 2, 1, 0,		9, 8, 7, 0},
/*9*/	{30, 1, 0x1, 		3, 3, 3, 3, 3, 3, 3, 2, 1, 0,       9, 8, 7, 0}
};

/*
	ANT_compress_relative10::bits_to_use10[]
	----------------------------------------
	This is the number of bits that simple-9 will be used to store an integer of the given the number of bits in length
*/
long ANT_compress_relative10::bits_to_use10[] = 
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
	ANT_compress_relative10::table_row10[]
	--------------------------------------
	This is the row of the table to use given the number of integers we can pack into the word
*/
long ANT_compress_relative10::table_row10[] = 
{
0, 1, 2, 3, 4, 5, 6, 6, 6, 
7, 7, 7, 7, 7, 8, 8, 8, 8, 
8, 8, 8, 8, 8, 8, 8, 8, 8, 
8, 8, 9, 9, 9
};

/*
	ANT_COMPRESS_RELATIVE10::COMPRESS()
	-----------------------------------
*/
long long ANT_compress_relative10::compress(unsigned char *destination, long long destination_length, ANT_compressable_integer *source, long long source_integers)
{
ANT_compressable_integer *from = source;
long long words_in_compressed_string, pos;
long row, bits_per_integer, needed_for_this_integer, needed, term, r;
uint32_t *into, *end;

/*
	Init
*/
into = (uint32_t *)destination;
end = (uint32_t *)(destination + destination_length);
from = source;
pos = 0;

/*
	Encode the first word using Simple 9
*/
needed = 0;
for (term = 0; term < 28 && pos + term < source_integers; term++)
	{
	needed_for_this_integer = bits_to_use[ANT_ceiling_log2(source[pos + term])];
	if (needed_for_this_integer > 28 || needed_for_this_integer < 1)
		return 0;					// we fail because there is an integer greater then 2^28 (or 0) and so we cannot pack it
	if (needed_for_this_integer > needed)
		needed = needed_for_this_integer;
	if (needed * (term + 1) > 28)				// then we'll overflow so break out
		break;
	}

row = table_row[term - 1];
pos = simple9_table[row].numbers;
bits_per_integer = simple9_table[row].bits;

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
		needed_for_this_integer = bits_to_use10[ANT_ceiling_log2(source[pos + term])];
		if (needed_for_this_integer > 30 || needed_for_this_integer < 1)
			return 0;					// we fail because there is an integer greater then 2^30 (or 0) and so we cannot pack it
		if (needed_for_this_integer > needed)
			needed = needed_for_this_integer;
		if (needed * (term + 1) > 30)				// then we'll overflow so break out
			break;
		}

	r = relative10_table[row].transfer_array[table_row10[term - 1]];
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
	ANT_COMPRESS_RELATIVE10::DECOMPRESS()
	-------------------------------------
*/
void ANT_compress_relative10::decompress(ANT_compressable_integer *destination, unsigned char *source, long long destination_integers)
{
long long numbers;
long mask, bits;
uint32_t *compressed_sequence = (uint32_t *)source;
uint32_t value, row;
ANT_compressable_integer *end = destination + destination_integers;

/*
	The first word is encoded in Simple-9
*/
value = *compressed_sequence++;
row = value >> 28;
value &= 0x0fffffff;

bits = simple9_table[row].bits;
mask = simple9_table[row].mask;
numbers = simple9_table[row].numbers;

if (numbers > destination_integers)
	numbers = destination_integers;

while (numbers-- > 0)
	{
	*destination++ = value & mask;
	value >>= bits;
	}

if (numbers == destination_integers)
	return;			// we're done as it all fits in the first word!

/*
	The remainder is in relative-10
*/
for (;;)		/* empty loop */
	{
	value = *compressed_sequence++;
	row = relative10_table[row].relative_row[value >> 30];

	value &= 0x3fffffff;		// top 2 bits are the relative selector, botton 30 are the encoded integer

	bits = relative10_table[row].bits;
	mask = relative10_table[row].mask;
	numbers = relative10_table[row].numbers;

	if (destination + numbers < end)
		while (numbers-- > 0)
			{
			*destination++ = value & mask;
			value >>= bits;
			}
	else
		{
		numbers = end - destination;
		while (numbers-- > 0)
			{
			*destination++ = value & mask;
			value >>= bits;
			}
		break;
		}
	}
}
