/*
	REVERSE.H
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Use reverse iterators in a for loop
	@details Based on https://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop

	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <vector>
#include <sstream>

#include "asserts.h"

namespace JASS
	{
	/*
		CLASS REVERSE_ADAPTER
		---------------------
	*/
	/*!
		@brief Addaptor class used in reverse iteration
		@details When used with reverse() this class makes it possible to write (in the JASS namespace):
		@code
		void example(void)
			{
			std::vector<int> numbers {1, 2, 3, 4, 5};

			for (const auto value : reverse(numbers))
				std::cout << value;
			}
		@endcode
		and to get 54321 as the answer.
	*/
	template <typename TYPE>
	class reverse_adapter
		{
		private:
			TYPE &object;				///< The object to reverse iterate over

		public:
			/*
				REVERSE_ADAPTER::REVERSE
				------------------------
			*/
			/*!
				@brief Constructor
				@param object [in] The object to reverse iterate over

			*/
			reverse_adapter(TYPE &object):
				object(object)
				{
				/* Noting */
				}

			/*
				REVERSE_ADAPTER::BEGIN()
				------------------------
			*/
			/*!
				@brief Return a reversing iterator pointing to the end of the collection
				@return Iterator pointing to end of array.
			*/
			auto begin(void) const
				{
				return object.rbegin();
				}

			/*
				REVERSE_ADAPTER::END()
				----------------------
			*/
			/*!
				@brief Return a reversing iterator pointing to one before the start of the collection
				@return Iterator pointing to one before the beginning of array.
			*/
			auto end() const
				{
				return object.rend();
				}

			/*
				REVERSE_ADAPTER::UNITTEST()
				---------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};

	/*
		REVERSE()
		---------
	*/
	/*!
		@brief Construct an object that causes for(const auto &a : reverse(b)) to iterate fromthe end of b to the start of b.
		@param object [in] The object that will be iterated over.
	*/
	template <typename TYPE>
	auto reverse(TYPE &object)
		{
		return reverse_adapter<TYPE>(object);
		}

	/*
		REVERSE_ADAPTER::UNITTEST()
		---------------------------
	*/
	/*!
		@brief Unit test this
	*/
	template <typename TYPE>
	void reverse_adapter<TYPE>::unittest(void)
		{
		std::ostringstream got;
		std::vector<int> numbers {1, 2, 3, 4, 5};

		for (const auto value : reverse(numbers))
			got << value;

		JASS_assert(got.str() == "54321");
		puts("reverse_adapter:PASSED");
		}
	}
