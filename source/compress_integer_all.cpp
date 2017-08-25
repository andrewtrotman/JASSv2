/*
	COMPRESS_INTEGER_ALL.CPP
	------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <vector>

#include "compress_integer_all.h"
#include "compress_integer_none.h"
#include "compress_integer_variable_byte.h"
#include "compress_integer_qmx_improved.h"

namespace JASS
	{
	/*
		List of known compressors
	*/
	static compress_integer_none none;
	static compress_integer_qmx_improved qmx_improved;
	static compress_integer_variable_byte variable_byte;

	/*
		Table of known copressors and their command line parameter names and actual names
	*/
	std::array<compress_integer_all::details, compress_integer_all::compressors_size> compress_integer_all::compressors
		{{
		{"-cn", "--compress_none", "None", &none},
		{"-cv", "--compress_vbyte", "Variable Byte", &variable_byte},
		{"-cq", "--compress_qmx_improved", "QMX Improved", &qmx_improved},
		}};
	}
