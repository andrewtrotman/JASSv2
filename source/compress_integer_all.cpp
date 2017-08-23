/*
	COMPRESS_INTEGER_ALL.CPP
	------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "compress_integer_all.h"
#include "compress_integer_variable_byte.h"
#include "compress_integer_none.h"

namespace JASS
	{
	static compress_integer_variable_byte variable_byte;
	static compress_integer_none none;

	compress_integer_all compress_integer_all_compressors[] =
		{
		{"-cn", "--compress_none", "Do not compress", &none},
		{"-cv", "--compress_vbyte", "Compress using variable byte", &variable_byte},
		};
	}
