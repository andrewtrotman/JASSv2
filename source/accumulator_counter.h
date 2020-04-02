/*
	ACCUMULTOR_COUNTER.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
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
	*/
	template <typename ELEMENT, size_t NUMBER_OF_ACCUMULATORS, typename = typename std::enable_if<std::is_arithmetic<ELEMENT>::value, ELEMENT>::type>
	class accumulator_counter
		{
		/*
			This somewhat bizar line is so that unittest() can see the private members of another instance of the class.
			Does anyone know what the actual syntax is to make it only unittest() that can see the private members?
		*/
		template<typename A, size_t B, typename C> friend class accumulator_2d;

		private:
			size_t number_of_accumulators;									///< The number of accumulators that the user asked for
			uint8_t clean_id;														///< If clean_flag[x] == clean_id then accumulator[x] is valid
			decltype(clean_id) clean_flag[NUMBER_OF_ACCUMULATORS];	///< The clean flags are kept as bytes for faster lookup
			ELEMENT accumulator[NUMBER_OF_ACCUMULATORS];					///< The accumulators are kept in an array

		public:
			/*
				ACCUMULATOR_COUNTER::ACCUMULATOR_COUNTER()
				------------------------------------------
			*/
			/*!
				@brief Constructor.
				@param number_of_accumulators [in] The numnber of elements in the array being managed.
			*/
			accumulator_counter(size_t number_of_accumulators) :
				number_of_accumulators(number_of_accumulators)
				{
				/*
					Clear the clean flags and accumulators ready for use.
				*/
				clean_id = 0;
				std::fill(clean_flag, clean_flag + number_of_accumulators, 0);
				std::fill(accumulator, accumulator + number_of_accumulators, 0);
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
			*/
			forceinline ELEMENT &operator[](size_t which)
				{
				if (clean_flag[which] != clean_id)
					{
					accumulator[which] = 0;
					clean_flag[which] = clean_id;
					}
				return accumulator[which];
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
				if (clean_id == std::numeric_limits<decltype(clean_id)>::max())
					{
					clean_id = std::numeric_limits<decltype(clean_id)>::min();
					std::fill(clean_flag, clean_flag + number_of_accumulators, clean_id);
					}
				else
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
				accumulator_counter<size_t, 64> array(64);

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
