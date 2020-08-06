/*
	TOP_K_QSORT.H
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Implementation of a top-k quick sort using
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include <utility>

#include "maths.h"
#include "forceinline.h"

namespace JASS
	{
	/*
		CLASS TOP_K_QSORT
		-----------------
	*/
	/*!
		@brief Top-k quick sort, modified from Bentley & McIlroy (1993).
		@details PROGRAM 7 from:
			J.L. Bentley, M.D. McIlroy (1993), Engineering a Sort Function, Software-Practice and Experience, 23(11):1249-1265
		modified to return just the top-k elements (where k can equal n).
	*/
	class top_k_qsort
		{
		private:
			/*
				TOP_K_SORT::MED3()
				------------------
			*/
			/*!
				@brief return the median of 3 elements
				@param a [in] pointer to one of the values
				@param b [in] pointer to one of the values
				@param c [in] pointer to one of the values
				@return pointer to the median element
			*/
			template <typename TYPE>
			static forceinline TYPE *med3(TYPE &a, TYPE &b, TYPE &c)
				{
				return a < b ?
					(b < c ? &b : a < c ? &c : &a) :
					(b > c ? &b : a > c ? &c : &a);
				}

			/*
				TOP_K_SORT::SWAPFUNC()
				----------------------
			*/
			/*!
				@brief swap n elements from a with n elements from b
				@param a [in] pointer to the start of the range to swap
				@param b [in] pointer to the start of the tange to swap with
				@param n [in] the number of consecutive elements to swap
			*/
			template <typename TYPE>
			static forceinline void swapfunc(TYPE *a, TYPE *b, size_t n)
				{
				while (n-- > 0)
					{
					std::swap(*a, *b);
					a++;
					b++;
					}
				}
		public:
			/*
				TOP_K_SORT::SORT()
				------------------
			*/
			/*!
				@brief Perform a top-k sort.  At then k elements are in order and the other are shuffled
				@param a [in] a pointer to an array to sort
				@param n [in] The number of elements in the array to sort
				@param top_k [in] Sort so that the top-k elements are sorted
			*/
			template <typename TYPE>
			static void sort(TYPE *a, size_t n, size_t top_k)
				{
				TYPE *pa, *pb, *pc, *pd, *pl, *pm, *pn;
				TYPE pv;
				size_t s;

				if (n < 7)
					{ /* Insertion sort on smallest arrays */
					for (pm = a + 1; pm < a + n; pm++)
						for (pl = pm; pl > a && *(pl - 1) > *pl; pl--)
							std::swap(*pl, *(pl - 1));
					return;
					}
				pm = a + (n / 2); /* Small arrays, middle element */
				if (n > 7)
					{
					pl = a;
					pn = a + (n - 1);
					if (n > 40)
						{ /* Big arrays, pseudomedian of 9 */
						s = (n / 8);
						pl = med3(*pl, *(pl + s), *(pl + 2 * s));
						pm = med3(*(pm - s), *pm, *(pm + s));
						pn = med3(*(pn - 2 * s), *(pn - s), *pn);
						}
					pm = med3(*pl, *pm, *pn); /* Mid-size, med of 3 */
					}

				pv = *pm;		/* pv is the partition value */

				pa = pb = a;
				pc = pd = a + (n - 1);
				for (;;)
					{
					while (pb <= pc && *pb <= pv)
						{
						if (*pb == pv)
							{
							std::swap(*pa, *pb);
							pa++;
							}
						pb++;
						}
					while (pc >= pb && *pc >= pv)
						{
						if (*pc == pv)
							{
							std::swap(*pc, *pd);
							pd--;
							}
						pc--;
						}
					if (pb > pc)
						break;
					std::swap(*pb, *pc);
					pb++;
					pc--;
					}
				pn = a + n;

				s = maths::minimum(pa - a, pb - pa);
				if (s > 0)
					swapfunc(a, pb - s, s);

				s =  maths::minimum(pd - pc, pn - pd - 1);
				if (s > 0)
					swapfunc(pb, pn - s, s);

				s = pb - pa;
				if (s > 1)
					sort(a, s, top_k);

				if (s < top_k)
					{
					size_t s2;
					if ((s2 = pd - pc) > 1)
						sort(pn - s2, s2, top_k - s);
					}
				}

			/*
				TOP_K_SORT::SORT()
				------------------
			*/
			/*!
				@brief Quicksort elements in an array
				@param a [in] a pointer to an array to sort
				@param n [in] The number of elements in the array to sort
				@param cmp [in] the comparison functor
			*/
			template <typename TYPE>
			static forceinline void sort(TYPE *a, size_t n)
				{
				sort(a, n, n);
				}

			/*
				TOP_K_HEAP::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
			};
	}
