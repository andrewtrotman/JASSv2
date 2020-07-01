/*
	ACCUMULTOR_COUNTER_INTERLEAVED.H
	--------------------------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Store the accumulator in a an array and use a query-counter array to know when to clear.
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/

#pragma once

#include <new>
#include <vector>
#include <numeric>

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "maths.h"
#include "forceinline.h"

namespace JASS
	{
	/*
		CLASS ACCUMULTOR_COUNTER_INTERLEAVED
		------------------------------------
	*/
	/*!
		@brief Store the accumulator in a an array and use a query-counter array to know when to clear.
		@details Thanks go to Antonio Mallia for inveting this method.
		@tparam ELEMENT The type of accumulator being used (default is uint16_t)
		@tparam NUMBER_OF_ACCUMULATORS The maxium number of documents allowed in any index
		@tparam COUNTER_BITSIZE The number of bits used for the query counter
	*/
	template <typename ELEMENT, size_t NUMBER_OF_ACCUMULATORS, size_t COUNTER_BITSIZE, typename = typename std::enable_if<std::is_arithmetic<ELEMENT>::value, ELEMENT>::type>
	class accumulator_counter_interleaved
		{
		static_assert(COUNTER_BITSIZE == 8 || COUNTER_BITSIZE == 4);

		/*
			This somewhat bizar line is so that unittest() can see the private members of different type instance of the class.
			Does anyone know what the actual syntax is to make it only unittest() that can see the private members?
		*/
		template<typename A, size_t B, size_t C, typename D> friend class accumulator_counter_interleaved;

		/*
			A cache line is 64 bytes so at uint16_t for an accumumulator we get 64/2=32 accumulators per chunk
			but we need clean flags (8 bits per flag) so we really only get (64*8)/(16+8) = 21 accumulators per chunk
			that means we need 21 bytes for the clean flags.  21 * 2 + 21 = 64  so there is 1 byte of padding.
			With 4 bits per flag we get  (64*8)/(16+4)=25 accumulators and 1 byte of adding
		*/
//		static constexpr size_t accumulators_per_chunk = (COUNTER_BITSIZE == 8 ? 21 : 25);
		static constexpr size_t accumulators_per_chunk = 1;
		static constexpr size_t clean_flag_bytes = COUNTER_BITSIZE == 8 ? accumulators_per_chunk : (accumulators_per_chunk / 2) + (accumulators_per_chunk & 1);

		typedef uint16_t query_counter_type;
		/*
			CLASS ACCUMULTOR_COUNTER_INTERLEAVED::CHUNK
			-------------------------------------------
		*/
		/*!
			@brief The block of memory we're going to use is an interleaving of clean flags and the accumulators
		*/

#pragma pack(push, 1)
		template <int ACCUMULATORS_PER_CHUNK>
		class chunk
			{
			public:
				/*
					We put the accumulators first so that we can do pointer arithmatic to go from an accumulator pointer to an index
				*/
				ELEMENT accumulator[accumulators_per_chunk];			///< The accumulators
				query_counter_type clean_flag[clean_flag_bytes];					///< The clean flags for this block

			public:
				/*
					ACCUMULTOR_COUNTER_INTERLEAVED::CHUNK::REWIND()
					-----------------------------------------------
				*/
				/*!
					@brief Clear the clean flags marking each accumulator as dirty
				*/
				forceinline void rewind(void)
					{
					::memset(clean_flag, min_clean_id, clean_flag_bytes);
					}

				/*
					ACCUMULTOR_COUNTER_INTERLEAVED::CHUNK::ALLOCATE_ACCUMULATOR()
					-------------------------------------------------------------
				*/
				/*!
					@brief Return a correctly cleared accumulator
					@param part_of_chunk [in] The accumulator within this chunk to use
					@param clean_id [in] The current query ID
				*/
				forceinline ELEMENT &allocate_accumulator(size_t part_of_chunk, query_counter_type clean_id)
					{
					if constexpr (COUNTER_BITSIZE == 8)
						{
						if (clean_flag[part_of_chunk] != clean_id)
							{
							clean_flag[part_of_chunk] = clean_id;
							accumulator[part_of_chunk] = 0;
							}
						}
					else
						{
						size_t clean_flag_byte = part_of_chunk >> 1;
						size_t clean_shift = (part_of_chunk & 1) * 4;

						if (((clean_flag[clean_flag_byte] >> clean_shift) & max_clean_id) != clean_id)
							{
							clean_flag[clean_flag_byte] = (clean_flag[clean_flag_byte] & ~(max_clean_id << clean_shift)) | (clean_id << clean_shift);
							accumulator[part_of_chunk] = 0;
							}
						}
					return accumulator[part_of_chunk];
					}
			};
#ifdef NEVER
		template <>
		class chunk <1>
			{
			public:
				ELEMENT accumulator;			///< The accumulators
				query_counter_type clean_flag;			///< The clean flag

			public:
				/*
					ACCUMULTOR_COUNTER_INTERLEAVED::CHUNK::REWIND()
					-----------------------------------------------
				*/
				/*!
					@brief Clear the clean flags marking each accumulator as dirty
				*/
				forceinline void rewind(void)
					{
					clean_flag = 0;
					}

				/*
					ACCUMULTOR_COUNTER_INTERLEAVED::CHUNK::ALLOCATE_ACCUMULATOR()
					-------------------------------------------------------------
				*/
				/*!
					@brief Return a correctly cleared accumulator
					@param part_of_chunk [in] The accumulator within this chunk to use
					@param clean_id [in] The current query ID
				*/
				forceinline ELEMENT &allocate_accumulator(size_t part_of_chunk, query_counter_type clean_id)
					{
					if (clean_flag != clean_id)
						{
						clean_flag = clean_id;
						accumulator = 0;
						}

					return accumulator;
					}
			};
#endif
#pragma pack(pop)

		private:
			size_t number_of_accumulators;									///< The number of accumulators that the user asked for
			size_t number_of_chunks;											///< The number of chunks of accumulators that are needed
			query_counter_type clean_id;														///< If clean_flag[x] == clean_id then accumulator[x] is valid
			static constexpr query_counter_type min_clean_id = 0;					///< The smallest clean_id, used as an initialiser for the clean flags
			static constexpr query_counter_type max_clean_id = COUNTER_BITSIZE == 8 ? 0xFF : 0x0F;	///< The largest clean_id, if we exceed this we must reinitialise the clean flags
			chunk<accumulators_per_chunk> accumulator_chunk[(NUMBER_OF_ACCUMULATORS + accumulators_per_chunk - 1) / accumulators_per_chunk];		///< The accumulators are kept in an array of chunks

		public:
			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::ACCUMULTOR_COUNTER_INTERLEAVED()
				----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
				@param number_of_accumulators [in] The numnber of elements in the array being managed.
			*/
			accumulator_counter_interleaved(size_t number_of_accumulators) :
				number_of_accumulators(number_of_accumulators),
				number_of_chunks((number_of_accumulators + accumulators_per_chunk - 1) / accumulators_per_chunk)
				{
				/*
					Clear the clean flags ready for use.
				*/
				clean_id = min_clean_id;
				for (size_t which = 0; which < number_of_chunks; which++)
					accumulator_chunk[which].rewind();
				}

			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::OPERATOR[]()
				--------------------------------------------
			*/
			/*!
				@brief Return a reference to the given accumulator
				@details The only valid way to access the accumulators is through this interface.  It ensures the accumulator
				has been initialised before the first time it is returned to the caller.
				@param which [in] The accumulator to return.
			*/
			forceinline ELEMENT &operator[](size_t which)
				{
				size_t chunk;
				size_t part_of_chunk;

				if constexpr (accumulators_per_chunk == (1 << maths::floor_log2(accumulators_per_chunk)))
					{
					/*
						Whole power of 2 so do a dividion with a shift
					*/
					chunk = which >> maths::floor_log2(accumulators_per_chunk);
					part_of_chunk = which & (accumulators_per_chunk - 1);
					}
				else
					{
					/*
						Slow division
					*/
					chunk = which / accumulators_per_chunk;
					part_of_chunk = which % accumulators_per_chunk;
					}

				return accumulator_chunk[chunk].allocate_accumulator(part_of_chunk, clean_id);
				}

			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::GET_INDEX()
				-------------------------------------------
			*/
			/*!
				@brief Given a pointer to an accumulator, return the acumulator index
				@param return a value such that get_index(&accumulator[x]) == x
			*/
			forceinline size_t get_index(ELEMENT *pointer)
				{
				auto distance = reinterpret_cast<uint8_t *>(pointer) - reinterpret_cast<uint8_t *>(accumulator_chunk);
				auto blocks = distance / sizeof(accumulator_chunk[0]);
				auto extra_bytes = distance % sizeof(accumulator_chunk[0]);

				return blocks * accumulators_per_chunk + extra_bytes / sizeof(ELEMENT);
				}

			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::SIZE()
				--------------------------------------
			*/
			/*!
				@brief Return the number of accumulators this object manages.
				@details Return the number of accumulators this object manages, which may be fewer than have been allocated.
				@return The number of accumulators being managed.
			*/
			size_t size(void) const
				{
				return number_of_accumulators;
				}

			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::REWIND()
				----------------------------------------
			*/
			/*!
				@brief Clear the accumulators ready for use
				@details This clears the clean flags so that the next time an accumulator is requested it is initialised to zero before being returned.
			*/
			void rewind(void)
				{
				if (clean_id == max_clean_id)
					{
					clean_id = min_clean_id;
					/*
						with interleaving we have to set the clean flags of each chunk seperately in order to avoid
						having to set all the accumulator flags
					*/
					for (size_t which = 0; which < number_of_chunks; which++)
						accumulator_chunk[which].rewind();
					}
				clean_id++;
				}

			/*
				ACCUMULTOR_COUNTER_INTERLEAVED::UNITTEST()
				------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Allocate an array of 64 accumulators and make sure the width and height are correct
				*/
				accumulator_counter<size_t, 64, 8> array(64);

				/*
					Populate an array with the shuffled sequence 0..instance.size()
				*/
				std::vector<size_t> sequence(array.size());
				std::iota(sequence.begin(), sequence.end(), 0);
				std::random_device random_number_generator;
				std::shuffle(sequence.begin(), sequence.end(), std::knuth_b(random_number_generator()));

				/*
					Set elemenets and make sure they're correct
				*/
				for (const auto &position : sequence)
					{
					JASS_assert(array[position] == 0);
					array[position] = position;
					JASS_assert(array[position] == position);
					}

				/*
					Make sure no over-writing happened
				*/
				for (size_t element = 0; element < array.size(); element++)
					JASS_assert(array[element] == element);

				puts("accumulator_counter_interleaved::PASSED");
				}
		};
	}
