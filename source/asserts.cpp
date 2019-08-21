/*
	ASSERT.CPP
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>
#include <stdlib.h>

#include "asserts.h"

namespace JASS
	{
	/*
		FAIL()
		------
	*/
	void fail(const char *function, const char *file, size_t line, const char *expression)
		{
		fprintf(stderr, "%s:%llu:%s: JASS_assertion \"%s\" failed", file, (unsigned long long)line, function, expression);
		abort();
		}
	}
