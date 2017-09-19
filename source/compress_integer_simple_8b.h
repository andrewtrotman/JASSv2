/*
	COMPRESS_INTEGER_SIMPLE_8B.H
	----------------------------
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_SIMPLE_8B
		--------------------------------
	*/
	class compress_integer_simple_8b : public compress_integer
		{
		protected:
			static const size_t ints_packed_table[];			///< Number of integers packed into a 64-bit word, given its mask type
			static const size_t can_pack_table[];				///< Bitmask map for valid masks at an offset (column) for some num_bits_needed (row)
			static const size_t row_for_bits_needed[];			///< Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
			static const size_t invalid_masks_for_offset[];		///< We AND out masks for offsets where we don't know if we can fully pack for that offset
			static const size_t simple8b_shift_table[];			///< Number of bits to shift across when packing - is sum of prior packed ints

		public:
			/*
				COMPRESS_INTEGER_SIMPLE_8B::COMPRESS_INTEGER_SIMPLE_8B()
				--------------------------------------------------------
			*/
			/*!
				@brief Consructor
			*/
			compress_integer_simple_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_8B::~COMPRESS_INTEGER_SIMPLE_8B()
				--------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_simple_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_8B::ENCODE()
				-----------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);

			/*
				COMPRESS_INTEGER_SIMPLE_8B::DECODE()
				-----------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);

			/*
				COMPRESS_INTEGER_SIMPLE_8B::UNITTEST()
				-------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
