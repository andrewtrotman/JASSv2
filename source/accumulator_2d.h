/*
	ACCUMULTOR_2D.H
	---------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <immintrin.h>

#include <new>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>

#include "simd.h"
#include "maths.h"
#include "forceinline.h"

//#define USE_QUERY_IDS 1

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
		@tparam NUMBER_OF_ACCUMULATORS The maxium number of documents allowed in any index
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
#ifdef USE_QUERY_IDS
			typedef uint16_t flag_type;
#else
			typedef uint8_t flag_type;
#endif

		private:
			/*
				This class is templated so as to put the array and the clean flags in-object rather than pointers.  So, its necessary
				to work out the maxumum sizes of the two arrays at compile time and the check at construction that no overflow
				is happening. This code works out the maximums and allocates the arrays.
			*/
			static constexpr size_t maximum_shift = maths::floor_log2(maths::sqrt_compiletime(NUMBER_OF_ACCUMULATORS));					///< The amount to shift to get the right clean flag
			static constexpr size_t maximum_width = 1 << maximum_shift;																					///< Each clean flag represents this number of accumulators in a "row"
		public:
			static constexpr size_t maximum_number_of_clean_flags = (NUMBER_OF_ACCUMULATORS + maximum_width - 1) / maximum_width;	///< The number of "rows" (i.e. clean flags).
		private:
			static constexpr size_t maximum_number_of_accumulators_allocated = maximum_width * maximum_number_of_clean_flags;			///< The numner of accumulators that were actually allocated (recall that this is a 2D array)
		public:
			typedef std::array<flag_type, maximum_number_of_clean_flags> clean_flag_t;
			clean_flag_t clean_flag;																								///< The clean flags are kept as bytes for faster lookup
			typedef std::array<ELEMENT, maximum_number_of_accumulators_allocated> accumulator_t;																				///< The accumulators are kept in an array
			accumulator_t accumulator;																							///< The accumulators are kept in an array
//			flag_type clean_flag[maximum_number_of_clean_flags];																								///< The clean flags are kept as bytes for faster lookup
//			ELEMENT accumulator[maximum_number_of_accumulators_allocated];																							///< The accumulators are kept in an array

#ifdef USE_QUERY_IDS
			flag_type query_id;
#endif
			/*
				At run-time we use these parameters
			*/
		public:
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
			*/
			accumulator_2d()
#ifdef USE_QUERY_IDS
				:
				query_id(std::numeric_limits<decltype(query_id)>::max())
