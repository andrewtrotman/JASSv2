/*
	COMPRESS_INTEGER_CARRYOVER_12.CPP
	---------------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_carryover_12.h"


/*
	By defining CARRY_DEBUG this code will dump out encoding and decoding details.
*/
//#define CARRY_DEBUG

namespace JASS
	{
	const compress_integer_carryover_12::selector compress_integer_carryover_12::transition_table[] =
		{
			/* Selector in the 32-bit integer (30-bit payload) */
		/*0*/  {"a30", 1, 30, false, {0, 1, 2, 11}},
		/*1*/  {"b30", 2, 15, false, {0, 1, 2, 11}},
		/*2*/  {"c30", 3, 10, false, {1, 2, 3, 11}},
		/*3*/  {"d30", 4,  7, true,  {14, 15, 16, 23}},
		/*4*/  {"e30", 5, 6, false, {3, 4, 5, 11}},
		/*5*/  {"f30", 6, 5, false, {4, 5, 6, 11}},
		/*6*/  {"g30", 7, 4, true, {17, 18, 19, 23}},
		/*7*/  {"h30", 9, 3, true, {18, 19, 20, 23}},
		/*8*/  {"i30", 10, 3, false, {7, 8, 9, 11}},
		/*9*/  {"j30", 14, 2, true, {20, 21, 22, 23}},
		/*10*/ {"k30", 15, 2, false, {8, 9, 10, 11}},
		/*11*/ {"l30", 28, 1, true, {17, 20, 22, 23}},

			/* Selector in the previous 32-bit integer (32-bit payload) */
		/*12*/ {"a32", 1, 32, false, {0, 1, 2, 11}},
		/*13*/ {"b32", 2, 16, false, {0, 1, 2, 11}},
		/*14*/ {"c32", 3, 10, true, {13, 14, 15, 23}},
		/*15*/ {"d32", 4, 8, false, {2, 3, 4, 11}},
		/*16*/ {"e32", 5, 6, true, {15, 16, 17, 23}},
		/*17*/ {"f32", 6, 5, true, {16, 17, 18, 23}},
		/*18*/ {"g32", 7, 4, true, {17, 18, 19, 23}},
		/*19*/ {"h32", 8, 4, false, {6, 7, 8, 11}},
		/*20*/ {"i32", 10, 3, true, {19, 20, 21, 23}},
		/*21*/ {"j32", 15, 2, true, {20, 21, 22, 23}},
		/*22*/ {"k32", 16, 2, false, {8, 9, 10, 11}},
		/*23*/ {"l32", 30, 1, true, {17, 20, 22, 23}},
		};

	/*
		COMPRESS_INTEGER_CARRYOVER_12::ENCODE()
		---------------------------------------
	*/
	size_t compress_integer_carryover_12::encode(void *encoded, size_t destination_length, const integer *source, size_t source_integers)
		{
		const integer *from = source;
		uint32_t *destination = static_cast<uint32_t *>(encoded);
		uint32_t *end = destination + (destination_length >> 2);

		/*
			Check for 0 input or 0 sized output buffer
		*/
		if (source_integers == 0)
			return 0;

		if (destination_length < 4)
			return 0;

		/*
			The paper states: Encoder and decoder must agree on an initial value of “current selector” in order to process the first word.
			In all of the experiments described here, row j was assumed as an initial configuration.".  That is, 1 30-bit integer.  Because of
			this we can set the first selector to 00 and that fills the codeword.
		*/
		uint32_t current_selector = 23;	// first selector is 1 integer of 28-bits
		if (maths::ceiling_log2(*from) > transition_table[current_selector].bits)
			return 0;			// the first integer does not fit into the first encoded word.
		*destination++ = *from;
		bool next_selector_in_previous_word = true;
		size_t used = 1;
#ifdef CARRY_DEBUG
printf("source[0] %d * %d-bits [23]\n", (int)transition_table[current_selector].integers, (int)transition_table[current_selector].bits);
#endif
		/*
			Now encode the remainder using the transition tables.
		*/
		do
			{
			/*
				Check that the next codeword fits into the output buffer
			*/
			if (destination >= end)
				return 0;

			/*
				Start at this selectors lowest possible tranition (that has the most possible integers in it)
			*/
			uint32_t selector = 0;
			uint32_t terms;
			uint32_t trial;

			/*
				Find out how many integers we can pack
			*/
			trial = transition_table[current_selector].new_selector[selector];
			do
				{
				/*
					Try each selector starting with the most dense selector first
				*/
				for (terms = 0; terms < transition_table[trial].integers && used + terms < source_integers; terms++)
					if (maths::ceiling_log2(from[used + terms]) > transition_table[trial].bits)
						{
						selector++;
						break;
						}

				/*
					if we fit then we've got a selector
				*/
				if (selector < 4)
					{
					trial = transition_table[current_selector].new_selector[selector];
					if (terms >= transition_table[trial].integers || used + terms >= source_integers)
						break;
					}
				}
			while (selector < 4);

			/*
				If we exceed the maximum number allowed in the worst transition then we must have an integer that is too large (i.e. x > 2^28 or x > 2^30)
			*/
			if (selector >= 4)
				return 0;

			uint32_t bits_per_integer = transition_table[trial].bits;
			terms = transition_table[trial].integers;

			/*
				Pack into an integer;
			*/
			uint32_t word = 0;
			for (int term = terms - 1; term >= 0; term--)
				{
				uint32_t value = (used + term >= source_integers) ? 0 : from[used + term];		// Make sure we don't overflow the input buffer
				word = word << bits_per_integer | value;
				}

			/*
				add the selector
			*/
			if (next_selector_in_previous_word)
				*(destination - 1) |= selector << 30;			// shove it in the high 2 bits.
			else
				word = word << 2 | selector;						// shove it in the low 2 bits on the current word.

#ifdef CARRY_DEBUG
printf("source[%d] %d * %d-bits [%d->%d]\n", (int)used, (int)transition_table[trial].integers, (int)transition_table[trial].bits, (int)selector, (int)trial);
#endif

			*destination++ = word;
			used += terms;
			next_selector_in_previous_word = transition_table[trial].next_selector;
			current_selector = trial;
			}
		while (used < source_integers);

		return reinterpret_cast<uint8_t *>(destination) - reinterpret_cast<uint8_t *>(encoded);
		}

	/*
		COMPRESS_INTEGER_CARRYOVER_12::DECODE()
		---------------------------------------
	*/
	void compress_integer_carryover_12::decode(integer *destination, size_t integers_to_decode, const void *compressed, size_t compressed_size_in_bytes)
		{
#ifdef CARRY_DEBUG
		integer *start_of_output = destination;
#endif
		const integer *end = destination + integers_to_decode;
		const uint32_t *source = static_cast<const uint32_t *>(compressed);

		/*
			Get the initial selector
		*/
		size_t selector = 23;						// first selector is 1 integer of 28 bits
		size_t payload = *source;

		while (destination < end)
			{
#ifdef CARRY_DEBUG
printf("destination[%d] ", (int)(destination - start_of_output));
#endif
			switch (selector)
				{
				/*
					30-bit payload
				*/
				case 0:
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
					destination += 30;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 1:
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
					destination += 15;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 2:
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
					destination += 10;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 3:
					*(destination + 0) = payload >> 0 & 0x0F;
					*(destination + 1) = payload >> 4 & 0x0F;
					*(destination + 2) = payload >> 8 & 0x0F;
					*(destination + 3) = payload >> 12 & 0x0F;
					*(destination + 4) = payload >> 16 & 0x0F;
					*(destination + 5) = payload >> 20 & 0x0F;
					*(destination + 6) = payload >> 24 & 0x0F;
					destination += 7;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 28) & 0x03), (int)transition_table[selector].new_selector[(payload >> 28) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 28) & 0x03];
					source++;
					payload = *source;
					break;
				case 4:
					*(destination + 0) = payload >> 0 & 0x1F;
					*(destination + 1) = payload >> 5 & 0x1F;
					*(destination + 2) = payload >> 10 & 0x1F;
					*(destination + 3) = payload >> 15 & 0x1F;
					*(destination + 4) = payload >> 20 & 0x1F;
					*(destination + 5) = payload >> 25 & 0x1F;
					destination += 6;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 5:
					*(destination + 0) = payload >> 0 & 0x3F;
					*(destination + 1) = payload >> 6 & 0x3F;
					*(destination + 2) = payload >> 12 & 0x3F;
					*(destination + 3) = payload >> 18 & 0x3F;
					*(destination + 4) = payload >> 24 & 0x3F;
					destination += 5;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 6:
					*(destination + 0) = payload >> 0 & 0x7F;
					*(destination + 1) = payload >> 7 & 0x7F;
					*(destination + 2) = payload >> 14 & 0x7F;
					*(destination + 3) = payload >> 21 & 0x7F;
					destination += 4;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 28) & 0x03), (int)transition_table[selector].new_selector[(payload >> 28) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 28) & 0x03];
					source++;
					payload = *source;
					break;
				case 7:
					*(destination + 0) = payload >> 0 & 0x1FF;
					*(destination + 1) = payload >> 9 & 0x1FF;
					*(destination + 2) = payload >> 18 & 0x1FF;
					destination += 3;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 28) & 0x03), (int)transition_table[selector].new_selector[(payload >> 28) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 28) & 0x03];
					source++;
					payload = *source;
					break;
				case 8:
					*(destination + 0) = payload >> 0 & 0x3FF;
					*(destination + 1) = payload >> 10 & 0x3FF;
					*(destination + 2) = payload >> 20 & 0x3FF;
					destination += 3;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 9:
					*(destination + 0) = payload >> 0 & 0x3FFF;
					*(destination + 1) = payload >> 14 & 0x3FFF;
					destination += 2;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 28) & 0x03), (int)transition_table[selector].new_selector[(payload >> 28) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 28) & 0x03];
					source++;
					payload = *source;
					break;
				case 10:
					*(destination + 0) = payload >> 0 & 0x7FFF;
					*(destination + 1) = payload >> 15 & 0x7FFF;
					destination += 2;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 11:
					*(destination + 0) = payload >> 0 & 0x0FFFFFFF;
					destination++;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 28) & 0x03), (int)transition_table[selector].new_selector[(payload >> 28) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 28) & 0x03];
					source++;
					payload = *source;
					break;

				/*
					32-bit payload
				*/
				case 12:			// Can't happen!
					// LCOV_EXCL_START
					JASS_assert(false);
					break;
					// LCOV_EXCL_STOP
				case 13:
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
					destination += 16;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 14:
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
					destination += 10;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 15:
					*(destination + 0) = payload >> 0 & 0x0F;
					*(destination + 1) = payload >> 4 & 0x0F;
					*(destination + 2) = payload >> 8 & 0x0F;
					*(destination + 3) = payload >> 12 & 0x0F;
					*(destination + 4) = payload >> 16 & 0x0F;
					*(destination + 5) = payload >> 20 & 0x0F;
					*(destination + 6) = payload >> 24 & 0x0F;
					*(destination + 7) = payload >> 28 & 0x0F;
					destination += 8;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 16:
					*(destination + 0) = payload >> 0 & 0x1F;
					*(destination + 1) = payload >> 5 & 0x1F;
					*(destination + 2) = payload >> 10 & 0x1F;
					*(destination + 3) = payload >> 15 & 0x1F;
					*(destination + 4) = payload >> 20 & 0x1F;
					*(destination + 5) = payload >> 25 & 0x1F;
					destination += 6;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 17:
					*(destination + 0) = payload >> 0 & 0x3F;
					*(destination + 1) = payload >> 6 & 0x3F;
					*(destination + 2) = payload >> 12 & 0x3F;
					*(destination + 3) = payload >> 18 & 0x3F;
					*(destination + 4) = payload >> 24 & 0x3F;
					destination += 5;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 18:
					*(destination + 0) = payload >> 0 & 0x7F;
					*(destination + 1) = payload >> 7 & 0x7F;
					*(destination + 2) = payload >> 14 & 0x7F;
					*(destination + 3) = payload >> 21 & 0x7F;
					destination += 4;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 19:
					*(destination + 0) = payload >> 0 & 0xFF;
					*(destination + 1) = payload >> 8 & 0xFF;
					*(destination + 2) = payload >> 16 & 0xFF;
					*(destination + 3) = payload >> 24 & 0xFF;
					destination += 4;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 20:
					*(destination + 0) = payload >> 0 & 0x3FF;
					*(destination + 1) = payload >> 10 & 0x3FF;
					*(destination + 2) = payload >> 20 & 0x3FF;
					destination += 3;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 21:
					*(destination + 0) = payload >> 0 & 0x7FFF;
					*(destination + 1) = payload >> 15 & 0x7FFF;
					destination += 2;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				case 22:
					*(destination + 0) = payload >> 0 & 0xFFFF;
					*(destination + 1) = payload >> 16 & 0xFFFF;
					destination += 2;

					source++;
