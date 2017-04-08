/*
	ACCUMULTOR_2D.H
	---------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Manage an accumulator array as a 2D array with dirty bits.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#include <new>
#include <numeric>

#include <math.h>
#include <stdint.h>

namespace JASS
	{
	/*
		CLASS ACCUMULATOR_2D
		--------------------
	*/
	/*!
		@brief Store the accumulators in a 2D array as originally used in ATIRE.
		@details Manage the accumulagtor array as a two dimensional array.  This approach avoids initialising the accumulators on each search by
		breakig it into a series of pages and keeping dirty flag for each page.  A page of accmumulators is only initialised if one of the elements
		in that page is touched.  This detail is kepts in a set of flags (one per page) known as the dirty flags.  The details are described in
		X.-F. Jia, A. Trotman, R. O'Keefe (2010), Efficient Accumulator Initialisation, Proceedings of the 15th Australasian Document Computing Symposium (ADCS 2010).
		This implementation differs from that implenentation is so far as the size of the page is alwaya a whole power of 2 and thus the dirty flag can
		be found wiht a bit shit rather than a mod.
		@tparam ELEMENT The type of accumulator being used (default is uint16_t)
	*/
	template <typename ELEMENT = uint16_t>
	class accumulator_2d
		{
		/*
			This somewhat bizar line is so that unittest() can see the private members of another instance of the class.
			Does anyone know what the actual syntax is to make it only unittest() that can see the private members?
		*/
		template<typename A> friend class accumulator_2d;

		private:
			ELEMENT *accumulator;							///< The accumulators are kept in an array
			uint8_t *dirty_flag;								///< The dirty flags are kept as bytes for faster lookup
			size_t width;										///< Each dirty flag represents this number of accumulators in a "row"
			size_t number_of_dirty_flags;					///< The numnber of "rows" (i.e. dirty flags).
			size_t shift;										///< How far we need to shift right the index to get the index of the dirty flag
			size_t number_of_accumulators;				///< The number of accumulators as asked for by the user (more may be allocated)

		public:
			/*
				ACCUMULATOR_2D::ACCUMULATOR_2D()
				--------------------------------
			*/
			/*!
				@brief Constructor.
				@param elements [in] The numnber of elements in the array being managed.
			*/
			accumulator_2d(size_t elements) :
				number_of_accumulators(elements)
				{
				/*
					If the width of the accumulator array is a whole power of 2 the its quick to find the dirty flag.  If the width is the square root of the
					number of accumulators then it ballances the number of accumulator with the number of dirty flags.  Both techniques are used.
					Simply taking log2(sqrt(element)) can result in massive disparity in width vs height (63->4x16) so we try to ballane this
					by checking if they are closer together if we take the ceiling of the log rather than the floor.
				*/
				auto square_root = sqrt(elements);
				shift = log2(square_root);
				width = 1 << shift;
				if (((size_t)square_root & (1 << (shift - 1))) != 0)
					width = 1 << ++shift;

				/*
					Round up the number of dirty flags so that if the number of accumulators isn't a square that we don't miss the last row
				*/
				number_of_dirty_flags = (elements + width - 1) / width;

				/*
					Allocate the dirty flags and the accumulators making sure to allocate a true square number of accumulators so that the last row is full.
				*/
				dirty_flag = new (std::nothrow) uint8_t [number_of_dirty_flags];
				accumulator = new (std::nothrow) ELEMENT [width * number_of_dirty_flags];

				/*
					Clear the dirty flags ready for use.
				*/
				rewind();
				}

			/*
				ACCUMULATOR_2D::~ACCUMULATOR_2D()
				---------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			~accumulator_2d()
				{
				delete [] accumulator;
				delete [] dirty_flag;
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
			ELEMENT &operator[](size_t which)
				{
				auto flag = which >> shift;
				if (!dirty_flag[flag])
					{
					std::fill(&accumulator[flag * width], &accumulator[flag * width + width], 0);
					dirty_flag[flag] = true;
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
				@details This clears the dirty flags so that the next time an accumulator is requested it ix initialised to zero before being returned.
			*/
			void rewind(void)
				{
				std::fill(dirty_flag, dirty_flag + number_of_dirty_flags, false);
				}


			/*
				ACCUMULATOR_2D::UNITTEST_EXAMPLE()
				----------------------------------
			*/
			/*!
				@brief Unit test a single 2D accumulator instance making sure its correct
			*/
			static void unittest_example(accumulator_2d &instance)
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
				accumulator_2d array(64);
				JASS_assert(array.width == 8);
				JASS_assert(array.shift == 3);
				JASS_assert(array.number_of_dirty_flags == 8);

				unittest_example(array);

				/*
					Make sure it all works right when there is a single accumulator in the last row
				*/
				accumulator_2d array_hangover(65);
				JASS_assert(array_hangover.width == 8);
				JASS_assert(array_hangover.shift == 3);
				JASS_assert(array_hangover.number_of_dirty_flags == 9);

				unittest_example(array_hangover);

				/*
					Make sure it all works right when there is a single accumulator missing from the last row
				*/
				accumulator_2d array_hangunder(63);
				JASS_assert(array_hangunder.width == 8);
				JASS_assert(array_hangunder.shift == 3);
				JASS_assert(array_hangunder.number_of_dirty_flags == 8);

				unittest_example(array_hangunder);


				puts("accumulator_2d::PASSED");
				}
		};
	}








