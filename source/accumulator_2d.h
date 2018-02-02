/*
	ACCUMULTOR_2D.H
	---------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Manage an accumulator array as a 2D array with clean bits.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

//#define DYNAMIC_ALLOCATION

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
		CLASS ACCUMULATOR_2D
		--------------------
	*/
	/*!
		@brief Store the accumulators in a 2D array as originally used in ATIRE.
		@details Manage the accumulagtor array as a two dimensional array.  This approach avoids initialising the accumulators on each search by
		breakig it into a series of pages and keeping clean flag for each page.  A page of accmumulators is only initialised if one of the elements
		in that page is touched.  This detail is kepts in a set of flags (one per page) known as the clean flags.  The details are described in
		X.-F. Jia, A. Trotman, R. O'Keefe (2010), Efficient Accumulator Initialisation, Proceedings of the 15th Australasian Document Computing Symposium (ADCS 2010).
		This implementation differs from that implenentation is so far as the size of the page is alwaya a whole power of 2 and thus the clean flag can
		be found wiht a bit shit rather than a mod.
		@tparam ELEMENT The type of accumulator being used (default is uint16_t)
	*/
	template <typename ELEMENT, size_t NUMBER_OF_ACCUMULATORS, typename = typename std::enable_if<std::is_arithmetic<ELEMENT>::value, ELEMENT>::type>
	class accumulator_2d
		{
		/*
			This somewhat bizar line is so that unittest() can see the private members of another instance of the class.
			Does anyone know what the actual syntax is to make it only unittest() that can see the private members?
		*/
		template<typename A, size_t B, typename C> friend class accumulator_2d;

		private:
			/*
				This class is templated so as to put the array and the clean flags in-object rather than pointers.  So, its necessary
				to work out the maxumum sizes of the two arrays at compile time and the check at construction that no overflow
				is happening. This code works out the maximums and allocates the arrays.
			*/
			static constexpr size_t maximum_shift = maths::floor_log2(maths::sqrt_compiletime(NUMBER_OF_ACCUMULATORS));					///< The amount to shift to get the right clean flag
			static constexpr size_t maximum_width = 1 << maximum_shift;																					///< Each clean flag represents this number of accumulators in a "row"
			static constexpr size_t maximum_number_of_clean_flags = (NUMBER_OF_ACCUMULATORS + maximum_width - 1) / maximum_width;	///< The number of "rows" (i.e. clean flags).
			static constexpr size_t maximum_number_of_accumulators_allocated = maximum_width * maximum_number_of_clean_flags;			///< The numner of accumulators that were actually allocated (recall that this is a 2D array)
#ifdef DYNAMIC_ALLOCATION
			uint8_t *clean_flag;
			ELEMENT *accumulator;
#else
			uint8_t clean_flag[maximum_number_of_clean_flags];																								///< The clean flags are kept as bytes for faster lookup
			ELEMENT accumulator[maximum_number_of_accumulators_allocated];																				///< The accumulators are kept in an array
#endif
			/*
				At run-time we use these parameters
			*/
			size_t width;												///< Each clean flag represents this number of accumulators in a "row"
			size_t shift;												///< The amount to shift to get the right clean flag
			size_t number_of_clean_flags;							///< The number of "rows" (i.e. clean flags)
			size_t number_of_accumulators_allocated;			///< The numner of accumulators that were actually allocated (recall that this is a 2D array)
			size_t number_of_accumulators;						///< The number of accumulators that the user asked for

		public:
			/*
				ACCUMULATOR_2D::ACCUMULATOR_2D()
				--------------------------------
			*/
			/*!
				@brief Constructor.
				@param number_of_accumulators [in] The numnber of elements in the array being managed.
			*/
			accumulator_2d(size_t number_of_accumulators) :
				number_of_accumulators(number_of_accumulators)
				{
				/*
					If the width of the accumulator array is a whole power of 2 the its quick to find the clean flag.  If the width is the square root of the
					number of accumulators then it ballances the number of accumulator with the number of clean flags.  Both techniques are used.
					Simply taking log2(sqrt(element)) can result in massive disparity in width vs height (63->4x16) so we try to ballance this
					by checking if they are closer together if we take the ceiling of the log rather than the floor.
				*/
				size_t square_root = (size_t)sqrt(number_of_accumulators);
				shift = maths::floor_log2(square_root);
//				if (((size_t)square_root & ((size_t)1 << (shift - 1))) != 0)
//					shift++;

				width = (size_t)1 << shift;

				/*
					Round up the number of clean flags so that if the number of accumulators isn't a square that we don't miss the last row
				*/
				number_of_clean_flags = (number_of_accumulators + width - 1) / width;
				
				/*
					Round up the number of accumulators to make a rectangle (we're a 2D array)
				*/
				number_of_accumulators_allocated = width * number_of_clean_flags;

				/*
					Check we've not gone past the end of the arrays
				*/
				if (number_of_clean_flags > maximum_number_of_clean_flags || number_of_accumulators_allocated > maximum_number_of_accumulators_allocated)
					throw std::bad_array_new_length();

#ifdef DYNAMIC_ALLOCATION
				/*
					Allocate the clean flags and the accumulators using new
				*/
				clean_flag = new uint8_t[number_of_clean_flags];
				accumulator = new ELEMENT[number_of_accumulators_allocated];
#endif

				/*
					Clear the clean flags ready for use.
				*/
				rewind();
				}

			/*
				ACCUMULATOR_2D::OPERATOR[]()
				----------------------------
			*/
			/*!
				@brief Return a reference to the given accumulator
				@details The only valid way to access the accumulators is through this interface.  It ensures the accumulator has been initialised before thr first
				time it is returned to the caller.
				@param which [in] The accumulator to return.
			*/
			forceinline ELEMENT &operator[](size_t which)
				{
				size_t flag = which >> shift;
				if (!clean_flag[flag])
					{
					std::fill(&accumulator[flag * width], &accumulator[flag * width + width], 0);
					clean_flag[flag] = true;
					}

				return accumulator[which];
				}
			
			/*
				ACCUMULATOR_2D::SIZE()
				----------------------
			*/
			/*!
				@brief Return the number of accumulators in the array.
				@details Return the number of accumulators in the array which may be fewer than have been allocated.
				@return Size of the accumulator array.
			*/
			size_t size(void) const
				{
				return number_of_accumulators;
				}

			/*
				ACCUMULATOR_2D::REWIND()
				------------------------
			*/
			/*!
				@brief Clear the accumulators ready for use
				@details This clears the clean flags so that the next time an accumulator is requested it ix initialised to zero before being returned.
			*/
			void rewind(void)
				{
				std::fill(clean_flag, clean_flag + number_of_clean_flags, false);
				}

			/*
				ACCUMULATOR_2D::UNITTEST_EXAMPLE()
				----------------------------------
			*/
			/*!
				@brief Unit test a single 2D accumulator instance making sure its correct
			*/
			template <typename ACCUMULATOR_2D>
			static void unittest_example(ACCUMULATOR_2D &instance)
				{
				/*
					Populate an array with the shuffled sequence 0..instance.size()
				*/
				std::vector<size_t> sequence(instance.size());
				std::iota(sequence.begin(), sequence.end(), 0);
				std::random_shuffle(sequence.begin(), sequence.end());

				/*
					Set elemenets and make sure they're correct
				*/
				for (const auto &position : sequence)
					{
					JASS_assert(instance[position] == 0);
					instance[position] = position;
					JASS_assert(instance[position] == position);
					}

				/*
					Make sure no over-writing happened
				*/
				for (size_t element = 0; element < instance.size(); element++)
					JASS_assert(instance[element] == element);
				}

			/*
				ACCUMULATOR_2D::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Allocate an array of 64 accumulators and make sure the width and height are correct
				*/
				accumulator_2d<size_t, 64> array(64);
				JASS_assert(array.width == 8);
				JASS_assert(array.shift == 3);
				JASS_assert(array.number_of_clean_flags == 8);

				unittest_example(array);

				/*
					Make sure it all works right when there is a single accumulator in the last row
				*/
				accumulator_2d<size_t, 65> array_hangover(65);
				JASS_assert(array_hangover.width == 8);
				JASS_assert(array_hangover.shift == 3);
				JASS_assert(array_hangover.number_of_clean_flags == 9);

				unittest_example(array_hangover);

				/*
					Make sure it all works right when there is a single accumulator missing from the last row
				*/
				accumulator_2d<size_t, 63> array_hangunder(63);
				JASS_assert(array_hangunder.width == 4);
				JASS_assert(array_hangunder.shift == 2);
				JASS_assert(array_hangunder.number_of_clean_flags == 16);

				unittest_example(array_hangunder);

				/*
					Make sure it all works right when there is a single accumulator
				*/
				accumulator_2d<size_t, 1> array_one(1);
				JASS_assert(array_one.width == 1);
				JASS_assert(array_one.shift == 0);
				JASS_assert(array_one.number_of_clean_flags == 1);

				unittest_example(array_one);

				puts("accumulator_2d::PASSED");
				}
		};
	}
