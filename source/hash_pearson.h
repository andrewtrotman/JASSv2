/*
	HASH_PEARSON.H
	--------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	see: Pearson, Peter K., Fast hashing of variable-length text strings, CACM Volume 33, No. 6 (June 1990), pp. 677-680
*/
/*!
	@file
	@brief Faithful implementation of Person's hashing function
	@details see: Pearson, Peter K., Fast hashing of variable-length text strings, CACM Volume 33, No. 6 (June 1990), pp. 677-680
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "slice.h"

namespace JASS
	{
	/*!
		@brief Person's Random Hash.
		@details  Pearson's hash funcition similates a directed random walk through a space of size 2^8.  It is extended to work in spaces
		larger than that by interleaving different hashes over that same string.  That is, a 16-bit generates two 8-bit hashes on the 0,2,4... 
		1,3,5,... characters which are then combined together for the final hash (a << 8 | b).
		
		The original reference is: Pearson, Peter K., Fast hashing of variable-length text strings, CACM Volume 33, No. 6 (June 1990), pp. 677-680
	*/
	class hash_pearson
		{
		protected:
			static uint8_t random_walk[];				///< Pearson's random walk table.

		public:
			/*
				HASH_PEARSON::HASH_8()
				----------------------
			*/
			/*!
				@brief To-the-letter implementaiton of Pearson's hash function.
				@param string [in] The byte string to hash.
				@param length [in] The length of the byte stream, measured in bytes.
				@param seed [in] The intial value used to seed the random walk (default = 0).
				@return an 8-bit hash value.
			*/
			static inline size_t hash_8(const void *string, size_t length, uint8_t seed = 0)
				{
				uint8_t *byte = (uint8_t *)string;

				/*
					Perform the random walk
				*/
				for (size_t pos = 0; pos < length; pos++)
					seed = random_walk[seed ^ *byte++];

				return seed;
				}
				
			/*
				HASH_PEARSON::HASH_16()
				-----------------------
			*/
			/*!
				@brief Pearson's hash into a 16-bit value.
				@details Generates 16-bit hash on the a = 0,2,4... and b = 1,3,5,... characters which are then combined together  (b << 8 | a) for the final hash.
				@param string [in] The byte string to hash.
				@param length [in] The length of the byte stream, measured in bytes.
				@return an 16-bit hash value.
			*/
			static inline size_t hash_16(const void *string, size_t length)
				{
				/*
					Compute the results seperately then combine at the end.
					This approach is taken so that hash-values are consistent across different endian architectures.
				*/
				uint8_t seed_0 = 0;
				uint8_t seed_1 = 0;

				/*
					Take pointers to the start and end of the string
				*/
				uint8_t *byte = (uint8_t *)string;
				uint8_t *end = byte + length;

				/*
					Walk through the string to hash
				*/
				while (byte < end)
					{
					seed_0 = random_walk[seed_0 ^ *byte++];
					if (byte >= end)
						break;

					seed_1 = random_walk[seed_1 ^ *byte++];

					}
				
				/*
					construct the hash from each part.
				*/
				return seed_1 << 8 | seed_0;
				}

			/*
				HASH_PEARSON::HASH_24()
				-----------------------
			*/
			/*!
				@brief Pearson's hash into a 24-bit value.
				@details Generates 16-bit hash on the a = 0,3,6..., b = 1,4,7,..., and c = 2,5,8,...characters which are then combined together  (c << 16 | b << 8 | a) for the final hash.
				@param string [in] The byte string to hash.
				@param length [in] The length of the byte stream, measured in bytes.
				@return an 24-bit hash value.
			*/
			static inline size_t hash_24(const void *string, size_t length)
				{
				/*
					Compute the results seperately then combine at the end.
					This approach is taken so that hash-values are consistent across different endian architectures.
				*/
				uint8_t seed_0 = 0;
				uint8_t seed_1 = 0;
				uint8_t seed_2 = 0;

				/*
					Take pointers to the start and end of the string
				*/
				uint8_t *byte = (uint8_t *)string;
				uint8_t *end = byte + length;

				/*
					Walk through the string to hash
				*/
				while (byte < end)
					{
					seed_0 = random_walk[seed_0 ^ *byte++];
					if (byte >= end)
						break;

					seed_1 = random_walk[seed_1 ^ *byte++];

					if (byte >= end)
						break;
						
					seed_2 = random_walk[seed_2 ^ *byte++];
					}
				
				/*
					construct the hash from each part.
				*/
				return seed_2 << 16 | seed_1 << 8 | seed_0;
				}
				
			/*
				HASH_PEARSON::HASH_32()
				-----------------------
				Generates 32-bit hash on the 0,4,8... 1,5,9,... 2,6,9,..., 3,7,10... characters which are then combined togehter for the final hash.
			*/
			/*!
				@brief Pearson's hash into a 32-bit value.
				@details Generates 16-bit hash on the a = 0,4,8..., b = 1,5,9,..., c = 2,6,9,..., d = 3,7,10... characters which are then combined together  (d << 24 | c << 16 | b << 8 | a) for the final hash.
				@param string [in] The byte string to hash.
				@param length [in] The length of the byte stream, measured in bytes.
				@return an 32-bit hash value.
			*/
			static inline size_t hash_32(const void *string, size_t length)
				{
				/*
					Compute the results seperately then combine at the end.
					This approach is taken so that hash-values are consistent across different endian architectures.
				*/
				uint8_t seed_0 = 0;
				uint8_t seed_1 = 0;
				uint8_t seed_2 = 0;
				uint8_t seed_3 = 0;

				/*
					Take pointers to the start and end of the string
				*/
				uint8_t *byte = (uint8_t *)string;
				uint8_t *end = byte + length;

				/*
					Walk through the string to hash
				*/
				while (byte < end)
					{
					seed_0 = random_walk[seed_0 ^ *byte++];
					if (byte >= end)
						break;

					seed_1 = random_walk[seed_1 ^ *byte++];

					if (byte >= end)
						break;
						
					seed_2 = random_walk[seed_2 ^ *byte++];
						
					if (byte >= end)
						break;

					seed_3 = random_walk[seed_3 ^ *byte++];
					}
				
				/*
					construct the hash from each part.
				*/
				return (size_t)seed_3 << 24 | (size_t)seed_2 << 16 | (size_t)seed_1 << 8 | (size_t)seed_0;
				}
				
			/*
				HASH_PEARSON::HASH()
				--------------------
			*/
			/*!
				@brief Pearson's hash into a result of SIZE bits (where size = 8, 16, 24, or 32)
				@param string [in] The byte string to hash.
				@param length [in] The length of the byte stream, measured in bytes.
				@return The hash value.
			*/
			template <size_t SIZE>
			static size_t hash(const void *string, size_t length)
				{
				if (SIZE == 8)
					return hash_8(string, length);
				else if (SIZE == 16)
					return hash_16(string, length);
				else if (SIZE == 24)
					return hash_24(string, length);
				else if (SIZE == 32)
					return hash_32(string, length);
				else
					{
					assert(SIZE == 8 || SIZE == 16 || SIZE == 24 || SIZE == 32);
					}
				}
			
			/*
				HASH_PEARSON::HASH()
				--------------------
			*/
			/*!
				@brief Pearson's hash into a result of SIZE bits (where size = 8, 16, 24, or 32)
				@param string [in] The slice to hash.
				@return The hash value.
			*/
			template <size_t SIZE>
			static size_t hash(const slice &string)
				{
				if (SIZE == 8)
					return hash_8(string.address(), string.size());
				else if (SIZE == 16)
					return hash_16(string.address(), string.size());
				else if (SIZE == 24)
					return hash_24(string.address(), string.size());
				else if (SIZE == 32)
					return hash_32(string.address(), string.size());
				else
					{
					assert(SIZE == 8 || SIZE == 16 || SIZE == 24 || SIZE == 32);
					}
				}
			
			/*
				HASH_PEARSON::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

