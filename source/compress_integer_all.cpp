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
#include "compress_integer_qmx_original.h"

namespace JASS
	{
	/*
		List of known compressors
	*/
	static compress_integer_none none;
	static compress_integer_qmx_original qmx_original;
	static compress_integer_qmx_improved qmx_improved;
	static compress_integer_variable_byte variable_byte;

	/*
		Table of known copressors and their command line parameter names and actual names
	*/
	std::array<compress_integer_all::details, compress_integer_all::compressors_size> compress_integer_all::compressors
		{
			{
			{"-cn", "--compress_none", "None", &none},
			{"-cv", "--compress_vbyte", "Variable Byte", &variable_byte},
			{"-cq", "--compress_qmx_improved", "QMX Improved", &qmx_improved},
			{"-cQ", "--compress_qmx_original", "QMX Original with bug fixes (always large and slower than QMX improved)", &qmx_original},
			}
		};

	/*
		COMPRESS_INTEGER_ALL::UNITTEST()
		--------------------------------
	*/
	void compress_integer_all::unittest(void)
		{
		/*
			Allocate an array of selectors and get a parameter list
		*/
		std::array<bool, compressors_size> parameters = {};
		auto parameter_list = parameterlist(parameters);

		/*
			Fake argc and argv[]
		*/
		const char *argv[] = {"program", "-cv"};
		
		/*
			Call the command line parser to get the selected option
		*/
		std::string errors;
		auto success = commandline::parse(2, argv, parameter_list, errors);
		
		/*
			Make sure we succeeded
		*/
		JASS_assert(success == true);
		
		/*
			Check that only one parameter was selected
		*/
		size_t parameters_selected = 0;
		for (const auto param : parameters)
			if (param)
				parameters_selected++;
		JASS_assert(parameters_selected == 1);
		
		/*
			Make sure we got the correct parameter selected
		*/
		JASS_assert(parameters[1] == true);
		JASS_assert(name(parameters) == compressors[1].description);
		JASS_assert(&compressor(parameters) == compressors[1].codex);

		/*
			Check what happens if we don't have any parameters.
		*/
		parameters = {};
		const char *argv0[] = {"program"};
		success = commandline::parse(1, argv0, parameter_list, errors);
		JASS_assert(success == true);
		parameters_selected = 0;
		for (const auto param : parameters)
			if (param)
				parameters_selected++;					// LCOV_EXCL_LINE		// if the unit test is successful then this should not be called.
		JASS_assert(parameters_selected == 0);
		JASS_assert(name(parameters) == "None");
		JASS_assert(&compressor(parameters) == compressors[default_compressor].codex);

		puts("compress_integer_all::PASSED");
		}
	}
