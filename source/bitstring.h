/*
	BITSTRING.H
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
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
	*/
	/*!
		@brief Long bitstrings
		@details The internal storage is in bytes, but they are manipulated
		in chunks of bitstring::bitstring_word (which were intially uint64_t).  The memory used to store the bits
		is guaranteed to be contigious. The set, unset, and get methods are impemented as function calls because
		an overloaded operator[] would need to return something that can be assigned to - which would be an inter
		mediate object that needs to know which bit is being referred to, and that would result in an object creation
		which is undesirable.
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
			/*!
				@typedef bitstring_word
				@brief bitstring_word is used as the underlying type for Boolean operations and for popcount()
			*/
			typedef uint64_t bitstring_word;		// this is the size of word used for Boolean operatons (and so storage is rounded up to units of this)

		/*
			MEMBERS
			-------
		*/
		protected:
			/*!
				@var bits
			*/
			std::vector<uint8_t> bits;				///< The underlying storage for the long bitstring
			
			/*!
				@var bits_long
			*/
			size_t bits_long;							///< The length of the bitstring in bits.

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
				@param op [in] the operation we're going to perform.
				@param a [out] the answer (b op c)
				@param b [in] the first parameter
				@param c in] the second parameter
			*/
			virtual void operation(action op, bitstring &a, bitstring &b, bitstring &c);

		public:
			/*
				BITSTRING::BITSTRING()
				----------------------
			*/
			/*!
				@brief Constructor
			*/
			bitstring() : bits_long(0)
				{
				// nothing
				}
			/*
				BITSTRING::~BITSTRING()
				----------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~bitstring()
				{
				// nothing
				}
			
			/*
				BITSTRING::POPCOUNT()
				---------------------
			*/
			/*!
				@brief Count the number of bits set in value.  This uses the parallel algorithm from here: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
				@param value [in] The value whose set bits are counted.
				@return The number of set bits in the parameter.
			*/
			inline static size_t popcount(uint64_t value)
				{
				value = value - ((value >> 1) & 0x5555555555555555);
				value = (value & 0x3333333333333333) + ((value >> 2) & 0x3333333333333333);
				return (((value + (value >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
				}

			/*
				BITSTRING::POPCOUNT()
				---------------------
			*/
			/*!
				@overload
			*/
			inline static size_t popcount(uint32_t value)
				{
				value = value - ((value >> 1) & 0x55555555);
				value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
				return (((value + (value >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
				}

			/*
				BITSTRING::POPCOUNT()
				---------------------
			*/
			/*!
				@overload
			*/
			inline static size_t popcount(uint16_t value)
				{
				value = value - ((value >> 1) & 0x5555);
				value = (value & 0x3333) + ((value >> 2) & 0x3333);
				/*
					Is this a compiler bug?  we can't do a single line return and must instead do a two line return on Xcode 8.3.3
				*/
//				return (((value + (value >> 4)) & 0xF0F) * 0x101) >> 8;
				value = (((value + (value >> 4)) & 0xF0F) * 0x101);
				return value >> 8;
				}

			/*
				BITSTRING::POPCOUNT()
				---------------------
			*/
			/*!
				@overload
			*/
			inline static size_t popcount(uint8_t value)
				{
				value = value - ((value >> 1) & 0x55u);
				value = (value & 0x33u) + ((value >> 2) & 0x33u);
				return (((value + (value >> 4)) & 0xFu) * 0x1);
				}

			/*
				BITSTRING::RESIZE()
				-------------------
			*/
			/*!
				@brief Set the length of the bitstring to length_in_bits.  The new valid range is 0.. length_in_bits - 1
				@param length_in_bits [in] the new length of bitstring
			*/
			void resize(size_t length_in_bits);

			/*
				BITSTRING::SIZE()
				-----------------
			*/
			/*!
				@brief Return the length (in bits) of the bitstring
				@return The length of the bitstring measured in bits
			*/
			size_t size(void) const
				{
				return bits_long;
				}

			/*
				BITSTRING::UNSAFE_SETBIT()
				--------------------------
			*/
			/*!
				@brief Set the bit at position to 1.
				@details This method does not check for overflow and may result in unpredictable behaviour if a position outside the range [0..size()-1] is passed.
				@param position in] Which bit to set to 1.
			*/
			inline void unsafe_setbit(size_t position)
				{
				bits[position >> 3] |= 1 << (position & 7);
				}
			
			/*
				BITSTRING::UNSAFE_UNSETBIT()
				----------------------------
			*/
			/*!
				@brief Set the bit at position to 0.
				@details This method does not check for overflow and may result in unpredictable behaviour if a position outside the range [0..size()-1] is passed.
				@param position in] Which bit to set to 0.
			*/
			inline void unsafe_unsetbit(size_t position)
				{
				bits[position >> 3] &= ~(1 << (position & 7));
				}
			
			/*
				BITSTRING::UNSAFE_GETBIT()
				--------------------------
				Return the value of the bit at position (either 0 or 1)
			*/
			/*!
				@brief Return the state (0 or 1) of the bit at the given position.
				@details This method does not check for overflow and may result in unpredictable behaviour if a position outside the range [0..size()-1] is passed.
				@param position in] Which bit to check.
				@return true or false, the value of the bit at the given position
			*/
			bool unsafe_getbit(size_t position) const
				{
				return (bits[position >> 3] >> (position & 7)) & 0x01;
				}


			/*
				BITSTRING::SETBIT()
				-------------------
			*/
			/*!
				@brief Set the bit at position to 1.
				@details This method checks for overflow and has no effect if overflow is detected.
				@param position in] Which bit to set to 1.
			*/
			inline void setbit(size_t position)
				{
				if (position < bits_long)
					unsafe_setbit(position);
				}
			
			/*
				BITSTRING::UNSETBIT()
				---------------------
			*/
			/*!
				@brief Set the bit at position to 0.
				@details This method checks for overflow and has no effect if overflow is detected.
				@param position in] Which bit to set to 0.
			*/
			inline void unsetbit(size_t position)
				{
				if (position < bits_long)
					unsafe_unsetbit(position);
				}
			
			/*
				BITSTRING::GETBIT()
				-------------------
				Return the value of the bit at position (either 0 or 1)
			*/
			/*!
				@brief Return the state (0 or 1) of the bit at the given position.
				@details This method checks for overflow and returns false in the case of overflow.
				@param position in] Which bit to check.
				@return true or false, the value of the bit at the given position (or false is position is out of range)
			*/
			bool getbit(size_t position) const
				{
				return position < bits_long ? unsafe_getbit(position) : 0;
				}

			/*
				BITSTRING::BIT_OR()
				-------------------
			*/
			/*!
				@brief answer = this | with
				@details OR this and with, returning the result in answer.  This method is used to avoid object copies and allocation that would come from using operator overloading in C++.
				With is [in, out] because this method sets the length of all three bitstrings to the length of the longest.
				@param answer [out] The answer or this OR with
				@param with [in, out] the bitstring to OR this with
			*/
			inline void bit_or(bitstring &answer, bitstring &with)
				{
				operation(OR, answer, *this, with);
				}
			
			/*
				BITSTRING::BIT_XOR()
				--------------------
			*/
			/*!
				@brief answer = this ^ with
				@details OR this and with, returning the result in answer.  This method is used to avoid object copies and allocation that would come from using operator overloading in C++.
				With is [in, out] because this method sets the length of all three bitstrings to the length of the longest.
				@param answer [out] The answer or this XOR with
				@param with [in, out] the bitstring to XOR this with
			*/
			inline void bit_xor(bitstring &answer, bitstring &with)
				{
				operation(XOR, answer, *this, with);
				}
			
			
			/*
				BITSTRING::BIT_AND()
				--------------------
			*/
			/*!
				@brief answer = this & with
				@details OR this and with, returning the result in answer.  This method is used to avoid object copies and allocation that would come from using operator overloading in C++.
				With is [in, out] because this method sets the length of all three bitstrings to the length of the longest.
				@param answer [out] The answer or this AND with
				@param with [in, out] the bitstring to AND this with
			*/
			inline void bit_and(bitstring &answer, bitstring &with)
				{
				operation(AND, answer, *this, with);
				}
			

			/*
				BITSTRING::BIT_AND_NOT()
				------------------------
			*/
			/*!
				@brief answer = this & ~with
				@details OR this and with, returning the result in answer.  This method is used to avoid object copies and allocation that would come from using operator overloading in C++.
				With is [in, out] because this method sets the length of all three bitstrings to the length of the longest.
				@param answer [out] The answer or this AND NOT with
				@param with [in, out] the bitstring to AND NOT this with
			*/
			inline void bit_and_not(bitstring &answer, bitstring &with)
				{
				operation(AND_NOT, answer, *this, with);
				}

			/*
				BITSTRING::POPCOUNT()
				---------------------
			*/
			
			/*!
				@brief Return the number of set bits in the bitstring.  This uses popcount(bitstring_word) to do the count
				@return The number of set bits in this bitstring.
			*/
			size_t popcount(void) const;

			/*
				BITSTRING::INDEX()
				------------------
			*/
			/*!
				@brief Returns the bit position of the which-th set bit.
				@param [in] which set bit we're looking for.
				@return The bit positon of the given set bit or std::numeric_limits<size_t>::max() if which is greater than the number of set bits.
			*/
			size_t index(size_t which);

			/*
				BITSTRING::ZERO()
				-----------------
			*/
			/*!
				@brief Set the bitstring to all zeros
			*/
			void zero(void);

			/*
				BITSTRING::ONE()
				----------------
			*/
			/*!
				@brief Set the bitstring to all ones
			*/
			void one(void);
			
			/*
				BITSTRING::SERIALISE()
				----------------------
			*/
			/*!
				@brief Return a serialised version of this object as an array of uint8_t.
				@details  One frequent (and the initial use) of this class is to convert a bit-string into a byte-string.
				This method does exactly that.  It returns a pointer to the internal byte-string and a length.  The returned
				string is contigious.  It is valid until the length of the bitstring changes.
				@param length [out] The length of the bytestring (measured in bytes).
				@return The bitstring as a byte string (passed with 0s at the end).
			*/
			uint8_t *serialise(size_t &length)
				{
				length = bits.size();
				return &bits[0];
				}
			
			/*
				BITSTRING::UNITTEST()
				---------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
}
