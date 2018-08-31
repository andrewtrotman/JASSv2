/*
	BITSTREAM.H
	-----------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
/*!
	@file
	@brief Long bitstreams (push and pull from a long bitstring).
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

namespace JASS
	{
	/*
		CLASS BITSTREAM
		---------------
	*/
	/*!
		@brief Long bitstreams (push and pull from a long bitstring).
	*/
	class bitstream
		{
		protected:
			uint64_t total_bits;			///< the length of the bitstring (in bits)

			uint32_t *stream;				///< the bitstream
			size_t stream_length;		///< the length of the bitstream (in 32-bit words)
			size_t stream_pos;			///< current word (in stream) of the most recently written word

			size_t bit_pos;				///< the current bit position (in buffer) of the head of the stream
			uint32_t buffer;				///< the current head of the buffer

			bool failed;				///< The bitstream is invalid because it has overflown its allocated length

		protected:
			/*
				BITSTREAM::PUSH_BUFFER()
				------------------------
			*/
			/*!
				@brief push the current word onto the end of the stream
			*/
			inline void push_buffer(void)
				{
				if (stream_pos >= stream_length)
					{
					failed = true;
					return;
					}
				stream[stream_pos] = buffer;
				stream_pos++;
				buffer = 0;
				bit_pos = 0;
				}

		public:
			/*
				BITSTREAM::BITSTREAM()
				----------------------
			*/
			/*!
				@brief Constructor
			*/
			bitstream()
				{
				rewind();
				}

			/*
				BITSTREAM::PUSH_ZERO()
				----------------------
			*/
			/*!
				@brief push a 0 into the stream
			*/
			inline void push_zero(void)
				{
				if (++bit_pos >= 32)
					push_buffer();

				total_bits++;
				}

			/*
				BITSTREAM::PUSH_ZEROS()
				-----------------------
			*/
			/*!
				@brief push length number of 0s into the stream
				@param length [in] the number of 0s to push
			*/
			inline void push_zeros(size_t length)
				{
				while (length-- > 0)
					push_zero();
				}

			/*
				BITSTREAM::PUSH_ONE()
				---------------------
			*/
			/*!
				@brief push a 1 into the stream
			*/
			inline void push_one(void)
				{
				buffer |= ((unsigned long)1) << bit_pos;

				if (++bit_pos >= 32)
					push_buffer();

				total_bits++;
				}

			/*
				BITSTREAM::PUSH_BITS()
				----------------------
			*/
			/*!
				@brief push up-to 64 bits into the steam
				@param bits [in] the bits to push (stored from high to low, so 1011 pushes 1, then 0, then 1, then 1).
				@param length [in] the number of bits to push
			*/
			inline void push_bits(uint64_t bits, size_t length)
				{
				while (length-- > 0)
					if (bits & ((unsigned long long)1 << length))
						push_one();
					else
						push_zero();
				}

			/*
				BITSTREAM::REWIND()
				-------------------
			*/
			/*!
				@brief Rewind the bistream over the given memory
				@param destination [in] The buffer in which the bitstring is constructed
				@param destination_length [in] The length of destination (in bytes)
			*/
			void rewind(void *destination = 0, size_t destination_length = 0)
				{
				stream = (uint32_t *)destination;
				stream_length = destination_length / sizeof(uint32_t);		// convert into chunk sizes

				failed = false;
				stream_pos = 0;
				bit_pos = 0;
				buffer = 0;
				total_bits = 0;
				}

			/*
				BITSTREAM::EOF()
				----------------
			*/
			/*!
				@brief Flush unflushed data to the stram and retun the stream length (or 0 on failure) in bytes
				@return The length of the stream (in bytes).
			*/
			uint64_t eof(void)
				{
				push_buffer();
				return failed ? 0 : (total_bits + 7) / 8;			// return the number of bytes used or 0 on error
				}

			/*
				BITSTREAM::GET_BIT()
				--------------------
			*/
			/*!
				@brief return the next bit in the stream
				@return true or false
			*/
			inline int get_bit(void)
				{
				bool ans;

				ans = (stream[bit_pos >> 5] >> (bit_pos & 31)) & 0x01;
				bit_pos++;

				return ans;
				}

			/*
				BITSTREAM::GET_BITS()
				---------------------
			*/
			/*!
				@brief get up-to 64 bits from the stream.
				@return The next bits in the bitstream.
			*/
			inline uint64_t get_bits(size_t bits)													// get up-to 64 bits
				{
				uint64_t ans = 0;

				while (bits-- > 0)
					ans = (ans << 1) | get_bit();

				return ans;
				}

				/*
					BITSTREAM::UNITTEST()
					---------------------
				*/
				/*!
					@brief Unit test this class
				*/
				static void unittest(void)
					{
					puts("bitstream::*** NOT TESTED ***");
					}
			};
	}
