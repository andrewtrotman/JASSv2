/*
	POINTER_CONTAINER.H
	-------------------
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
	template <typename POINTER>
	class pointer_container
		{
		private:
			POINTER &element;

		public:
			pointer_container(POINTER &to) :
				element(to)
				{
				/* Nothing */
				}

			bool operator<(const pointer_container<POINTER> &to) const
				{
				return element < to.element;
				}

			bool operator<(const POINTER &to) const
				{
				return element < to;
				}

			bool operator<=(const pointer_container<POINTER> &to) const
				{
				return element <= to.element;
				}

			bool operator<=(const POINTER &to) const
				{
				return element <= to;
				}


			bool operator>(const pointer_container<POINTER> &to) const
				{
				return element > to.element;
				}

			bool operator>(const POINTER &to) const
				{
				return element > to;
				}

			bool operator>=(const pointer_container<POINTER> &to) const
				{
				return element >= to.element;
				}

			bool operator>=(const POINTER &to) const
				{
				return element >= to;
				}



			bool operator==(const pointer_container<POINTER> &to) const
				{
				return element == to.element;
				}

			bool operator==(const POINTER &to) const
				{
				return element == to;
				}

			bool operator!=(const pointer_container<POINTER> &to) const
				{
				return element != to.element;
				}

			bool operator!=(const POINTER &to) const
				{
				return element != to;
				}

			POINTER *operator->() const
				{
				return &element;
				}

			POINTER &operator*() const
				{
				return element;
				}



			static void unittest(void)
				{
				int array[] = {6, 3, 6};

				pointer_container<int> a(array[0]);
				pointer_container<int> b(array[1]);
				pointer_container<int> c(array[2]);
				int *ap = &array[0];
				int *bp = &array[1];

				JASS_assert(b < a);
				JASS_assert(b <= a);
				JASS_assert(a > b);
				JASS_assert(a >= b);
				JASS_assert(c == a);
				JASS_assert(a != b);

				JASS_assert(b < *ap);
				JASS_assert(b <= *ap);
				JASS_assert(a > *bp);
				JASS_assert(a >= *bp);
				JASS_assert(c == *ap);
				JASS_assert(a != *bp);

				JASS_assert(*a == *ap);

				typedef struct
					{
					int a;
					int b;
					} int_pair;
				int_pair pair {2, 4};

				pointer_container<int_pair> pair_container(pair);
				JASS_assert(pair_container->a == 2);
				JASS_assert(pair_container->b == 4);

				puts("pointer_container::PASS");
				}
		};
	}
