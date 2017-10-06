/*
	COMPRESS_INTEGER_CARRY_8B.CPP
	-----------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_carry_8b.h"

/*
	By defining CARRY_DEBUG this code will dump out encoding and decoding details.
*/
#define CARRY_DEBUG

namespace JASS
	{
	const compress_integer_carry_8b::selector compress_integer_carry_8b::selector_table[] =
		{
			/* Selector in the 64-bit integer (60-bit payload) */
		/*0*/   {"a60", 1, 255, true},
		/*1*/  {"b60", 1, 128, true},
		/*2*/   {"c60", 1, 60, false},
		/*3*/   {"d60", 2, 30, false},
		/*4*/   {"e60", 3, 20, false},
		/*5*/   {"f60", 4, 15, false},
		/*6*/   {"g60", 5, 12, false},
		/*7*/   {"h60", 6, 10, false},
		/*8*/   {"i60", 7,  8, true},
		/*9*/   {"j60", 8,  7, true},
		/*10*/  {"k60", 9, 6, true},
		/*11*/  {"l60", 10, 6, false},
		/*12*/  {"m60", 11, 5, true},
		/*13*/  {"n60", 12, 5, false},
		/*14*/  {"o60", 14, 4, true},
		/*15*/  {"p60", 15, 4, false},
		/*16*/  {"q60", 18, 3, true},
		/*17*/  {"r60", 20, 3, false},
		/*18*/  {"s60", 28, 2, true},
		/*19*/  {"t60", 30, 2, false},
		/*20*/  {"u60", 56, 1, false},				// >2^32
		/*21*/  {"v60", 60, 1, false},				// >2^32
			/* Selector in the previous 64-bit integer (64-bit payload) */
		/*22*/   {"a64", 1, 255, true},
		/*23*/   {"b64", 1, 180, true},
		/*24*/   {"c64", 1, 120, true},
		/*25*/   {"d64", 1, 64, false},
		/*26*/   {"e64", 2, 32, false},
		/*27*/   {"f64", 3, 21, false},
		/*28*/   {"g64", 4, 16, false},
		/*29*/   {"h64", 5, 12, true},
		/*30*/   {"i64", 6, 10, true},
		/*31*/   {"j64", 7, 9, false},
		/*32*/   {"k64", 8, 8, false},
		/*33*/   {"l64", 9, 7, false},
		/*34*/   {"m64", 10, 6, true},
		/*35*/   {"n64", 12, 5, true},
		/*36*/   {"o64", 15, 4, true},
		/*37*/   {"p64", 16, 4, false},
		/*38*/   {"q64", 20, 3, true},
		/*39*/   {"r64", 21, 3, false},
		/*40*/   {"s64", 30, 2, true},
		/*41*/   {"t64", 32, 2, false},
		/*42*/   {"u64", 60, 1, true},				// >2^32
		/*43*/   {"v64", 64, 1, false},				// >2^32
			/* First integer has 3-bit base then 4-bit selector then 57 bit payload */
		/*44*/   {"a57", 1, 57, false},
		/*45*/   {"b57", 2, 28, false},
		/*46*/   {"c57", 3, 19, false},
		/*47*/   {"d57", 4, 14, false},
		/*48*/   {"e57", 5, 11, false},
		/*49*/   {"f57", 6, 9, false},
		/*50*/   {"g57", 7, 8, false},
		/*51*/   {"h57", 8, 7, false},
		/*52*/   {"i57", 9, 6, false},
		/*53*/   {"j57", 10, 5, true},
		/*54*/   {"k57", 11, 5, false},
		/*55*/   {"l57", 12, 4, true},
		/*56*/   {"m57", 14, 4, false},
		/*57*/   {"n57", 19, 3, false},
		/*58*/   {"o57", 28, 2, false},
		/*59*/   {"p57", 57, 1, false},				// >2^32
		};

	static const size_t table_fifty_seven_start = 44;			///< the start of the table for 57-bit payloads
	static const size_t table_sixty_start = 0;					///< the start of the table for 60-bit payloads
	static const size_t table_sixty_four_start = 22;			///< the start of the table for 64-bit payloads


	/*
		given the number of bits in the largest integer, what should the starting point in selector_table be?
		Note, this must be the smaller of the 60-bit and 64-bit sub-tables from above.
	*/
	const size_t compress_integer_carry_8b::base_table[] =
		{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 2, 2, 3, 3, 3,
		3, 3, 3, 3, 3, 4, 4, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 6, 6, 6, 6, 7, 7, 7,
	 	7
		};

/*
	COMPRESS_INTEGER_CARRY_8B::PACK_ONE_WORD()
	------------------------------------------
*/
size_t compress_integer_carry_8b::pack_one_word(size_t base, size_t highest, uint64_t *destination, const integer *source, size_t source_integers, bool &next_selector_in_previous_word)
	{
	uint64_t selector = 0;

	/*
		Iterate through the list of selectors looking for the best one to use
	*/
	size_t terms = 0;
	do
		{
		while (terms < selector_table[base + selector].integers && terms < source_integers)
			{
			if (maths::ceiling_log2(source[terms]) > selector_table[base + selector].bits)
				{
				selector++;
				break;
				}
			terms++;
			}
		}
	while (terms < selector_table[base + selector].integers && terms < source_integers && selector < highest);

	/*
		We have an integer that is too large to pack into a single codeword.
	*/
	if (selector >= highest)
		return 0;

	/*
		Pack integers into codewords - note that in the case of 255 * 1 (value, not bit), this will wrap, but that's OK because the payload is ignored
	*/
	uint64_t integers_to_encode = selector_table[base + selector].integers <= source_integers ? selector_table[base + selector].integers : source_integers;
	uint64_t bits_per_integer = selector_table[base + selector].bits;

#ifdef CARRY_DEBUG
printf("Selector:%d (%d x %d-bits)\n", (int)(base + selector), (int)integers_to_encode, (int) bits_per_integer);
#endif

	uint64_t word = 0;
	for (int term = integers_to_encode - 1; term >= 0; term--)
		{
		size_t value = term >= source_integers ? 0 : source[term];		// Make sure we don't overflow the input buffer
		word = word << bits_per_integer | value;
		}

	/*
		Pack the selector into the codeword.  If we're bungingin into the previous selector then we first turn off the top 4 bits as they might be crud from low selectors that over-pack value 1's into a word.
	*/
	if (next_selector_in_previous_word)
		*(destination - 1) = (*(destination - 1) & (uint64_t)0x0FFFFFFFFFFFFFFF) | (selector << ((uint64_t)60));
	else
		word = word << 4 | selector;

	*destination = word;

	/*
		What to do with the next selector
	*/
	next_selector_in_previous_word = selector_table[base + selector].next_selector;

	/*
		return the number of selectors we used.
	*/
	return integers_to_encode;
	}

	/*
		COMPRESS_INTEGER_CARRY_8B::ENCODE()
		-----------------------------------
	*/
	size_t compress_integer_carry_8b::encode(void *encoded, size_t destination_length, const integer *source, size_t source_integers)
		{
		const integer *from = source;
		uint64_t *destination = static_cast<uint64_t *>(encoded);
		uint64_t *end = destination + (destination_length >> 3);
		size_t took;
		size_t used = 0;

		bool next_selector_in_previous_word = false;
		/*
			Check for 0 input or 0 sized output buffer
		*/
		if (source_integers == 0)
			return 0;

		if (destination_length < 8)
			return 0;

		/*
			We need to work out which parts of the Carryover table to use - which we do by finding the largest integers in the sequence and
			seeing which part of the table is that bit-ness or smaller.
		*/
		integer largest = 0;
		for (const integer *current = source; current < source + source_integers; current++)
			largest = maths::maximum(largest, *current);
		uint64_t base = base_table[maths::ceiling_log2(largest)];

		/*
			The paper states: "Note that for the 60 data bits case, there are more selector options than presented in Table I for Simple-8b,
			arising from the desire to have space for the next selector whenever it is possible.  For example, a selector for data width of
			9 is now plausible, as it allows the same number of data elements as in case of 10-bit width, and also provides space for the
			next selector. It turns out that the number of selector options, even with the supposition that the largest data size is 28,
			is slightly larger than 16. We bypass this problem by employing exactly 16 consecutive options from the all-possible list, in
			such a way that the largest data symbol in the input stream, and nothing larger, is covered."

			What Anh's code actually does is to work out the largest integer in the sequence and to find that bit-ness in the table and then
			compute an offset for the start of the table.  They call this base.  Its a 3-bit integer stored at the start of the first codeword
			before the first 4-bit selector leaving 57-bits for the first payload.  This is a bit horrid - but if you don't do it then there are
			few chances to use the Carryover technique.  I'll do the same here.  Anh also works out how to adjust the table to pack into the 57
			bits.  I don't do that here, I just compute the 57-bit table and use that.
		*/
		took = pack_one_word(table_fifty_seven_start, 16, destination, from, source_integers, next_selector_in_previous_word);
		if (took == 0)
			return 0;
		used += took;

		/*
			shove the "base" into the high bits of the first codeword
		*/
		*destination = (*destination & (uint64_t)0x1FFFFFFFFFFFFFFF) | ((base & 0x07) << ((uint64_t)61));
		destination++;

		/*
			Now pack according to the selector table
		*/
		do
			{
			/*
				Make sure we fit in the output buffer
			*/
			if (destination >= end)
				return 0;

			/*
				work out which table to use
			*/
			if (next_selector_in_previous_word)
				took = pack_one_word(table_sixty_four_start + base, 16, destination, from + used, source_integers - used, next_selector_in_previous_word);
			else
				took = pack_one_word(table_sixty_start + base, 16, destination, from + used, source_integers - used, next_selector_in_previous_word);

			/*
				failed (integer too large)
			*/
			if (took == 0)
				return 0;
			used += took;
			destination++;
			}
		while (used < source_integers);

		return reinterpret_cast<uint8_t *>(destination) - reinterpret_cast<uint8_t *>(encoded);
		}

	/*
		COMPRESS_INTEGER_CARRY_8B::DECODE()
		-----------------------------------
	*/
	void compress_integer_carry_8b::decode(integer *destination, size_t integers_to_decode, const void *compressed, size_t compressed_size_in_bytes)
		{
		const integer *end = destination + integers_to_decode;
		const uint64_t *source = static_cast<const uint64_t *>(compressed);

		/*
			Get the first selector and payload
		*/
		size_t base = table_fifty_seven_start;
		size_t selector = *source & 0x0F;
		uint64_t payload = (*source & 0x1FFFFFFFFFFFFFF0) >> 4;

		/*
			Where in the table do we start from?
		*/
		size_t offset = *source >> 61;

#ifdef CARRY_DEBUG
printf("Table offset:%d\n", (int)offset);
#endif

		/*
			where does each table start?
		*/
		size_t sixty_start = table_sixty_start + offset;
		size_t sixty_four_start = table_sixty_four_start + offset;


#ifdef CARRY_DEBUG
integer *destination_at_start = destination;
#endif
		while (destination < end)
			{
#ifdef CARRY_DEBUG
printf("[%d] Decode:%d\n", (int)(destination - destination_at_start), (int)(selector + base));
#endif
			switch (selector + base)
				{
				/*
					60-bit selector
				*/
				case 0:			// {"a60", 1, 255, true}
					std::fill(destination, (destination + selector_table[0].integers), 1);
					destination += selector_table[0].integers;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 1:			// {"b60", 1, 128, true}
					std::fill(destination, (destination + selector_table[1].integers), 1);
					destination += selector_table[1].integers;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 2:			// {"c60", 1, 60, false}
					/*
						Since we're not permitted to encode a '0', a 1-bit integer must be a 1
					*/
					std::fill(destination, (destination + selector_table[2].integers), 1);
					destination += selector_table[2].integers;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 3:		// 	{"d60", 2, 30, false}
					*(destination + 0) = payload >> 0 & 0x03;
					*(destination + 1) = payload >> 2 & 0x03;
					*(destination + 2) = payload >> 4 & 0x03;
					*(destination + 3) = payload >> 6 & 0x03;
					*(destination + 4) = payload >> 8 & 0x03;
					*(destination + 5) = payload >> 10 & 0x03;
					*(destination + 6) = payload >> 12 & 0x03;
					*(destination + 7) = payload >> 14 & 0x03;
					*(destination + 8) = payload >> 16 & 0x03;
					*(destination + 9) = payload >> 18 & 0x03;
					*(destination + 10) = payload >> 20 & 0x03;
					*(destination + 11) = payload >> 22 & 0x03;
					*(destination + 12) = payload >> 24 & 0x03;
					*(destination + 13) = payload >> 26 & 0x03;
					*(destination + 14) = payload >> 28 & 0x03;
					*(destination + 15) = payload >> 30 & 0x03;
					*(destination + 16) = payload >> 32 & 0x03;
					*(destination + 17) = payload >> 34 & 0x03;
					*(destination + 18) = payload >> 36 & 0x03;
					*(destination + 19) = payload >> 38 & 0x03;
					*(destination + 20) = payload >> 40 & 0x03;
					*(destination + 21) = payload >> 42 & 0x03;
					*(destination + 22) = payload >> 44 & 0x03;
					*(destination + 23) = payload >> 46 & 0x03;
					*(destination + 24) = payload >> 48 & 0x03;
					*(destination + 25) = payload >> 50 & 0x03;
					*(destination + 26) = payload >> 52 & 0x03;
					*(destination + 27) = payload >> 54 & 0x03;
					*(destination + 28) = payload >> 56 & 0x03;
					*(destination + 29) = payload >> 58 & 0x03;
					destination += 30;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 4:			// {"e60", 3, 20, false}
					*(destination + 0) = payload >> 0 & 0x07;
					*(destination + 1) = payload >> 3 & 0x07;
					*(destination + 2) = payload >> 6 & 0x07;
					*(destination + 3) = payload >> 9 & 0x07;
					*(destination + 4) = payload >> 12 & 0x07;
					*(destination + 5) = payload >> 15 & 0x07;
					*(destination + 6) = payload >> 18 & 0x07;
					*(destination + 7) = payload >> 21 & 0x07;
					*(destination + 8) = payload >> 24 & 0x07;
					*(destination + 9) = payload >> 27 & 0x07;
					*(destination + 10) = payload >> 30 & 0x07;
					*(destination + 11) = payload >> 33 & 0x07;
					*(destination + 12) = payload >> 36 & 0x07;
					*(destination + 13) = payload >> 39 & 0x07;
					*(destination + 14) = payload >> 42 & 0x07;
					*(destination + 15) = payload >> 45 & 0x07;
					*(destination + 16) = payload >> 48 & 0x07;
					*(destination + 17) = payload >> 51 & 0x07;
					*(destination + 18) = payload >> 54 & 0x07;
					*(destination + 19) = payload >> 57 & 0x07;
					destination += 20;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 5:			// {"f60", 4, 15, false}
					*(destination + 0) = payload >> 0 & 0x0F;
					*(destination + 1) = payload >> 4 & 0x0F;
					*(destination + 2) = payload >> 8 & 0x0F;
					*(destination + 3) = payload >> 12 & 0x0F;
					*(destination + 4) = payload >> 16 & 0x0F;
					*(destination + 5) = payload >> 20 & 0x0F;
					*(destination + 6) = payload >> 24 & 0x0F;
					*(destination + 7) = payload >> 28 & 0x0F;
					*(destination + 8) = payload >> 32 & 0x0F;
					*(destination + 9) = payload >> 36 & 0x0F;
					*(destination + 10) = payload >> 40 & 0x0F;
					*(destination + 11) = payload >> 44 & 0x0F;
					*(destination + 12) = payload >> 48 & 0x0F;
					*(destination + 13) = payload >> 52 & 0x0F;
					*(destination + 14) = payload >> 56 & 0x0F;
					destination += 15;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 6:			// {"g60", 5, 12, false}
					*(destination + 0) = payload >> 0 & 0x1F;
					*(destination + 1) = payload >> 5 & 0x1F;
					*(destination + 2) = payload >> 10 & 0x1F;
					*(destination + 3) = payload >> 15 & 0x1F;
					*(destination + 4) = payload >> 20 & 0x1F;
					*(destination + 5) = payload >> 25 & 0x1F;
					*(destination + 6) = payload >> 30 & 0x1F;
					*(destination + 7) = payload >> 35 & 0x1F;
					*(destination + 8) = payload >> 40 & 0x1F;
					*(destination + 9) = payload >> 45 & 0x1F;
					*(destination + 10) = payload >> 50 & 0x1F;
					*(destination + 11) = payload >> 55 & 0x1F;
					destination += 12;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 7:			//		{"h60", 6, 10, false}
					*(destination + 0) = payload >> 0 & 0x3F;
					*(destination + 1) = payload >> 6 & 0x3F;
					*(destination + 2) = payload >> 12 & 0x3F;
					*(destination + 3) = payload >> 18 & 0x3F;
					*(destination + 4) = payload >> 24 & 0x3F;
					*(destination + 5) = payload >> 30 & 0x3F;
					*(destination + 6) = payload >> 36 & 0x3F;
					*(destination + 7) = payload >> 42 & 0x3F;
					*(destination + 8) = payload >> 48 & 0x3F;
					*(destination + 9) = payload >> 54 & 0x3F;
					destination += 10;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 8:			//	{"i60", 7,  8, true}
					*(destination + 0) = payload >> 0 & 0x7F;
					*(destination + 1) = payload >> 7 & 0x7F;
					*(destination + 2) = payload >> 14 & 0x7F;
					*(destination + 3) = payload >> 21 & 0x7F;
					*(destination + 4) = payload >> 28 & 0x7F;
					*(destination + 5) = payload >> 35 & 0x7F;
					*(destination + 6) = payload >> 42 & 0x7F;
					*(destination + 7) = payload >> 49 & 0x7F;
					destination += 8;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 9:			//	{"j60", 8,  7, true}
					*(destination + 0) = payload >> 0 & 0xFF;
					*(destination + 1) = payload >> 8 & 0xFF;
					*(destination + 2) = payload >> 16 & 0xFF;
					*(destination + 3) = payload >> 24 & 0xFF;
					*(destination + 4) = payload >> 32 & 0xFF;
					*(destination + 5) = payload >> 40 & 0xFF;
					*(destination + 6) = payload >> 48 & 0xFF;
					destination += 7;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
 				case 10:			//	{"k60", 9, 6, true}
					*(destination + 0) = payload >> 0 & 0x1FF;
					*(destination + 1) = payload >> 9 & 0x1FF;
					*(destination + 2) = payload >> 18 & 0x1FF;
					*(destination + 3) = payload >> 27 & 0x1FF;
					*(destination + 4) = payload >> 36 & 0x1FF;
					*(destination + 5) = payload >> 45 & 0x1FF;
					destination += 6;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 11:			// {"l60", 10, 6, false}
					*(destination + 0) = payload >> 0 & 0x3FF;
					*(destination + 1) = payload >> 10 & 0x3FF;
					*(destination + 2) = payload >> 20 & 0x3FF;
					*(destination + 3) = payload >> 30 & 0x3FF;
					*(destination + 4) = payload >> 40 & 0x3FF;
					*(destination + 5) = payload >> 50 & 0x3FF;
					destination += 6;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 12:			// {"m60", 11, 5, true}
					*(destination + 0) = payload >> 0 & 0x7FF;
					*(destination + 1) = payload >> 11 & 0x7FF;
					*(destination + 2) = payload >> 22 & 0x7FF;
					*(destination + 3) = payload >> 33 & 0x7FF;
					*(destination + 4) = payload >> 44 & 0x7FF;
					destination += 5;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 13:			//		{"n60", 12, 5, false}
					*(destination + 0) = payload >> 0 & 0xFFF;
					*(destination + 1) = payload >> 12 & 0xFFF;
					*(destination + 2) = payload >> 24 & 0xFFF;
					*(destination + 3) = payload >> 36 & 0xFFF;
					*(destination + 4) = payload >> 48 & 0xFFF;
					destination += 5;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 14:			//		{"o60", 14, 4, true}
					*(destination + 0) = payload >> 0 & 0x3FFF;
					*(destination + 1) = payload >> 14 & 0x3FFF;
					*(destination + 2) = payload >> 28 & 0x3FFF;
					*(destination + 3) = payload >> 42 & 0x3FFF;
					destination += 4;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 15:			//		{"p60", 15, 4, false}
					*(destination + 0) = payload >> 0 & 0x7FFF;
					*(destination + 1) = payload >> 15 & 0x7FFF;
					*(destination + 2) = payload >> 30 & 0x7FFF;
					*(destination + 3) = payload >> 15 & 0x7FFF;
					destination += 4;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 16:			//		{"q60", 18, 3, true}
					*(destination + 0) = payload >> 0 & 0x3FFFF;
					*(destination + 1) = payload >> 18 & 0x3FFFF;
					*(destination + 2) = payload >> 36 & 0x3FFFF;
					destination += 3;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 17:			//		{"r60", 20, 3, false}
					*(destination + 0) = payload >> 0 & 0xFFFFF;
					*(destination + 1) = payload >> 20 & 0xFFFFF;
					*(destination + 2) = payload >> 40 & 0xFFFFF;
					destination += 3;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 18:			// {"s60", 28, 2, true}
					*(destination + 0) = payload >> 0 & 0xFFFFFFF;
					*(destination + 1) = payload >> 28 & 0xFFFFFFF;
					destination += 2;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 19:			//		{"t60", 30, 2, false}
					*(destination + 0) = payload >> 0 & 0x3FFFFFFF;
					*(destination + 1) = payload >> 30 & 0x3FFFFFFF;
					destination += 2;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_START		// Can't test integers 2^32
				case 20:			//		{"u60", 56, 1, false}
					*(destination + 0) = payload >> 0 & 0xFFFFFFFFFFFFFF;
					destination += 1;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 21:			//		{"v60", 60, 1, false}
					*(destination + 0) = payload >> 0 & 0x0FFFFFFFFFFFFFFF;
					destination += 1;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_STOP
				/*
					64-bit selector
				*/
				case 22:			//		{"a64", 1, 255, true}
					std::fill(destination, (destination + selector_table[22].integers), 1);
					destination += selector_table[22].integers;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 23:			//		{"a64", 1, 180, true}
					std::fill(destination, (destination + selector_table[23].integers), 1);
					destination += selector_table[23].integers;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 24:			//		{"a64", 1, 128, true}
					std::fill(destination, (destination + selector_table[24].integers), 1);
					destination += selector_table[24].integers;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 25:			//		{"d64", 1, 64, false}
					/*
						Since we're not permitted to encode a '0', a 1-bit integer must be a 1
					*/
					std::fill(destination, (destination + selector_table[25].integers), 1);
					destination += selector_table[25].integers;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 26:			//		{"e64", 2, 32, false}
					*(destination + 0) = payload >> 0 & 0x03;
					*(destination + 1) = payload >> 2 & 0x03;
					*(destination + 2) = payload >> 4 & 0x03;
					*(destination + 3) = payload >> 6 & 0x03;
					*(destination + 4) = payload >> 8 & 0x03;
					*(destination + 5) = payload >> 10 & 0x03;
					*(destination + 6) = payload >> 12 & 0x03;
					*(destination + 7) = payload >> 14 & 0x03;
					*(destination + 8) = payload >> 16 & 0x03;
					*(destination + 9) = payload >> 18 & 0x03;
					*(destination + 10) = payload >> 20 & 0x03;
					*(destination + 11) = payload >> 22 & 0x03;
					*(destination + 12) = payload >> 24 & 0x03;
					*(destination + 13) = payload >> 26 & 0x03;
					*(destination + 14) = payload >> 28 & 0x03;
					*(destination + 15) = payload >> 30 & 0x03;
					*(destination + 16) = payload >> 32 & 0x03;
					*(destination + 17) = payload >> 34 & 0x03;
					*(destination + 18) = payload >> 36 & 0x03;
					*(destination + 19) = payload >> 38 & 0x03;
					*(destination + 20) = payload >> 40 & 0x03;
					*(destination + 21) = payload >> 42 & 0x03;
					*(destination + 22) = payload >> 44 & 0x03;
					*(destination + 23) = payload >> 46 & 0x03;
					*(destination + 24) = payload >> 48 & 0x03;
					*(destination + 25) = payload >> 50 & 0x03;
					*(destination + 26) = payload >> 52 & 0x03;
					*(destination + 27) = payload >> 54 & 0x03;
					*(destination + 28) = payload >> 56 & 0x03;
					*(destination + 29) = payload >> 58 & 0x03;
					*(destination + 30) = payload >> 60 & 0x03;
					*(destination + 31) = payload >> 62 & 0x03;
					destination += 32;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 27:			//		{"f64", 3, 21, false}
					*(destination + 0) = payload >> 0 & 0x07;
					*(destination + 1) = payload >> 3 & 0x07;
					*(destination + 2) = payload >> 6 & 0x07;
					*(destination + 3) = payload >> 9 & 0x07;
					*(destination + 4) = payload >> 12 & 0x07;
					*(destination + 5) = payload >> 15 & 0x07;
					*(destination + 6) = payload >> 18 & 0x07;
					*(destination + 7) = payload >> 21 & 0x07;
					*(destination + 8) = payload >> 24 & 0x07;
					*(destination + 9) = payload >> 27 & 0x07;
					*(destination + 10) = payload >> 30 & 0x07;
					*(destination + 11) = payload >> 33 & 0x07;
					*(destination + 12) = payload >> 36 & 0x07;
					*(destination + 13) = payload >> 39 & 0x07;
					*(destination + 14) = payload >> 42 & 0x07;
					*(destination + 15) = payload >> 45 & 0x07;
					*(destination + 16) = payload >> 48 & 0x07;
					*(destination + 17) = payload >> 51 & 0x07;
					*(destination + 18) = payload >> 54 & 0x07;
					*(destination + 19) = payload >> 57 & 0x07;
					*(destination + 20) = payload >> 60 & 0x07;
					destination += 21;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 28:			//	{"g64", 4, 16, false}
					*(destination + 0) = payload >> 0 & 0x0F;
					*(destination + 1) = payload >> 4 & 0x0F;
					*(destination + 2) = payload >> 8 & 0x0F;
					*(destination + 3) = payload >> 12 & 0x0F;
					*(destination + 4) = payload >> 16 & 0x0F;
					*(destination + 5) = payload >> 20 & 0x0F;
					*(destination + 6) = payload >> 24 & 0x0F;
					*(destination + 7) = payload >> 28 & 0x0F;
					*(destination + 8) = payload >> 32 & 0x0F;
					*(destination + 9) = payload >> 36 & 0x0F;
					*(destination + 10) = payload >> 40 & 0x0F;
					*(destination + 11) = payload >> 44 & 0x0F;
					*(destination + 12) = payload >> 48 & 0x0F;
					*(destination + 13) = payload >> 52 & 0x0F;
					*(destination + 14) = payload >> 56 & 0x0F;
					*(destination + 15) = payload >> 60 & 0x0F;
					destination += 16;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 29:			//	{"h64", 5, 12, true}
					*(destination + 0) = payload >> 0 & 0x1F;
					*(destination + 1) = payload >> 5 & 0x1F;
					*(destination + 2) = payload >> 10 & 0x1F;
					*(destination + 3) = payload >> 15 & 0x1F;
					*(destination + 4) = payload >> 20 & 0x1F;
					*(destination + 5) = payload >> 25 & 0x1F;
					*(destination + 6) = payload >> 30 & 0x1F;
					*(destination + 7) = payload >> 35 & 0x1F;
					*(destination + 8) = payload >> 40 & 0x1F;
					*(destination + 9) = payload >> 45 & 0x1F;
					*(destination + 10) = payload >> 50 & 0x1F;
					*(destination + 11) = payload >> 55 & 0x1F;
					destination += 12;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 30:			//		{"i64", 6, 10, true}
					*(destination + 0) = payload >> 0 & 0x3F;
					*(destination + 1) = payload >> 6 & 0x3F;
					*(destination + 2) = payload >> 12 & 0x3F;
					*(destination + 3) = payload >> 18 & 0x3F;
					*(destination + 4) = payload >> 24 & 0x3F;
					*(destination + 5) = payload >> 30 & 0x3F;
					*(destination + 6) = payload >> 36 & 0x3F;
					*(destination + 7) = payload >> 42 & 0x3F;
					*(destination + 8) = payload >> 48 & 0x3F;
					*(destination + 9) = payload >> 54 & 0x3F;
					destination += 10;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 31:			//		{"j64", 7, 9, false}
					*(destination + 0) = payload >> 0 & 0x7F;
					*(destination + 1) = payload >> 7 & 0x7F;
					*(destination + 2) = payload >> 14 & 0x7F;
					*(destination + 3) = payload >> 21 & 0x7F;
					*(destination + 4) = payload >> 28 & 0x7F;
					*(destination + 5) = payload >> 35 & 0x7F;
					*(destination + 6) = payload >> 42 & 0x7F;
					*(destination + 7) = payload >> 49 & 0x7F;
					*(destination + 8) = payload >> 56 & 0x7F;
					destination += 9;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 32:			//	{"k64", 8, 8, false}
					*(destination + 0) = payload >> 0 & 0xFF;
					*(destination + 1) = payload >> 8 & 0xFF;
					*(destination + 2) = payload >> 16 & 0xFF;
					*(destination + 3) = payload >> 24 & 0xFF;
					*(destination + 4) = payload >> 32 & 0xFF;
					*(destination + 5) = payload >> 40 & 0xFF;
					*(destination + 6) = payload >> 48 & 0xFF;
					*(destination + 7) = payload >> 56 & 0xFF;
					destination += 8;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 33:			//	{"l64", 9, 7, false}
					*(destination + 0) = payload >> 0 & 0x1FF;
					*(destination + 1) = payload >> 9 & 0x1FF;
					*(destination + 2) = payload >> 18 & 0x1FF;
					*(destination + 3) = payload >> 27 & 0x1FF;
					*(destination + 4) = payload >> 36 & 0x1FF;
					*(destination + 5) = payload >> 45 & 0x1FF;
					*(destination + 6) = payload >> 54 & 0x1FF;
					destination += 7;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 34:			// {"m64", 10, 6, true}
					*(destination + 0) = payload >> 0 & 0x3FF;
					*(destination + 1) = payload >> 10 & 0x3FF;
					*(destination + 2) = payload >> 20 & 0x3FF;
					*(destination + 3) = payload >> 30 & 0x3FF;
					*(destination + 4) = payload >> 40 & 0x3FF;
					*(destination + 5) = payload >> 50 & 0x3FF;
					destination += 6;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 35:			// {"n64", 12, 5, true}
					*(destination + 0) = payload >> 0 & 0xFFF;
					*(destination + 1) = payload >> 12 & 0xFFF;
					*(destination + 2) = payload >> 24 & 0xFFF;
					*(destination + 3) = payload >> 36 & 0xFFF;
					*(destination + 4) = payload >> 48 & 0xFFF;
					destination += 5;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 36:			//		{"o64", 15, 4, true}
					*(destination + 0) = payload >> 0 & 0x7FFF;
					*(destination + 1) = payload >> 15 & 0x7FFF;
					*(destination + 2) = payload >> 30 & 0x7FFF;
					*(destination + 3) = payload >> 45 & 0x7FFF;
					destination += 4;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 37:			//		{"p64", 16, 4, false}
					*(destination + 0) = payload >> 0 & 0xFFFF;
					*(destination + 1) = payload >> 16 & 0xFFFF;
					*(destination + 2) = payload >> 32 & 0xFFFF;
					*(destination + 3) = payload >> 48 & 0xFFFF;
					destination += 4;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 38:			//		{"q64", 20, 3, true}
					*(destination + 0) = payload >> 0 & 0xFFFFF;
					*(destination + 1) = payload >> 20 & 0xFFFFF;
					*(destination + 2) = payload >> 40 & 0xFFFFF;
					destination += 3;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 39:			//		{"r64", 21, 3, false}
					*(destination + 0) = payload >> 0 & 0x1FFFFF;
					*(destination + 1) = payload >> 21 & 0x1FFFFF;
					*(destination + 2) = payload >> 42 & 0x1FFFFF;
					destination += 3;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 40:			//		{"s64", 30, 2, true}
					*(destination + 0) = payload >> 0 & 0x3FFFFFFF;
					*(destination + 1) = payload >> 30 & 0x3FFFFFFF;
					destination += 2;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 41:			//		{"t64", 32, 2, false}
					*(destination + 0) = payload >> 0 & 0xFFFFFFFF;
					*(destination + 1) = payload >> 32 & 0xFFFFFFFF;
					destination += 2;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_START		// Can't test integers 2^32
				case 42:			//		{"u64", 60, 1, true}
					*(destination + 0) = payload >> 0 & 0x0FFFFFFFFFFFFFFF;
					destination += 1;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 43:			//		{"v64", 64, 1, false},
					*(destination + 0) = payload >> 0 & 0xFFFFFFFFFFFFFFFF;
					destination += 1;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_STOP
				/*
					57-bit selector
				*/
				case 44:			//   {"a57", 1, 57, false}
					*(destination + 0) = payload >> 0 & 0x01;
					*(destination + 1) = payload >> 1 & 0x01;
					*(destination + 2) = payload >> 2 & 0x01;
					*(destination + 3) = payload >> 3 & 0x01;
					*(destination + 4) = payload >> 4 & 0x01;
					*(destination + 5) = payload >> 5 & 0x01;
					*(destination + 6) = payload >> 6 & 0x01;
					*(destination + 7) = payload >> 7 & 0x01;
					*(destination + 8) = payload >> 8 & 0x01;
					*(destination + 9) = payload >> 9 & 0x01;
					*(destination + 10) = payload >> 10 & 0x01;
					*(destination + 11) = payload >> 11 & 0x01;
					*(destination + 12) = payload >> 12 & 0x01;
					*(destination + 13) = payload >> 13 & 0x01;
					*(destination + 14) = payload >> 14 & 0x01;
					*(destination + 15) = payload >> 15 & 0x01;
					*(destination + 16) = payload >> 16 & 0x01;
					*(destination + 17) = payload >> 17 & 0x01;
					*(destination + 18) = payload >> 18 & 0x01;
					*(destination + 19) = payload >> 19 & 0x01;
					*(destination + 20) = payload >> 20 & 0x01;
					*(destination + 21) = payload >> 21 & 0x01;
					*(destination + 22) = payload >> 22 & 0x01;
					*(destination + 23) = payload >> 23 & 0x01;
					*(destination + 24) = payload >> 24 & 0x01;
					*(destination + 25) = payload >> 25 & 0x01;
					*(destination + 26) = payload >> 26 & 0x01;
					*(destination + 27) = payload >> 27 & 0x01;
					*(destination + 28) = payload >> 28 & 0x01;
					*(destination + 29) = payload >> 29 & 0x01;
					*(destination + 30) = payload >> 30 & 0x01;
					*(destination + 31) = payload >> 31 & 0x01;
					*(destination + 32) = payload >> 32 & 0x01;
					*(destination + 33) = payload >> 33 & 0x01;
					*(destination + 34) = payload >> 34 & 0x01;
					*(destination + 35) = payload >> 35 & 0x01;
					*(destination + 36) = payload >> 36 & 0x01;
					*(destination + 37) = payload >> 37 & 0x01;
					*(destination + 38) = payload >> 38 & 0x01;
					*(destination + 39) = payload >> 39 & 0x01;
					*(destination + 40) = payload >> 40 & 0x01;
					*(destination + 41) = payload >> 41 & 0x01;
					*(destination + 42) = payload >> 42 & 0x01;
					*(destination + 43) = payload >> 43 & 0x01;
					*(destination + 44) = payload >> 44 & 0x01;
					*(destination + 45) = payload >> 45 & 0x01;
					*(destination + 46) = payload >> 46 & 0x01;
					*(destination + 47) = payload >> 47 & 0x01;
					*(destination + 48) = payload >> 48 & 0x01;
					*(destination + 49) = payload >> 49 & 0x01;
					*(destination + 50) = payload >> 50 & 0x01;
					*(destination + 51) = payload >> 51 & 0x01;
					*(destination + 52) = payload >> 52 & 0x01;
					*(destination + 53) = payload >> 53 & 0x01;
					*(destination + 54) = payload >> 54 & 0x01;
					*(destination + 55) = payload >> 55 & 0x01;
					*(destination + 56) = payload >> 56 & 0x01;
					destination += 57;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 45:			//   {"b57", 2, 28, false}
					*(destination + 0) = payload >> 0 & 0x03;
					*(destination + 1) = payload >> 2 & 0x03;
					*(destination + 2) = payload >> 4 & 0x03;
					*(destination + 3) = payload >> 6 & 0x03;
					*(destination + 4) = payload >> 8 & 0x03;
					*(destination + 5) = payload >> 10 & 0x03;
					*(destination + 6) = payload >> 12 & 0x03;
					*(destination + 7) = payload >> 14 & 0x03;
					*(destination + 8) = payload >> 16 & 0x03;
					*(destination + 9) = payload >> 18 & 0x03;
					*(destination + 10) = payload >> 20 & 0x03;
					*(destination + 11) = payload >> 22 & 0x03;
					*(destination + 12) = payload >> 24 & 0x03;
					*(destination + 13) = payload >> 26 & 0x03;
					*(destination + 14) = payload >> 28 & 0x03;
					*(destination + 15) = payload >> 30 & 0x03;
					*(destination + 16) = payload >> 32 & 0x03;
					*(destination + 17) = payload >> 34 & 0x03;
					*(destination + 18) = payload >> 36 & 0x03;
					*(destination + 19) = payload >> 38 & 0x03;
					*(destination + 20) = payload >> 40 & 0x03;
					*(destination + 21) = payload >> 42 & 0x03;
					*(destination + 22) = payload >> 44 & 0x03;
					*(destination + 23) = payload >> 46 & 0x03;
					*(destination + 24) = payload >> 48 & 0x03;
					*(destination + 25) = payload >> 50 & 0x03;
					*(destination + 26) = payload >> 52 & 0x03;
					*(destination + 27) = payload >> 54 & 0x03;
					destination += 28;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 46:			//   {"c57", 3, 19, false}
					*(destination + 0) = payload >> 0 & 0x07;
					*(destination + 1) = payload >> 3 & 0x07;
					*(destination + 2) = payload >> 6 & 0x07;
					*(destination + 3) = payload >> 9 & 0x07;
					*(destination + 4) = payload >> 12 & 0x07;
					*(destination + 5) = payload >> 15 & 0x07;
					*(destination + 6) = payload >> 18 & 0x07;
					*(destination + 7) = payload >> 21 & 0x07;
					*(destination + 8) = payload >> 24 & 0x07;
					*(destination + 9) = payload >> 27 & 0x07;
					*(destination + 10) = payload >> 30 & 0x07;
					*(destination + 11) = payload >> 33 & 0x07;
					*(destination + 12) = payload >> 36 & 0x07;
					*(destination + 13) = payload >> 39 & 0x07;
					*(destination + 14) = payload >> 42 & 0x07;
					*(destination + 15) = payload >> 45 & 0x07;
					*(destination + 16) = payload >> 48 & 0x07;
					*(destination + 17) = payload >> 51 & 0x07;
					*(destination + 18) = payload >> 54 & 0x07;
					destination += 19;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 47:			//   {"d57", 4, 14, false}
					*(destination + 0) = payload >> 0 & 0x0F;
					*(destination + 1) = payload >> 4 & 0x0F;
					*(destination + 2) = payload >> 8 & 0x0F;
					*(destination + 3) = payload >> 12 & 0x0F;
					*(destination + 4) = payload >> 16 & 0x0F;
					*(destination + 5) = payload >> 20 & 0x0F;
					*(destination + 6) = payload >> 24 & 0x0F;
					*(destination + 7) = payload >> 28 & 0x0F;
					*(destination + 8) = payload >> 32 & 0x0F;
					*(destination + 9) = payload >> 36 & 0x0F;
					*(destination + 10) = payload >> 40 & 0x0F;
					*(destination + 11) = payload >> 44 & 0x0F;
					*(destination + 12) = payload >> 48 & 0x0F;
					*(destination + 13) = payload >> 52 & 0x0F;
					destination += 14;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 48:			//   {"e57", 5, 11, false}
					*(destination + 0) = payload >> 0 & 0x1F;
					*(destination + 1) = payload >> 5 & 0x1F;
					*(destination + 2) = payload >> 10 & 0x1F;
					*(destination + 3) = payload >> 15 & 0x1F;
					*(destination + 4) = payload >> 20 & 0x1F;
					*(destination + 5) = payload >> 25 & 0x1F;
					*(destination + 6) = payload >> 30 & 0x1F;
					*(destination + 7) = payload >> 35 & 0x1F;
					*(destination + 8) = payload >> 40 & 0x1F;
					*(destination + 9) = payload >> 45 & 0x1F;
					*(destination + 10) = payload >> 50 & 0x1F;
					destination += 11;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 49:			//   {"f57", 6, 9, false}
					*(destination + 0) = payload >> 0 & 0x3F;
					*(destination + 1) = payload >> 6 & 0x3F;
					*(destination + 2) = payload >> 12 & 0x3F;
					*(destination + 3) = payload >> 18 & 0x3F;
					*(destination + 4) = payload >> 24 & 0x3F;
					*(destination + 5) = payload >> 30 & 0x3F;
					*(destination + 6) = payload >> 36 & 0x3F;
					*(destination + 7) = payload >> 42 & 0x3F;
					*(destination + 8) = payload >> 48 & 0x3F;
					destination += 9;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 50:			//   {"g57", 7, 8, false}
					*(destination + 0) = payload >> 0 & 0x7F;
					*(destination + 1) = payload >> 7 & 0x7F;
					*(destination + 2) = payload >> 14 & 0x7F;
					*(destination + 3) = payload >> 21 & 0x7F;
					*(destination + 4) = payload >> 28 & 0x7F;
					*(destination + 5) = payload >> 35 & 0x7F;
					*(destination + 6) = payload >> 42 & 0x7F;
					*(destination + 7) = payload >> 49 & 0x7F;
					destination += 8;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 51:			//   {"h57", 8, 7, false}
					*(destination + 0) = payload >> 0 & 0xFF;
					*(destination + 1) = payload >> 8 & 0xFF;
					*(destination + 2) = payload >> 16 & 0xFF;
					*(destination + 3) = payload >> 24 & 0xFF;
					*(destination + 4) = payload >> 32 & 0xFF;
					*(destination + 5) = payload >> 40 & 0xFF;
					*(destination + 6) = payload >> 48 & 0xFF;
					destination += 7;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 52:			//	{"i57", 9, 6, false}
					*(destination + 0) = payload >> 0 & 0x1FF;
					*(destination + 1) = payload >> 9 & 0x1FF;
					*(destination + 2) = payload >> 18 & 0x1FF;
					*(destination + 3) = payload >> 27 & 0x1FF;
					*(destination + 4) = payload >> 36 & 0x1FF;
					*(destination + 5) = payload >> 45 & 0x1FF;
					destination += 6;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 53:			//	{"j57", 10, 5, true}
					*(destination + 0) = payload >> 0 & 0x3FF;
					*(destination + 1) = payload >> 10 & 0x3FF;
					*(destination + 2) = payload >> 20 & 0x3FF;
					*(destination + 3) = payload >> 30 & 0x3FF;
					*(destination + 4) = payload >> 40 & 0x3FF;
					destination += 5;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 54:			//	{"k57", 11, 5, false}
					*(destination + 0) = payload >> 0 & 0x7FF;
					*(destination + 1) = payload >> 11 & 0x7FF;
					*(destination + 2) = payload >> 22 & 0x7FF;
					*(destination + 3) = payload >> 33 & 0x7FF;
					*(destination + 4) = payload >> 44 & 0x7FF;
					destination += 5;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 55:			//	{"l57", 12, 4, true}
					*(destination + 0) = payload >> 0 & 0xFFF;
					*(destination + 1) = payload >> 12 & 0xFFF;
					*(destination + 2) = payload >> 24 & 0xFFF;
					*(destination + 3) = payload >> 36 & 0xFFF;
					destination += 4;

					selector = *source >> 60;
					source++;
					payload = *source;
					base = sixty_four_start;
					break;
				case 56:			//	{"m57", 14, 4, false}
					*(destination + 0) = payload >> 0 & 0x3FFF;
					*(destination + 1) = payload >> 14 & 0x3FFF;
					*(destination + 2) = payload >> 28 & 0x3FFF;
					*(destination + 3) = payload >> 42 & 0x3FFF;
					destination += 4;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 57:			//	{"n57", 19, 3, false}
					*(destination + 0) = payload >> 0 & 0x7FFFF;
					*(destination + 1) = payload >> 19 & 0x7FFFF;
					*(destination + 2) = payload >> 38 & 0x7FFFF;
					destination += 3;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
				case 58:			//	 {"o57", 28, 2, false}
					*(destination + 0) = payload >> 0 & 0xFFFFFFF;
					*(destination + 1) = payload >> 28 & 0xFFFFFFF;
					destination += 2;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_START		// Can't test integers 2^32
				case 59:			//	 {"p57", 57, 1, false}
					*(destination + 0) = payload >> 0 & 0x1FFFFFFFFFFFFFF;
					destination += 1;

					source++;
					selector = *source & 0x0F;
					payload = *source >> 4;
					base = sixty_start;
					break;
// LCOV_EXCL_STOP
				}
			}
		}

	/*
		COMPRESS_INTEGER_CARRY_8B::UNITTEST()
		-------------------------------------
	*/
	void compress_integer_carry_8b::unittest(void)
		{
		std::vector<integer> every_case;
		size_t instance;

		/*
			Start with an offset of 3.
		*/
		for (instance = 0; instance < 2; instance++)			// 2*28-bit (58)
			every_case.push_back(0x0FFFFFFF);
		for (instance = 0; instance < 30; instance++)		// 30*2-bit (3)
			every_case.push_back(0x03);
		for (instance = 0; instance < 20; instance++)		// 20*3-bit (4)
			every_case.push_back(0x07);
		for (instance = 0; instance < 15; instance++)		// 15*4-bit (5)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 12; instance++)		// 12*5-bit (6)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 10; instance++)		// 10*6-bit (7)
			every_case.push_back(0x3F);
		for (instance = 0; instance < 6; instance++)			// 6*10-bit (11)
			every_case.push_back(0x03FF);
		for (instance = 0; instance < 5; instance++)			// 5*12-bit (13)
			every_case.push_back(0x0FFF);
		for (instance = 0; instance < 4; instance++)			// 4*15-bit (15)
			every_case.push_back(0x7FFF);
		for (instance = 0; instance < 3; instance++)			// 3*20-bit (17)
			every_case.push_back(0xFFFFF);
		for (instance = 0; instance < 8; instance++)			// 8*7-bit (8)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 12; instance++)		// 12*5-bit (29)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 10; instance++)		// 12*5-bit (30)
			every_case.push_back(0x3F);
		for (instance = 0; instance < 6; instance++)			// 6*10-bit (34)
			every_case.push_back(0x03FF);
		for (instance = 0; instance < 5; instance++)			// 5*12-bit (35)
			every_case.push_back(0x0FFF);
		for (instance = 0; instance < 4; instance++)			// 4*15-bit (36)
			every_case.push_back(0x7FFF);
		for (instance = 0; instance < 3; instance++)			// 3*20-bit (38)
			every_case.push_back(0x0FFFFF);
		for (instance = 0; instance < 64; instance++)		// 64*1-bit (25)
			every_case.push_back(0x01);
		for (instance = 0; instance < 7; instance++)			// 7*8-bit (9)
			every_case.push_back(0xFF);
		for (instance = 0; instance < 32; instance++)		// 32*2-bit (26)
			every_case.push_back(0x03);
		for (instance = 0; instance < 6; instance++)			// 6*9-bit (10)
			every_case.push_back(0x01FF);
		for (instance = 0; instance < 21; instance++)		// 21*3-bit (27)
			every_case.push_back(0x07);
		for (instance = 0; instance < 5; instance++)			// 5*11-bit (12)
			every_case.push_back(0x07FF);
		for (instance = 0; instance < 16; instance++)		// 16*4-bit (28)
			every_case.push_back(0x0F);
		for (instance = 0; instance < 4; instance++)			// 4*14-bit (14)
			every_case.push_back(0x3FFF);
		for (instance = 0; instance < 9; instance++)			// 9*7-bit (31)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 3; instance++)			// 3*18-bit (16)
			every_case.push_back(0x3FFFF);
		for (instance = 0; instance < 8; instance++)			// 8*8-bit (32)
			every_case.push_back(0xFF);
		for (instance = 0; instance < 2; instance++)			// 2*28-bit (18)
			every_case.push_back(0x0FFFFFFF);
		for (instance = 0; instance < 7; instance++)			// 7*9-bit (33)
			every_case.push_back(0x01FF);
		for (instance = 0; instance < 8; instance++)			// 8*7-bit (8)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 4; instance++)			// 4*16-bit (37)
			every_case.push_back(0xFFFF);
		for (instance = 0; instance < 8; instance++)			// 8*7-bit (8)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 3; instance++)			// 3*21-bit (39)
			every_case.push_back(0x1FFFFF);

		compress_integer_carry_8b compressor;
		std::vector<uint32_t>compressed(every_case.size() * 2);
		std::vector<uint32_t>decompressed(every_case.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor.decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);

		/*
			Offset of 0
		*/
		every_case.clear();
		for (instance = 0; instance < 28; instance++)			// 28*2-bit (45)
			every_case.push_back(0x03);
		for (instance = 0; instance < 60; instance++)			// 60*1-bit (2)
			every_case.push_back(0x01);
		for (instance = 0; instance < 30; instance++)			// 30*2-bit (3)
			every_case.push_back(0x03);
		for (instance = 0; instance < 128; instance++)			// 128*1-bit (1)
			every_case.push_back(0x01);
		for (instance = 0; instance < 12; instance++)			// 12*5-bit (29)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 255; instance++)			// 255*1-bit (22)
			every_case.push_back(0x01);
		for (instance = 0; instance < 180; instance++)			// 180*1-bit (23)
			every_case.push_back(0x01);
		for (instance = 0; instance < 12; instance++)			// 12*5-bit (29)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 120; instance++)			// 120*1-bit (24)
			every_case.push_back(0x01);
		for (instance = 0; instance < 32; instance++)			// 32*2-bit (26)
			every_case.push_back(0x03);
		for (instance = 0; instance < 10; instance++)			// 10*6-bit (7)
			every_case.push_back(0x3F);
		for (instance = 0; instance < 255; instance++)			// 255*1-bit (0)
			every_case.push_back(0x01);
		for (instance = 0; instance < 12; instance++)			// 12*5-bit (29)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 64; instance++)			// 64*1-bit (25)
			every_case.push_back(0x01);
		for (instance = 0; instance < 12; instance++)			// 12*5-bit (29)
			every_case.push_back(0x1F);

		decompressed.resize(every_case.size() + 256);
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor.decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);


#ifdef NEVER
		/*
			FIX THIS:  when using 57 bits the carried over selector is in the wrong place.
		*/
		/*
			Offset as high as possible with 32-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 4; instance++)			// 4*12-bit (55)
			every_case.push_back(0x0FFF);
		for (instance = 0; instance < 2; instance++)			// 2*30-bit (40)
			every_case.push_back(0x3FFFFFFF);
		for (instance = 0; instance < 2; instance++)			// 2*32-bit (41)
			every_case.push_back(0xFFFFFFFF);
		for (instance = 0; instance < 2; instance++)			// 2*30-bit (19)
			every_case.push_back(0x3FFFFFFF);

		decompressed.resize(every_case.size() + 256);
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor.decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);

#endif
		/*
			Try the error cases
			(1) 1 integer (encoded with Simple-9)
			(2) no integers
			(3) Integer overflow
			(4) Integer overflow in the Relative-10 encoder
			(5) buffer overflow on simple-9 encoder
			(6) buffer overflow on Relatice-10 encoder
		*/
		compressor.decode(&decompressed[0], 1, &compressed[0], size_once_compressed);
		JASS_assert(decompressed[0] == every_case[0]);

		integer one = 1;
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &one, 0);
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		every_case.clear();
		every_case.push_back(0x0FFFFFFF);
		every_case.push_back(0xFFFFFFFF);
		size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 16);			// this doesn't fail because we're encoding in 64-bit integere

		every_case.clear();
		for (size_t instance = 0; instance < 28; instance++)
			every_case.push_back(0x01);
		size_once_compressed = compressor.encode(&compressed[0], 1, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		for (size_t instance = 0; instance < 28; instance++)
			every_case.push_back(0xFF);
		size_once_compressed = compressor.encode(&compressed[0], 5, &every_case[0], every_case.size());
		JASS_assert(size_once_compressed == 0);

		puts("compress_integer_carry_8b::PASSED");
		}
	}

