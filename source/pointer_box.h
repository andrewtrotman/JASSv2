/*
	POINTER_BOX.H
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A container type for pointers that allows comparison of the values pointed to the pointer
	@details All the usual pointer operations should work, except for comparisons which compare to the
	value pointed to rather than the pointer itself.  This is useful when, for example, an ordered conatiner
	is being used to store pointers (or, for example, std::sort() an array of pointers based on the values
	those pointers point to).
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include "asserts.h"

namespace JASS
	{
	/*
		CLASS POINTER_BOX
		-----------------
	*/
	/*!
		@brief A boxed pointer object used so that comparison operators work on the value pointed to rather than the pointer
		@details All the usual pointer operations should work, except for comparisons which compare to the
		value pointed to rather than the pointer itself.  This is useful when, for example, an ordered conatiner
		is being used to store pointers (or, for example, std::sort() an array of pointers based on the values
		those pointers point to).  Note, ties are broken on the value of the pointer - that means that if the
		pointers are into an array then the earlier memeber of the array compares to less than the
		
*/
	template <typename TYPE>
	class pointer_box
		{
		private:
			TYPE *element;				///< We actually store a reference to the object rather than a pointer, but they're the same size.
			
		public:
			/*
				POINTER_BOX::POINTER_BOX()
				--------------------------
			*/
			/*!
				@brief Constructor
			*/
			pointer_box()
				{
				/* Nothing */
				}

			/*
				POINTER_BOX::POINTER_BOX()
				--------------------------
			*/
			/*!
				@brief Constructor
				@param to [in] The object we are being a pointer to.
			*/
			pointer_box(TYPE *to) :
				element(to)
				{
				/* Nothing */
				}

			/*
				POINTER_BOX::CMP()
				------------------
			*/
			/*!
				@brief Compare for less than, equal, or greater than
				@param first [in] The object we are Compareing to.
				@param to [in] The object we are Compareing to.
				@return -ve for less, 0 for equal, +ve for greater.
			*/
			static int cmp(const pointer_box<TYPE> first, const pointer_box<TYPE> to)
				{
				if (*first.element < *to.element)
					return -1;
				else if (*first.element > *to.element)
					return 1;
				else
					{
					if (first.element < to.element)
						return -1;
					else if (first.element > to.element)
						return 1;
					else
						return 0;
					}
				}

			/*
				POINTER_BOX::COMPARE()
				----------------------
			*/
			/*!
				@brief Compare for less than, equal, or greater than
				@param to [in] The object we are Compareing to.
				@return -ve for less, 0 for equal, +ve for greater.
			*/
			int compare(const pointer_box<TYPE> to) const
				{
				return cmp(*this, to);
				}

			/*
				POINTER_BOX::LESS_THAN()
				------------------------
			*/
			/*!
				@brief Compare for less than (*a < *b) || (*a == *b && a < b).
				@param first [in] This object
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			static bool less_than(TYPE *first, TYPE *to)
				{
				return *first < *to || (*first == *to && first < to);
				}

			/*
				POINTER_BOX::LESS_THAN()
				------------------------
			*/
			/*!
				@brief Compare for less than (*a < *b) || (*a == *b && a < b).
				@param first [in] This object
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			static bool less_than(TYPE *first, const pointer_box<TYPE> to)
				{
				return less_than(first, to.element);
				}

			/*
				POINTER_BOX::OPERATOR<()
				------------------------
			*/
			/*!
				@brief Compare for less than (*a < *b) || (*a == *b && a < b).
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			bool operator<(const pointer_box<TYPE> to) const
				{
				return *element < *to.element || (*element == *to.element && element < to.element);
				}

			/*
				POINTER_BOX::OPERATOR<()
				------------------------
			*/
			/*!
				@brief Compare for less than (*a < *b) || (*a == *b && a < b).
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			bool operator<(TYPE *to) const
				{
				return less_than(element, to);
				}


			/*
				POINTER_BOX::OPERATOR<=()
				-------------------------
			*/
			/*!
				@brief Compare for less than or equal to.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator<=(const pointer_box<TYPE> to) const
				{
				return *element < *to.element || (*element == *to.element && element <= to.element);
				}

			/*
				POINTER_BOX::GREATER_THAN()
				---------------------------
			*/
			/*!
				@brief Compare for more than (*a > *b) || (*a == *b && a > b).
				@param first [in] This object
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			static bool greater_than(TYPE *first, TYPE *to)
				{
				return *first > *to || (*first == *to && first > to);
				}

			/*
				POINTER_BOX::GREATER_THAN()
				---------------------------
			*/
			/*!
				@brief Compare for more than (*a > *b) || (*a == *b && a > b).
				@param first [in] This object
				@param to [in] The object we are comparing to.
				@return true or false.
			*/
			static bool greater_than(TYPE *first, pointer_box<TYPE> to)
				{
				return greater_than(first, to.element);
				}

			/*
				POINTER_BOX::OPERATOR>()
				-------------------------
			*/
			/*!
				@brief Compare for greater than.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator>(const pointer_box<TYPE> to) const
				{
				return greater_than(element, to.element);
				}

			/*
				POINTER_BOX::OPERATOR>=()
				--------------------------
			*/
			/*!
				@brief Compare for greater than or equal to.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator>=(const pointer_box<TYPE> to) const
				{
				return *element > *to.element || (*element == *to.element && element >= to.element);
				}

			/*
				POINTER_BOX::OPERATOR==()
				--------------------------
			*/
			/*!
				@brief Compare for equal to.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator==(const pointer_box<TYPE> to) const
				{
				return element == to.element;	// two objects are equal only if the values AND the pointers are the same, and so we can just check the pointers.
				}

			/*
				POINTER_BOX::OPERATOR!=()
				--------------------------
			*/
			/*!
				@brief Compare for not equal to.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator!=(const pointer_box<TYPE> to) const
				{
				return element != to.element;  // two objects are equal only if the value AND the pointers are the same, so if the pointers are different they are not equal.
				}

			/*
				POINTER_BOX::POINTER()
				----------------------
			*/
			/*!
				@brief Return the pointer this box holds.
				@return The boxed pointer
			*/
			TYPE *pointer() const
				{
				return element;
				}
			
			/*
				POINTER_BOX::OPERATOR->()
				-------------------------
			*/
			/*!
				@brief Pointer to member operator.
				@return A pointer to the object
			*/
			TYPE *operator->() const
				{
				return element;
				}

			/*
				POINTER_BOX::OPERATOR*()
				------------------------
			*/
			/*!
				@brief Value of operator.
				@return A reference to the value stored in the object
			*/
			TYPE &operator*() const
				{
				return *element;
				}

			/*
				POINTER_BOX::UNITTEST()
				-----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				int array[] = {6, 3, 6};

				pointer_box<int> a(&array[0]);
				pointer_box<int> b(&array[1]);
				pointer_box<int> c(&array[2]);
				int *ap = &array[0];
				int *bp = &array[1];

				JASS_assert(a.pointer() == &array[0]);

				JASS_assert(b < a);
				JASS_assert(b <= a);
				JASS_assert(a > b);
				JASS_assert(a >= b);
				JASS_assert(c > a);
				JASS_assert(a != b);
				JASS_assert(!(a == b));

				JASS_assert(*ap > *b);
				JASS_assert(*b < *ap);
				JASS_assert(*b <= *ap);
				JASS_assert(*a > *bp);
				JASS_assert(*a >= *bp);
				JASS_assert(*c == *ap);
				JASS_assert(*a != *bp);

				JASS_assert(*a == *ap);

				typedef struct
					{
					int a;
					int b;
					} int_pair;
				int_pair pair {2, 4};

				pointer_box<int_pair> pair_container(&pair);
				JASS_assert(pair_container->a == 2);
				JASS_assert(pair_container->b == 4);
				
				JASS_assert(a.compare(b) > 0);
				JASS_assert(a.compare(c) < 0);
				JASS_assert(a.compare(a) == 0);
				JASS_assert(b.compare(a) < 0);
				JASS_assert(c.compare(a) > 0);

				puts("pointer_box::PASSED");
				}
		};
	}
