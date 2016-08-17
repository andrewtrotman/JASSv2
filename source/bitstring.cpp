/*
	BITSTRING.C
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase.
*/
#include <string.h>
#include <stdlib.h>
#include "bitstring.h"

namespace JASS
	{
	#define BITS_PER_WORD 64		// 64 bits to a word
	typedef uint64_t bitstring_word;

	template <typename TYPE>
	TYPE max(TYPE a, TYPE b)
	{
	return a <= b ? a : b;
	}

	/*
		BITSTRING::BITSTRING()
		----------------------
	*/
	bitstring::bitstring()
		{
		bits = NULL;
		bits_long = bytes_long = chunks_long = 0;
		}

	/*
		BITSTRING::~BITSTRING()
		-----------------------
	*/
	bitstring::~bitstring()
		{
		free(bits);
		}


	/*
		BITSTRING::POPCOUNT()
		---------------------
		Count the numner of bits set in the given buffer
	*/
	size_t bitstring::popcount(uint8_t *ch, size_t bytes_long)
		{
		size_t total, here;

		total = 0;
		for (here = 0; here < bytes_long; here++)
			total += popcount(*ch++);

		return total;
		}

	/*
		BITSTRING::SET_LENGTH()
		-----------------------
	*/
	void bitstring::set_length(size_t len_in_bits)
	{
	size_t old_bytes_long, old_chunks_long;

	old_bytes_long = bytes_long;
	old_chunks_long = chunks_long;

	unsafe_set_length(len_in_bits);

	if (chunks_long != old_chunks_long)
		{
		bits = (uint8_t *)realloc(bits, bytes_long);
		if (bytes_long > old_bytes_long)
			memset(bits + old_bytes_long, 0, (size_t)(bytes_long - old_bytes_long));
		}
	}

	/*
		BITSTRING::UNSAFE_SET_LENGTH()
		------------------------------
		This resets the lengh of the bitstring but not the length of the underlying
		buffer that holds the bits.  Its only useful if you're going to resize within
		the bounds that the object was first created with.  It also doesn't zero the
		new stuff (because there is no new stuff)
	*/
	void bitstring::unsafe_set_length(size_t len_in_bits)
	{
	size_t new_chunks_long;

	new_chunks_long = (len_in_bits - 1) / BITS_PER_WORD + 1;
	if (new_chunks_long != chunks_long)
		{
		chunks_long = new_chunks_long;
		bytes_long = chunks_long * (BITS_PER_WORD / 8);
		}

	bits_long = len_in_bits;
	}

	/*
		BITSTRING::ZERO()
		-----------------
	*/
	void bitstring::zero(void)
	{
	memset(bits, 0, (size_t)bytes_long);
	}

	/*
		ANT_BITSTRING::ONE()
		--------------------
		Set all bits to 1 (and pad end with zeros)
	*/
	void bitstring::one(void)
	{
	size_t clearing, last_bit;
	memset(bits, 0xFF, (size_t)bytes_long);

	last_bit = bytes_long * 8;
	for (clearing = bits_long; clearing < last_bit; clearing++)
		unsafe_unsetbit(clearing);
	}

	/*
		BITSTRING::OPERATION()
		----------------------
	*/
	void bitstring::operation(action op, bitstring &a, bitstring &b, bitstring &c)
	{
	bitstring_word *aa, *bb, *cc;
	size_t longest, here;

	longest = max(b.get_length(), c.get_length());
	a.set_length(longest);
	b.set_length(longest);
	c.set_length(longest);

	aa = (bitstring_word *)&a.bits;
	bb = (bitstring_word *)&b.bits;
	cc = (bitstring_word *)&c.bits;

	switch (op)
		{
		case OR:
			for (here = 0; here < chunks_long; here++)
				*aa++ = *bb++ | *cc++;
			break;
		case AND:
			for (here = 0; here < chunks_long; here++)
				*aa++ = *bb++ & *cc++;
			break;
		case XOR:
			for (here = 0; here < chunks_long; here++)
				*aa++ = *bb++ ^ *cc++;
			break;
		case AND_NOT:
			for (here = 0; here < chunks_long; here++)
				*aa++ = *bb++ & ~*cc++;
			break;
		}
	}

	/*
		BITSTRING::INDEX()
		------------------
	*/
	size_t bitstring::index(size_t which)
	{
	size_t old_total, total, here, my_bit, bit;
	uint8_t *ch;

	total = 0;
	ch = bits;
	for (here = 0; here < bytes_long; here++)
		{
		old_total = total;
		total += popcount(*ch);
		if (total >= which)
			{
			my_bit = which - old_total;
			for (bit = 0; bit < 8; bit++)
				if (*ch &  1 << bit)
					{
					my_bit--;
					if (my_bit == 0)
						return here * 8 + bit;
					}
			}
		ch++;
		}

	return -1;		// no such bit set.
	}
}