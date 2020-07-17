/*
	ACCUMULTOR_COUNTER.H
	--------------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Store the accumulator in an array and use a query-counter array to know when to clear.
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
		CLASS ACCUMULATOR_COUNTER
		-------------------------
	*/
	/*!
		@brief Store the accumulator in a an array and use a query-counter array to know when to clear.
		@details Keep the accumulagor scores in an array, and use a second array as a set of dirty flags.  That second array
		simply stores the ID of the query that last used the accumulator. By incrementing the query_id on each rewind its possible to
		avoid initilising the acumulator array most of the time, at the cost of NUMBER_OF_ACCUMULATORS units of storage.
		Thanks go to Antonio Mallia for inveting this method.
		@tparam ELEMENT The type of accumulator being used (default is uint16_t)
		@tparam NUMBER_OF_ACCUMULATORS The maxium number of documents allowed in any index
		@tparam COUNTER_BITSIZE The number of bits used for the query counter
	*/
	template <typename ELEMENT, size_t NUMBER_OF_ACCUMULATORS, size_t COUNTER_BITSIZE, typename = typename std::enable_if<std::is_arithmetic<ELEMENT>::value, ELEMENT>::type>
	class accumulator_counter
		{
		static_assert(COUNTER_BITSIZE == 8 || COUNTER_BITSIZE == 4);
		/*
			This somewhat bizar line is so that unittest() can see the private members of different type instance of the class.
		*/
		template<typename A, size_t B, size_t C, typename D> friend class accumulator_counter;

		private:
			size_t number_of_accumulators;																///< The number of accumulators that the user asked for
			uint8_t clean_id;																					///< If clean_flag[x] == clean_id then accumulator[x] is valid
			static constexpr uint8_t max_clean_id = (COUNTER_BITSIZE == 8 ? 0xFF : 0x0F);	///< The largest allowable clean id
			static constexpr uint8_t min_clean_id = 0;												///< The smallest allowable clean id
			uint8_t clean_flag[NUMBER_OF_ACCUMULATORS];												///< The clean flags are kept as bytes for faster lookup
			ELEMENT accumulator[NUMBER_OF_ACCUMULATORS];												///< The accumulators are kept in an array

		public:
			/*
				ACCUMULATOR_COUNTER::ACCUMULATOR_COUNTER()
				------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			accumulator_counter() :
				number_of_accumulators(0),
				clean_id(min_clean_id)
				{
				/* Nothing */
				}

			/*
				ACCUMULATOR_COUNTER::INIT()
				---------------------------
			*/
			/*!
				@brief Initialise this object before first use.
				@param number_of_accumulators [in] The numnber of elements in the array being managed.
				@param preferred_width [in] Not used (for comparibility with other classes)
			*/
			void init(size_t number_of_accumulators, size_t preferred_width = 0)
				{
				this->number_of_accumulators = number_of_accumulators;
				clean_id = min_clean_id;

				/*
					Clear the clean flags and accumulators ready for use.
				*/
				std::fill(clean_flag, clean_flag + number_of_accumulators, min_clean_id);
				std::fill(accumulator, accumulator + number_of_accumulators, ELEMENT());
				}

			/*
				ACCUMULATOR_COUNTER::GET_VALUE()
				--------------------------------
			*/
			/*!
				@brief Return the value of the given accumulator
				@details This interface does not initialise an accumulator, it returns 0 if the accumulator is uninitialised
				@param which [in] The accumulator to return.
				@return The accumulator value or 0.
			*/
			forceinline ELEMENT get_value(size_t which)
				{
				if constexpr (COUNTER_BITSIZE == 8)
					if (clean_flag[which] != clean_id)
						return 0;
					else
						return accumulator[which];
				else
					if (((clean_flag[which >> 1] >> ((which & 1) * 4)) & max_clean_id) != clean_id)
						return 0;
					else
						return accumulator[which];
				}

			/*
				ACCUMULATOR_COUNTER::OPERATOR[]()
				---------------------------------
			*/
			/*!
				@brief Return a reference to the given accumulator
				@details The only valid way to access the accumulators is through this interface.  It ensures the accumulator
				has been initialised before the first time it is returned to the caller.
				@param which [in] The accumulator to return.
				@return The accumulator.
			*/
			forceinline ELEMENT &operator[](size_t which)
				{
				if constexpr (COUNTER_BITSIZE == 8)
					{
					if (clean_flag[which] != clean_id)
						{
						accumulator[which] = 0;
						clean_flag[which] = clean_id;
						}
					}
				else
					{
					size_t clean_flag_byte = which >> 1;
					size_t clean_shift = (which & 1) * 4;

					if (((clean_flag[clean_flag_byte] >> clean_shift) & max_clean_id) != clean_id)
						{
						accumulator[which] = 0;
						clean_flag[clean_flag_byte] = (clean_flag[clean_flag_byte] & ~(max_clean_id << clean_shift)) | (clean_id << clean_shift);
						}
					}
				return accumulator[which];
				}

			/*
				ACCUMULTOR_COUNTER::GET_INDEX()
				-------------------------------
			*/
			/*!
				@brief Given a pointer to an accumulator, return the acumulator index
				@param return a value such that get_index(&accumulator[x]) == x
			*/
			forceinline size_t get_index(ELEMENT *pointer)
				{
				return pointer - accumulator;
				}

			/*
				ACCUMULATOR_COUNTER::SIZE()
				---------------------------
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
				ACCUMULATOR_COUNTER::REWIND()
				-----------------------------
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
					if constexpr (COUNTER_BITSIZE == 8)
						std::fill(clean_flag, clean_flag + number_of_accumulators, min_clean_id);
					else
						std::fill(clean_flag, clean_flag + (number_of_accumulators + 1) / 2, min_clean_id);
					}
				clean_id++;
				}

			/*
				ACCUMULATOR_COUNTER::UNITTEST()
				-------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Allocate an array of 64 accumulators and make sure the width and height are correct
				*/
				accumulator_counter<size_t, 64, 8> array;
				array.init(64);

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

				puts("accumulator_counter::PASSED");
				}
		};
	}
