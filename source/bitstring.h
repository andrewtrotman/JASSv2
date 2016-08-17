/*
	BITSTRING.H
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase.
*/
#pragma once

#include <stdint.h>

namespace JASS
	{
	/*
		Routines to count bits in linear types (e.g. long long int).
	*/
	/*
		class BITSTRING
		---------------
	*/
	class bitstring
	{
	protected:
		enum action { OR, XOR, AND, AND_NOT };

	protected:
		uint8_t *bits;
		size_t bits_long;
		size_t bytes_long;
		size_t chunks_long;

	protected:
		virtual void operation(action op, bitstring &a, bitstring &b, bitstring &c);		// a = b op c

	public:
		bitstring();
		virtual ~bitstring();
		
	/*
		POPCOUNT()
		----------
		Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	*/
	inline size_t popcount(uint64_t i)
		{
		i = i - ((i >> 1) & 0x5555555555555555);                                                                                                  
		i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);                                                                           
		return (((i + (i >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;                                                                  
		}

	/*
		POPCOUNT()
		----------
		Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	*/
	inline size_t popcount(uint32_t i)
		{
		i = i - ((i >> 1) & 0x55555555);
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
		return (((i + (i >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
		}

	/*
		POPCOUNT()
		----------
		Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	*/
	inline size_t popcount(uint16_t i)
		{
		i = i - ((i >> 1) & 0x5555);
		i = (i & 0x3333) + ((i >> 2) & 0x3333);
		return (((i + (i >> 4)) & 0xF0F) * 0x101) >> 8;
		}

	/*
		POPCOUNT()
		----------
		Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	*/
	inline size_t popcount(uint8_t i)
		{
		i = i - ((i >> 1) & 0x55);
		i = (i & 0x3333) + ((i >> 2) & 0x33);
		return (((i + (i >> 4)) & 0xF) * 0x1);
		}

		size_t popcount(uint8_t *ch, size_t bytes_long);


		void set_length(size_t len_in_bits);
		void unsafe_set_length(size_t len_in_bits);

		size_t get_length(void) const					{ return bits_long; }

		inline void unsafe_setbit(size_t pos) 		{ bits[pos >> 3] |= 1 << (pos & 7); }
		inline void unsafe_unsetbit(size_t pos)	{ bits[pos >> 3] &= ~(1 << (pos & 7)); }
		inline long unsafe_getbit(size_t pos)		{ return (bits[pos >> 3] >> (pos & 7)) & 0x01; }

		void bit_or(bitstring &ans, bitstring &with) 		{ operation(OR, ans, *this, with); }
		void bit_xor(bitstring &ans, bitstring &with) 		{ operation(XOR, ans, *this, with); }
		void bit_and(bitstring &ans, bitstring &with) 		{ operation(AND, ans, *this, with); }
		void bit_and_not(bitstring &ans, bitstring &with) 	{ operation(AND_NOT, ans, *this, with); }

		size_t count_bits(void) const { return count_bits_in(bits, bytes_long); }
		size_t index(size_t which);
		void zero(void);
		void one(void);
	} ;
}