#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)(*source & 0x03), (int)transition_table[selector].new_selector[*source & 0x03]);
#endif
					selector = transition_table[selector].new_selector[*source & 0x03];
					payload = *source >> 2;
					break;
				case 23:
					*(destination + 0) = payload >> 0 & 0x0FFFFFFF;
					destination++;

#ifdef CARRY_DEBUG
printf("at %d, Transiton:%d NextSelector:%d\n", (int)selector, (int)((payload >> 30) & 0x03), (int)transition_table[selector].new_selector[(payload >> 30) & 0x03]);
#endif
					selector = transition_table[selector].new_selector[(payload >> 30) & 0x03];
					source++;
					payload = *source;
					break;
				}
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

		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0x0FFFFFFF);
		for (instance = 0; instance < 1; instance++)
			every_case.push_back(0xFFFF);
		for (instance = 0; instance < 3; instance++)
			every_case.push_back(0x3FF);
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

		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0xFF);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x07);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x03);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x0FFFFFFF);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x7FFF);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x3FFF);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x7F);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x3F);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x1F);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x03);
		for (instance = 0; instance < 128; instance++)
			every_case.push_back(0x01);

		for (instance = 0; instance < 10; instance++)					// 28-bits
			every_case.push_back(0x0FFFFFFF);
		for (instance = 0; instance < 5; instance++)					// 5*6-bit
			every_case.push_back(0x3F);
		for (instance = 0; instance < 6; instance++)					// 6*5-bit
			every_case.push_back(0x1F);
		for (instance = 0; instance < 8; instance++)					// 8*4-bit
			every_case.push_back(0x0F);
		for (instance = 0; instance < 6; instance++)					// 6*5-bit
			every_case.push_back(0x1F);
		for (instance = 0; instance < 5; instance++)					// 5*6-bit
			every_case.push_back(0x3F);
		for (instance = 0; instance < 6; instance++)					// 6*5-bit
			every_case.push_back(0x1F);
		for (instance = 0; instance < 7; instance++)					// 7*4-bit
			every_case.push_back(0xF);
		for (instance = 0; instance < 10; instance++)				// 10*3-bit
			every_case.push_back(0x07);
		for (instance = 0; instance < 16; instance++)				// 16*2-bit
			every_case.push_back(0x03);

		compress_integer_carryover_12 *compressor = new compress_integer_carryover_12;
		std::vector<uint32_t>compressed(every_case.size() * 2);
		std::vector<uint32_t>decompressed(every_case.size() + 256);

		auto size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &every_case[0], every_case.size());
		compressor->decode(&decompressed[0], every_case.size(), &compressed[0], size_once_compressed);
		decompressed.resize(every_case.size());
		JASS_assert(decompressed == every_case);

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
		puts("compress_integer_carryover_12::PASSED");
		}
	}
