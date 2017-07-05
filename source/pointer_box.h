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
		those pointers point to).
*/
	template <typename POINTER>
	class pointer_box
		{
		private:
			POINTER *element;				///< We actually store a reference to the object rather than a pointer, but they're the same size.
			
		public:
			/*
				POINTER_BOX::POINTER_BOX()
				--------------------------
			*/
			/*!
				@brief Constructor
				@param to [in] The object we are being a pointer to.
			*/
			pointer_box(POINTER *to) :
				element(to)
				{
				/* Nothing */
				}

			/*
				POINTER_BOX::OPERATOR<()
				------------------------
			*/
			/*!
				@brief Compare for less than.
				@param to [in] The object we are Compareing to.
				@return true or false.
			*/
			bool operator<(const pointer_box<POINTER> &to) const
				{
				return *element < *to.element;
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
			bool operator<=(const pointer_box<POINTER> &to) const
				{
				return *element <= *to.element;
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
			bool operator>(const pointer_box<POINTER> &to) const
				{
				return *element > *to.element;
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
			bool operator>=(const pointer_box<POINTER> &to) const
				{
				return *element >= *to.element;
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
			bool operator==(const pointer_box<POINTER> &to) const
				{
				return *element == *to.element;
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
			bool operator!=(const pointer_box<POINTER> &to) const
				{
				return *element != *to.element;
				}

			/*
				POINTER_BOX::OPERATOR->()
				-------------------------
			*/
			/*!
				@brief Pointer to member operator.
				@return A pointer to the object
			*/
			POINTER *operator->() const
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
			POINTER &operator*() const
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

				JASS_assert(b < a);
				JASS_assert(b <= a);
				JASS_assert(a > b);
				JASS_assert(a >= b);
				JASS_assert(c == a);
				JASS_assert(a != b);

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

				puts("pointer_box::PASS");
				}
		};
	}
