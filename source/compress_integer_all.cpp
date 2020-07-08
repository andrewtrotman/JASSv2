/*
	COMPRESS_INTEGER_ALL.CPP
	------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <vector>

#include "compress_integer_all.h"
#include "compress_integer_none.h"
#include "compress_integer_carry_8b.h"
#include "compress_integer_simple_9.h"
#include "compress_integer_simple_8b.h"
#include "compress_integer_simple_16.h"
#include "compress_integer_bitpack_64.h"
#include "compress_integer_elias_gamma.h"
#include "compress_integer_elias_delta.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_qmx_jass_v1.h"
#include "compress_integer_relative_10.h"
#include "compress_integer_carryover_12.h"
#include "compress_integer_variable_byte.h"
#include "compress_integer_qmx_improved.h"
#include "compress_integer_qmx_original.h"
#include "compress_integer_stream_vbyte.h"
#include "compress_integer_simple_9_packed.h"
#include "compress_integer_elias_delta_simd.h"
#include "compress_integer_simple_16_packed.h"
#include "compress_integer_simple_8b_packed.h"
#include "compress_integer_elias_gamma_simd.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_elias_gamma_bitwise.h"
#include "compress_integer_elias_delta_bitwise.h"
#include "compress_integer_elias_gamma_simd_vb.h"

namespace JASS
	{
	/*!
		@brief Table of known compressors and their command line parameter names and actual names
	*/
	std::array<compress_integer_all::details, compress_integer_all::compressors_size> compress_integer_all::compressors
		{
			{
			{"-cc",    "--compress_carryover_12", "Carryover-12"},
			{"-cC",    "--compress_carry_8b", "Carry-8b"},
			{"-cd",    "--compress_elias_delta", "Elias delta"},
			{"-cD",    "--compress_elias_delta_bitwise", "Elias delta with bit instuctions (slow)"},
			{"-ce",    "--compress_elias_delta_SIMD", "Group Elias Delta SIMD"},
			{"-cE",    "--compress_elias_gamma_SIMD", "Group Elias Gamma SIMD"},
			{"-cF",    "--compress_elias_gamma_SIMD_vb", "Group Elias Delta SIMD with Variable Byte"},
			{"-cg",    "--compress_elias_gamma", "Elias gamma"},
			{"-cG",    "--compress_elias_gamma_bitwise", "Elias gamma with bit instuctions (slow)"},
			{"-cn",    "--compress_none", "None"},
			{"-cp",    "--compress_simple_9_packed", "Optimal Packed Simple-9"},
			{"-cq",    "--compress_simple_16_packed", "Optimal Packed Simple-16"},
			{"-cQ",    "--compress_simple_8b_packed", "Optimal Packed Simple-8b"},
			{"-cr",    "--compress_relative_10", "Relative-10"},
			{"-cs",    "--compress_simple_9", "Simple-9"},
			{"-ct",    "--compress_simple_16", "Simple-16"},
			{"-cT",    "--compress_simple_8b", "Simple-8b"},
			{"-cv",    "--compress_vbyte", "Variable Byte"},
			{"-cV",    "--compress_stream_vbyte", "Stream VByte"},
			{"-cX",    "--compress_qmx_improved", "QMX Improved"},
			{"-cx",    "--compress_qmx_original", "QMX Original"},
			{"-cZ",    "--compress_qmx_jass_v1", "QMX JASS v1"},
			{"-c128",  "--compress_128", "Binpack into 128-bit SIMD integers"},
			{"-c256",  "--compress_256", "Binpack into 256-bit SIMD integers"},
			{"-c32r",  "--compress_32", "Binpack into 32-bit integers with 8 selectors"},
			{"-c64",   "--compress_64", "Binpack into 64-bit integers"},
			}
		};

	/*
		COMPRESS_INTEGER_ALL::REPLICATE()
		---------------------------------
	*/
	std::unique_ptr<compress_integer>compress_integer_all::replicate(std::string &shortname)
		{
		/*
			Put the most likley ones first.
		*/
		if (shortname == "-cE")
			return std::make_unique<compress_integer_elias_gamma_simd>();
		if (shortname == "-cF")
			return std::make_unique<compress_integer_elias_gamma_simd_vb>();
		if (shortname == "-cZ")
			return std::make_unique<compress_integer_qmx_jass_v1>();
		if (shortname == "-cn")
			return std::make_unique<compress_integer_none>();

		/*
			Now the least likley ones.
		*/
		if (shortname == "-cC")
			return std::make_unique<compress_integer_carry_8b>();
		if (shortname == "-cs")
			return std::make_unique<compress_integer_simple_9>();
		if (shortname == "-cT")
			return std::make_unique<compress_integer_simple_8b>();
		if (shortname == "-ct")
			return std::make_unique<compress_integer_simple_16>();
		if (shortname == "-c64")
			return std::make_unique<compress_integer_bitpack_64>();
		if (shortname == "-cg")
			return std::make_unique<compress_integer_elias_gamma>();
		if (shortname == "-cd")
			return std::make_unique<compress_integer_elias_delta>();
		if (shortname == "-c128")
			return std::make_unique<compress_integer_bitpack_128>();
		if (shortname == "-c256")
			return std::make_unique<compress_integer_bitpack_256>();
		if (shortname == "-cr")
			return std::make_unique<compress_integer_relative_10>();
		if (shortname == "-cc")
			return std::make_unique<compress_integer_carryover_12>();
		if (shortname == "-cx")
			return std::make_unique<compress_integer_qmx_original>();
		if (shortname == "-cX")
			return std::make_unique<compress_integer_qmx_improved>();
		if (shortname == "-cV")
			return std::make_unique<compress_integer_stream_vbyte>();
		if (shortname == "-cv")
			return std::make_unique<compress_integer_variable_byte>();
		if (shortname == "-cp")
			return std::make_unique<compress_integer_simple_9_packed>();
		if (shortname == "-ce")
			return std::make_unique<compress_integer_elias_delta_simd>();
		if (shortname == "-cq")
			return std::make_unique<compress_integer_simple_16_packed>();
		if (shortname == "-cQ")
			return std::make_unique<compress_integer_simple_8b_packed>();
		if (shortname == "-c32r")
			return std::make_unique<compress_integer_bitpack_32_reduced>();
		if (shortname == "-cG")
			return std::make_unique<compress_integer_elias_gamma_bitwise>();
		if (shortname == "-cD")
			return std::make_unique<compress_integer_elias_delta_bitwise>();

		assert(0);	// Unknown compressor;
		return nullptr;
		}


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
		const char *argv[] = {"program", "-cE"};
		
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
		JASS_assert(parameters[5] == true);
		JASS_assert(name(parameters) == compressors[5].description);

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
		JASS_assert(name(parameters) == compressors[default_compressor].description);

		puts("compress_integer_all::PASSED");
		}
	}