#endif
				{
				/* Nothing */
				}

			/*
				ACCUMULATOR_2D::~ACCUMULATOR_2D()
				---------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~accumulator_2d()
				{
				}

			/*
				ACCUMULATOR_2D::INIT()
				----------------------
			*/
			/*!
				@brief Initialise this object before first use.
				@param number_of_accumulators [in] The numnber of elements in the array being managed.
			*/
			void init(size_t number_of_accumulators)
				{
				this->number_of_accumulators = number_of_accumulators;
				/*
					If the width of the accumulator array is a whole power of 2 the its quick to find the clean flag.  If the width is the square root of the
					number of accumulators then it ballances the number of accumulator with the number of clean flags.  Both techniques are used.
					Simply taking log2(sqrt(element)) can result in massive disparity in width vs height (63->4x16) so we try to ballance this
					by checking if they are closer together if we take the ceiling of the log rather than the floor.
				*/
				size_t square_root = (size_t)sqrt(number_of_accumulators);
				shift = maths::floor_log2(square_root);

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

				/*
					Clear the clean flags ready for use.
				*/
				rewind();
				}

			/*
				ACCUMULATOR_2D::WHICH_CLEAN_FLAG()
				----------------------------------
			*/
			/*!
				@brief Return the id of the clean flag to use.
				@param element [in] The accumulator number.
				@return The clean flag number.
			*/
			forceinline size_t which_clean_flag(size_t element) const
				{
				return element >> shift;
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
				size_t flag = which_clean_flag(which);

#ifdef USE_QUERY_IDS
				if (clean_flag[flag] != query_id)
					{
					::memset(&accumulator[0] + flag * width, 0, width * sizeof(accumulator[0]));
					clean_flag[flag] = query_id;
					}
#else
				if (!clean_flag[flag])
					{
					::memset(&accumulator[0] + flag * width, 0, width * sizeof(accumulator[0]));
					clean_flag[flag] = true;
					}
#endif

				return accumulator[which];
				}

			/*
				ACCUMULATOR_2D::WHICH_CLEAN_FLAG()
				----------------------------------
			*/
			/*!
				@brief Return the ids of the clean flags to use.
				@param element [in] The accumulator numbers.
				@return The clean flag numbers.
			*/
			forceinline __m256i which_clean_flag(__m256i element) const
				{
				return _mm256_srli_epi32(element, shift);
				}

			/*
				ACCUMULATOR_2D::OPERATOR[]()
				----------------------------
			*/
			/*!
				@brief Return a set of acumulators
				@details The only valid way to access the accumulators is through this interface.
				It ensures the accumulator has been initialised before the first
				time it is returned to the caller.
				@param which [in] The accumulators to return.
				@return The values of the accumulators.
			*/
			forceinline __m256i operator[](__m256i which)
				{
				__m256i indexes = which_clean_flag(which);
				__m256i flags = simd::gather(&clean_flag[0], indexes);

				uint32_t got = _mm256_movemask_epi8(flags);
				if (got != 0x11111111)
					{
					uint32_t single_flag;
					/*
						At least one of the rows is unclean.  It might be that two
						bits represent the same row so we must check for that
					*/
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 0)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 1)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 2)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 3)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 4)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 5)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 6)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					if (!clean_flag[single_flag = _mm256_extract_epi32(indexes, 7)])
						{
						::memset(&accumulator[0] + single_flag * width, 0, width * sizeof(accumulator[0]));
						clean_flag[single_flag] = 0xFF;
						}
					}

				return simd::gather(&accumulator[0], which);
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
				return pointer - &accumulator[0];
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
#ifdef USE_QUERY_IDS
				if (query_id == std::numeric_limits<decltype(query_id)>::max())
					{
					::memset(&clean_flag[0], 0, number_of_clean_flags * sizeof(clean_flag[0]));
					query_id = 0;
					}
				query_id++;
#else
//				std::fill(clean_flag, clean_flag + number_of_clean_flags, false);
				::memset(&clean_flag[0], 0, number_of_clean_flags * sizeof(clean_flag[0]));
#endif
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
				std::random_device random_number_generator;
				std::shuffle(sequence.begin(), sequence.end(), std::knuth_b(random_number_generator()));

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
				accumulator_2d<size_t, 64> array;
				array.init(64);
				JASS_assert(array.width == 8);
				JASS_assert(array.shift == 3);
				JASS_assert(array.number_of_clean_flags == 8);

				unittest_example(array);

				/*
					Make sure it all works right when there is a single accumulator in the last row
				*/
				accumulator_2d<size_t, 65> array_hangover;
				array_hangover.init(65);
				JASS_assert(array_hangover.width == 8);
				JASS_assert(array_hangover.shift == 3);
				JASS_assert(array_hangover.number_of_clean_flags == 9);

				unittest_example(array_hangover);

				/*
					Make sure it all works right when there is a single accumulator missing from the last row
				*/
				accumulator_2d<size_t, 63> array_hangunder;
				array_hangunder.init(63);
				JASS_assert(array_hangunder.width == 4);
				JASS_assert(array_hangunder.shift == 2);
				JASS_assert(array_hangunder.number_of_clean_flags == 16);

				unittest_example(array_hangunder);

				/*
					Make sure it all works right when there is a single accumulator
				*/
				accumulator_2d<size_t, 1> array_one;
				array_one.init(1);
				JASS_assert(array_one.width == 1);
				JASS_assert(array_one.shift == 0);
				JASS_assert(array_one.number_of_clean_flags == 1);

				unittest_example(array_one);

				puts("accumulator_2d::PASSED");
				}
		};
	}
