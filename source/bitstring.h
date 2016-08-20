/*
	BITSTRING.H
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase.
*/
/*!
	@file
	@brief Long bitstrings.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <stdint.h>

#include <vector>

namespace JASS
	{
	/*
		CLASS BITSTRING
		---------------
		Storage class for arbitrary length bitstrings.  The internal storage is in bytes, but they are manipulated
		in chunks of bitstring::bitstring_word (which were intially uint64_t).
	*/
	/*!
		@brief Long bitstrings
	*/
	class bitstring
	{
	/*
		CONSTANTS
		---------
	*/
	protected:
		/*
			BITSTRING::ACTION
			-----------------
		*/
		/*!
			@brief The action enum is used to pass a Boolean operation to operation().
		*/
		enum action
			{
			OR,				///< OR two bitstrings together
			XOR,				///< XOR two bitstrings together
			AND,				///< AND two bitstrings together
			AND_NOT			///< a AND ~ b
			};
		
	/*
		TYPES
		-----
	*/
	protected:
		typedef uint64_t bitstring_word;		// this is the size of word used for Boolean operatons (and so storage is rounded up to units of this)

	/*
		MEMBERS
		-------
	*/
	protected:
		std::vector<uint8_t> bits;				// stograge for the bits
		size_t bits_long;							// the length of the bitstring in bits

	/*
		METHODS
		-------
	*/
	protected:
		/*
			BITSTRING::OPERATION()
			----------------------
		*/
		/*!
			@brief operation
		*/
		virtual void							// no return value
		operation
			(
			action op,							// [in] the operation we're going to perform
			bitstring &a,						// [out] the answer (b op c)
			bitstring &b,						// [in] the first parameter
			bitstring &c						// [in] the second parameter
			);

	public:
		/*
			BITSTRING::BITSTRING()
			----------------------
			Constructor
		*/
		bitstring() : bits_long(0)
			{
			// nothing
			}
		
		/*
			BITSTRING::POPCOUNT()
			---------------------
			Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		*/
		inline static size_t		// [out] number of set bits in the word
		popcount
			(
			uint64_t value			// [in] the value we wish to count the bits within
			)
			{
			value = value - ((value >> 1) & 0x5555555555555555);
			value = (value & 0x3333333333333333) + ((value >> 2) & 0x3333333333333333);
			return (((value + (value >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
			}

		/*
			BITSTRING::POPCOUNT()
			---------------------
			Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		*/
		inline static size_t		// [out] number of set bits in the word
		popcount
			(
			uint32_t value			// [in] the value we wish to count the bits within
			)
			{
			value = value - ((value >> 1) & 0x55555555);
			value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
			return (((value + (value >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
			}

		/*
			BITSTRING::POPCOUNT()
			---------------------
			Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		*/
		inline static size_t		// [out] number of set bits in the word
		popcount
			(
			uint16_t value			// [in] the value we wish to count the bits within
			)
			{
			value = value - ((value >> 1) & 0x5555);
			value = (value & 0x3333) + ((value >> 2) & 0x3333);
			return (((value + (value >> 4)) & 0xF0F) * 0x101) >> 8;
			}

		/*
			BITSTRING::POPCOUNT()
			---------------------
			Count the number of bits set in parallel, see here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		*/
		inline static size_t		// [out] number of set bits in the word
		popcount
			(
			uint8_t value			// [in] the value we wish to count the bits within
			)
			{
			value = value - ((value >> 1) & 0x55);
			value = (value & 0x3333) + ((value >> 2) & 0x33);
			return (((value + (value >> 4)) & 0xF) * 0x1);
			}

		/*
			BITSTRING::SET_LENGTH()
			-----------------------
			Set the length of the bitstring to length_in_bits.  The new valid range is 0.. length_in_bits - 1
		*/
		void								// no return value
		set_length
			(
			size_t length_in_bits	// [in] the new length of bitstring
			);

		/*
			BITSTRING::SIZE()
			-----------------
		*/
		size_t			// [out] return the length of the bitstring measured in bits
		size
			(
			void			// no paramters
			) const
			{
			return bits_long;
			}

		/*
			BITSTRING::UNSAFE_SETBIT()
			--------------------------
			Set the bit a position to 1.
		*/
		inline void								// no return value
		unsafe_setbit
			(
			size_t position					// [in] which bit to set to 1
			)
			{
			bits[position >> 3] |= 1 << (position & 7);
			}
		
		/*
			BITSTRING::UNSAFE_UNSETBIT()
			----------------------------
			Set the bit a position to 0.
		*/
		inline void								// no return value
		unsafe_unsetbit
			(
			size_t position					// [in] which bit to set to 0
			)
			{
			bits[position >> 3] &= ~(1 << (position & 7));
			}
		
		/*
			BITSTRING::UNSAFE_GETBIT()
			--------------------------
			Return the value of the bit at position (either 0 or 1)
		*/
		inline long					// [out] 0 or 1, the value of the bit at the given position
		unsafe_getbit
			(
			size_t position		// [in] The bit position
			) const
			{
			return (bits[position >> 3] >> (position & 7)) & 0x01;
			}

		/*
			BITSTRING::BIT_OR()
			-------------------
			answer = this OR with. This can may the length of either the this or with bitstrings.
		*/
		inline void								// no return value
		bit_or
			(
			bitstring &answer,		// [out] the answer or this OR with
			bitstring &with			// [in] the bitstring to or this with
			)
			{
			operation(OR, answer, *this, with);
			}
		
		/*
			BITSTRING::BIT_XOR()
			--------------------
			answer = this XOR with. This can may the length of either the this or with bitstrings.
		*/
		inline void								// no return value
		bit_xor
			(
			bitstring &answer,		// [out] the answer or this XOR with
			bitstring &with			// [in] the bitstring to or this with
			)
			{
			operation(XOR, answer, *this, with);
			}
		
		
		/*
			BITSTRING::BIT_AND()
			--------------------
			answer = this AND with. This can may the length of either the this or with bitstrings.
		*/
		inline void								// no return value
		bit_and
			(
			bitstring &answer,		// [out] the answer or this AND with
			bitstring &with			// [in] the bitstring to or this with
			)
			{
			operation(AND, answer, *this, with);
			}
		

		/*
			BITSTRING::BIT_AND_NOT()
			------------------------
			answer = this AND NOT with. This can may the length of either the this or with bitstrings.
		*/
		inline void								// no return value
		bit_and_not
			(
			bitstring &answer,		// [out] the answer or this AND NOT with
			bitstring &with			// [in] the bitstring to or this with
			)
			{
			operation(AND_NOT, answer, *this, with);
			}

		/*
			BITSTRING::POPCOUNT()
			---------------------
			Return the number of set bits in the bitstring
		*/
		size_t								// [out] returns the number of set bits in the bitstring
		popcount
			(
			void								// no parameters
			) const;

		/*
			BITSTRING::INDEX()
			------------------
			Return the bit position of the nth set bit (where n is called which)
		*/
		size_t						// [out] the bit positon of the given set bit
		index
			(
			size_t which			// [in] which set bit we're looking for
			);

		/*
			BITSTRING::ZERO()
			-----------------
			Set the bitstring to all zeros
		*/
		void									// no return value
		zero
			(
			void								// no parameters
			);

		/*
			BITSTRING::ONE()
			----------------
			Set all valid bits to 1

		*/
		void									// no return value
		one
			(
			void								// no parameters
			);
		
		/*
			BITSTRING::UNITTEST()
			---------------------
		*/
		static void							// no return value
		unittest
			(
			void								// no parameters
			);
	} ;
}