/*
	BITSTRING.C
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
#include <limits>

#include <string.h>
#include <stdlib.h>

#include "maths.h"
#include "assert.h"
#include "bitstring.h"

namespace JASS
	{
	/*
		BITSTRING::POPCOUNT()
		---------------------
		Return the number of set bits in the bitstring
	*/
	size_t bitstring::popcount(void) const
		{
		size_t count = 0;
		size_t chunks_long = bits.size() / sizeof(bitstring_word);
		bitstring_word *data = (bitstring_word *)&bits[0];
	
		for (size_t chunk = 0; chunk < chunks_long; chunk++)
			count += popcount(*data++);
			
		return count;
		}

	/*
		BITSTRING::RESIZE()
		-------------------
		Sets the length of the bitstring. Valid bits are in the range (0 .. new_length_in_bits-1)
	*/
	void bitstring::resize(size_t new_length_in_bits)
		{
		/*
			Compute the new and old lengths in "chunks"
		*/
		size_t chunks_long = (new_length_in_bits + 8 * sizeof(bitstring_word)) / (8 * sizeof(bitstring_word));
		size_t old_chunks_long = bits.size() / sizeof(bitstring_word);
		
		/*
			resize the vector, but only if we need to
		*/
		if (chunks_long > old_chunks_long)
			{
			size_t bytes_long = chunks_long * sizeof(chunks_long);
			bits.resize(bytes_long);
			}
			
		/*
			remember out length
		*/
		bits_long = new_length_in_bits;
		}

	/*
		BITSTRING::ZERO()
		-----------------
		Set the bitstring to all zeros
	*/
	void bitstring::zero(void)
		{
		memset(&bits[0], 0, bits.size());
		}

	/*
		ANT_BITSTRING::ONE()
		--------------------
		Set all valid bits to 1 (and pad to the end with zeros)
	*/
	void bitstring::one(void)
		{
		/*
			set the entire bitstring to all 1s
		*/
		memset(&bits[0], 0xFF, bits.size());

		/*
			turn the over-shoot bits back to 0
		*/
		size_t last_bit = bits.size() * 8;
		for (size_t clearing = bits_long; clearing < last_bit; clearing++)
			unsafe_unsetbit(clearing);
		}

	/*
		BITSTRING::OPERATION()
		----------------------
		Worker function to perform a binary operation a = b op c (e.g. a = b AND c)
		Its done this way to avoid repitition of the setup code.
	*/
	void bitstring::operation(action op, bitstring &a, bitstring &b, bitstring &c)
		{
		/*
			Make sure all the bitstrings are the same length
		*/
		size_t longest = maths::maximum(b.size(), c.size());
		a.resize(longest);
		b.resize(longest);
		c.resize(longest);

		/*
			get pointers to each of the internal buffers (as bitstring_word pointers)
		*/
		bitstring_word *aa = (bitstring_word *)&a.bits[0];
		bitstring_word *bb = (bitstring_word *)&b.bits[0];
		bitstring_word *cc = (bitstring_word *)&c.bits[0];
		
		/*
			work out when to stop
		*/
		bitstring_word *end = aa + (bits.size() / sizeof(bitstring_word));

		/*
			swith on op and run until we're done
		*/
		switch (op)
			{
			case OR:
				while (aa < end)
					*aa++ = *bb++ | *cc++;
				break;
			case AND:
				while (aa < end)
					*aa++ = *bb++ & *cc++;
				break;
			case XOR:
				while (aa < end)
					*aa++ = *bb++ ^ *cc++;
				break;
			case AND_NOT:
				while (aa < end)
					*aa++ = *bb++ & ~*cc++;
				break;
			}
		}
	

	/*
		BITSTRING::INDEX()
		------------------
		Return the bit position of the nth set bit (where n is called which)
	*/
	size_t bitstring::index(size_t which)
		{
		size_t total = 0;
		uint8_t *ch = &bits[0];
		size_t bytes_long = bits.size();
		for (size_t here = 0; here < bytes_long; here++)
			{
			size_t old_total = total;
			total += popcount(*ch);
			if (total >= which)
				{
				size_t my_bit = which - old_total;
				for (size_t bit = 0; bit < 8; bit++)
					if (*ch &  1 << bit)
						{
						my_bit--;
						if (my_bit == 0)
							return here * 8 + bit;
						}
				}
			ch++;
			}

		return (std::numeric_limits<size_t>::max)();		// no such bit set.
		}
	
	/*
		BITSTRING::UNITTEST()
		---------------------
	*/
	void bitstring::unittest(void)
		{
		bitstring b1;
		bitstring b2;
		bitstring b3;

		/*
			check that a bitstring is initialised to zero
		*/
		b1.resize(100);
		JASS_assert(b1.popcount() == 0);

		b2.resize(1000);
		JASS_assert(b2.popcount() == 0);
		
		/*
			check that setting a bit only sets one bit
		*/
		b1.unsafe_setbit(99);
		JASS_assert(b1.unsafe_getbit(99));
		
		b2.unsafe_setbit(999);
		JASS_assert(b2.unsafe_getbit(999));
		
		/*
			Check that OR words
		*/
		b1.bit_or(b3, b2);
		JASS_assert(b3.unsafe_getbit(99));
		JASS_assert(b3.unsafe_getbit(999));
		JASS_assert(b3.popcount() == 2);
		
		/*
			make sure OR didn't trash the original strings
		*/
		JASS_assert(b1.popcount() == 1);
		JASS_assert(b2.popcount() == 1);

		/*
			does index() work?
		*/
		JASS_assert(b3.index(0) == (std::numeric_limits<size_t>::max)());
		JASS_assert(b3.index(1) == 99);
		JASS_assert(b3.index(2) == 999);
		JASS_assert(b3.index(1024) == (std::numeric_limits<size_t>::max)());

		/*
			Check that XOR words
		*/
		b1.bit_xor(b3, b2);
		JASS_assert(b3.unsafe_getbit(99));
		JASS_assert(b3.unsafe_getbit(999));
		JASS_assert(b3.popcount() == 2);
		
		/*
			make sure XOR didn't trash the original strings
		*/
		JASS_assert(b1.popcount() == 1);
		JASS_assert(b2.popcount() == 1);

		/*
			Check that AND words
		*/
		b2.unsafe_setbit(99);
		b1.bit_and(b3, b2);
		JASS_assert(b3.unsafe_getbit(99) == 1);
		JASS_assert(b3.unsafe_getbit(999) == 0);
		JASS_assert(b3.popcount() == 1);
		
		/*
			make sure AND didn't trash the original strings
		*/
		JASS_assert(b1.popcount() == 1);
		JASS_assert(b2.popcount() == 2);

		/*
			Check that AND_NOT words
		*/
		b2.unsafe_unsetbit(99);
		b1.bit_and_not(b3, b2);
		JASS_assert(b3.unsafe_getbit(99) == 1);
		JASS_assert(b3.unsafe_getbit(999) == 0);
		JASS_assert(b3.popcount() == 1);
		
		/*
			make sure AND_NOT didn't trash the original strings
		*/
		JASS_assert(b1.popcount() == 1);
		JASS_assert(b2.popcount() == 1);

		/*
			zero() should work
		*/
		b2.zero();
		JASS_assert(b2.popcount() == 0);
		
		/*
			one() is a bit more complex, the check is that it doesn't overshoot
		*/
		b1.one();
		JASS_assert(b1.popcount() == b1.size());

		/*
			Check the popcount() methods that aren't already checked
		*/
		JASS_assert(bitstring::popcount(static_cast<uint64_t>(0xF103050701030507)) == 20);
		JASS_assert(bitstring::popcount(static_cast<uint32_t>(0xF1030507)) == 12);
		JASS_assert(bitstring::popcount(static_cast<uint16_t>(0xF103)) == 7);
		JASS_assert(bitstring::popcount(static_cast<uint8_t>(0xF7)) == 7);

		/*
			Check the safe methods
		*/
		bitstring b4;
		b4.resize(100);
		JASS_assert(b4.popcount() == 0);
		b4.setbit(22);
		JASS_assert(b4.popcount() == 1);
		JASS_assert(b4.getbit(22) == 1);
		b4.unsetbit(22);
		JASS_assert(b4.popcount() == 0);

		/*
			Yay, we passed
		*/
		puts("bitstring::PASSED");
		}
	}
