/*
	MATHS.CPP
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "maths.h"

namespace JASS
	{
	/*
		MATHS::UNITTEST()
		-----------------
	*/
	void maths::unittest(void)
		{
		JASS_assert(maximum(2, 1) == 2);
		JASS_assert(maximum(1, 2) == 2);
		
		JASS_assert(maximum(1, 2, 3) == 3);
		JASS_assert(maximum(2, 1, 3) == 3);
		JASS_assert(maximum(1, 3, 2) == 3);
		JASS_assert(maximum(2, 3, 1) == 3);
		JASS_assert(maximum(3, 1, 2) == 3);
		JASS_assert(maximum(3, 2, 1) == 3);

		JASS_assert(minimum(2, 1) == 1);
		JASS_assert(minimum(1, 2) == 1);
		
		JASS_assert(minimum(1, 2, 3) == 1);
		JASS_assert(minimum(2, 1, 3) == 1);
		JASS_assert(minimum(1, 3, 2) == 1);
		JASS_assert(minimum(2, 3, 1) == 1);
		JASS_assert(minimum(3, 1, 2) == 1);
		JASS_assert(minimum(3, 2, 1) == 1);

		JASS_assert(floor_log2(10) == 3);
		JASS_assert(ceiling_log2(10) == 4);
		
		auto got = find_first_set_bit((uint32_t)0b0101000);
		JASS_assert(got == 3);
 		JASS_assert(find_first_set_bit((uint64_t)0b0101001) == 0);

		JASS_assert(find_first_set_bit((uint32_t)0) == 0);
 		JASS_assert(find_first_set_bit((uint64_t)0) == 0);

		puts("maths::PASSED");
		}
	}
