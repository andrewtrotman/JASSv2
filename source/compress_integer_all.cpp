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

namespace JASS
	{
	/*
		List of known compressors
	*/
	static compress_integer_none none;											///< identity compressor
#ifdef NEVER
	static compress_integer_carry_8b carry_8b;								///< Carry-8b compressor
	static compress_integer_simple_9 simple_9;								///< Simple-9 compressor
	static compress_integer_simple_8b simple_8b;								///< Simple-8b compressor
	static compress_integer_simple_16 simple_16;								///< Simple-16 compressor
	static compress_integer_bitpack_64 bitpack_64;							///< fixed width bin-packed into 64-bit integers
	static compress_integer_elias_gamma elias_gamma;						///< Elias gamma using bit-extract instructions
	static compress_integer_elias_delta elias_delta;						///< Elias gamma using bit-extract instructions
#endif
	static compress_integer_qmx_jass_v1 qmx_jass_v1;						///< QMX compressor compatile with JASS v1 (do not use)
#ifdef NEVER
	static compress_integer_bitpack_128 bitpack_128;						///< fixed width bin-packed into 128-bit integers
	static compress_integer_bitpack_256 bitpack_256;						///< fixed width bin-packed into 256-bit integers
	static compress_integer_relative_10 relative_10;						///< Relative-10 compressor
	static compress_integer_carryover_12 carryover_12;						///< Carryover-12 compressor
	static compress_integer_qmx_original qmx_original;						///< QMX compressor
	static compress_integer_qmx_improved qmx_improved;						///< Improved QMX compressor
	static compress_integer_stream_vbyte stream_vbyte;						///< Stream VByte compressor
	static compress_integer_variable_byte variable_byte;					///< Variable Byte compressor
	static compress_integer_simple_9_packed simple_9_packed;				///< Packed Simple-9 compressor
	static compress_integer_elias_delta_simd elias_delta_simd;			///< Elias delta SIMD
	static compress_integer_simple_16_packed simple_16_packed;			///< Packed Simple-16 compressor
	static compress_integer_simple_8b_packed simple_8b_packed;			///< Packed Simple-8b compressor
#endif
	static compress_integer_elias_gamma_simd elias_gamma_simd;			///< elias gamma packed into SIMD regisers
#ifdef NEVER
	static compress_integer_bitpack_32_reduced bitpack_32_reduced;		///< fixed width bin-packed into 32-bit integers
	static compress_integer_elias_gamma_bitwise elias_gamma_bitwise;	///< Elias gamma done with bit-wise instructions (slow)
	static compress_integer_elias_delta_bitwise elias_delta_bitwise;	///< Elias delta done with bit-wise instructions (slow)
#endif

	/*!
		@brief Table of known compressors and their command line parameter names and actual names
	*/
	std::array<compress_integer_all::details, compress_integer_all::compressors_size> compress_integer_all::compressors
		{
			{
#ifdef NEVER
			{"-cc",    "--compress_carryover_12", "Carryover-12", &carryover_12},
			{"-cC",    "--compress_carry_8b", "Carry-8b", &carry_8b},
			{"-cd",    "--compress_elias_delta", "Elias delta", &elias_delta},
			{"-cD",    "--compress_elias_delta_bitwise", "Elias delta with bit instuctions (slow)", &elias_delta_bitwise},
			{"-ce",    "--compress_elias_delta_SIMD", "Group Elias Delta SIMD", &elias_delta_simd},
#endif
			{"-cE",    "--compress_elias_gamma_SIMD", "Group Elias Gamma SIMD", &elias_gamma_simd},
#ifdef NEVER
			{"-cg",    "--compress_elias_gamma", "Elias gamma", &elias_gamma},
			{"-cG",    "--compress_elias_gamma_bitwise", "Elias gamma with bit instuctions (slow)", &elias_gamma_bitwise},
#endif
			{"-cn",    "--compress_none", "None", &none},
#ifdef NEVER
			{"-cp",    "--compress_simple_9_packed", "Optimal Packed Simple-9", &simple_9_packed},
			{"-cq",    "--compress_simple_16_packed", "Optimal Packed Simple-16", &simple_16_packed},
			{"-cQ",    "--compress_simple_8b_packed", "Optimal Packed Simple-8b", &simple_8b_packed},
			{"-cr",    "--compress_relative_10", "Relative-10", &relative_10},
			{"-cs",    "--compress_simple_9", "Simple-9", &simple_9},
			{"-ct",    "--compress_simple_16", "Simple-16", &simple_16},
			{"-cT",    "--compress_simple_8b", "Simple-8b", &simple_8b},
			{"-cv",    "--compress_vbyte", "Variable Byte", &variable_byte},
			{"-cV",    "--compress_stream_vbyte", "Stream VByte", &stream_vbyte},
			{"-cX",    "--compress_qmx_improved", "QMX Improved", &qmx_improved},
			{"-cx",    "--compress_qmx_original", "QMX Original", &qmx_original},
#endif
			{"-cZ",    "--compress_qmx_jass_v1", "QMX JASS v1", &qmx_jass_v1},
#ifdef NEVER
			{"-c128",  "--compress_128", "Binpack into 128-bit SIMD integers", &bitpack_128},
			{"-c256",  "--compress_256", "Binpack into 256-bit SIMD integers", &bitpack_256},
			{"-c32r",  "--compress_32", "Binpack into 32-bit integers with 8 selectors", &bitpack_32_reduced},
			{"-c64",   "--compress_64", "Binpack into 64-bit integers", &bitpack_64},
#endif
			}
		};

	/*
		COMPRESS_INTEGER_ALL::REPLICATE()
		---------------------------------
	*/
	std::unique_ptr<compress_integer>compress_integer_all::replicate(compress_integer *codex)
		{
		if (codex == &none)
			return std::make_unique<compress_integer_none>();
	#ifdef NEVER
		if (codex == &carry_8b)
			return std::make_unique<compress_integer_carry_8b>();
		if (codex == &simple_9)
			return std::make_unique<compress_integer_simple_9>();
		if (codex == &simple_8b)
			return std::make_unique<compress_integer_simple_8b>();
		if (codex == &simple_16)
			return std::make_unique<compress_integer_simple_16>();
		if (codex == &bitpack_64)
			return std::make_unique<compress_integer_bitpack_64>();
		if (codex == &elias_gamma)
			return std::make_unique<compress_integer_elias_gamma>();
		if (codex == &elias_delta)
			return std::make_unique<compress_integer_elias_delta>();
	#endif
		if (codex == &qmx_jass_v1)
			return std::make_unique<compress_integer_qmx_jass_v1>();
	#ifdef NEVER
		if (codex == &bitpack_128)
			return std::make_unique<compress_integer_bitpack_128>();
		if (codex == &bitpack_256)
			return std::make_unique<compress_integer_bitpack_256>();
		if (codex == &relative_10)
			return std::make_unique<compress_integer_relative_10>();
		if (codex == &carryover_12)
			return std::make_unique<compress_integer_carryover_12>();
		if (codex == &qmx_original)
			return std::make_unique<compress_integer_qmx_original>();
		if (codex == &qmx_improved)
			return std::make_unique<compress_integer_qmx_improved>();
		if (codex == &stream_vbyte)
			return std::make_unique<compress_integer_stream_vbyte>();
		if (codex == &variable_byte)
			return std::make_unique<compress_integer_variable_byte>();
		if (codex == &simple_9_packed)
			return std::make_unique<compress_integer_simple_9_packed>();
		if (codex == &elias_delta_simd)
			return std::make_unique<compress_integer_elias_delta_simd>();
		if (codex == &simple_16_packed)
			return std::make_unique<compress_integer_simple_16_packed>();
		if (codex == &simple_8b_packed)
			return std::make_unique<compress_integer_simple_8b_packed>();
	#endif
		if (codex == &elias_gamma_simd)
			return std::make_unique<compress_integer_elias_gamma_simd>();
	#ifdef NEVER
		if (codex == &bitpack_32_reduced)
			return std::make_unique<compress_integer_bitpack_32_reduced>();
		if (codex == &elias_gamma_bitwise)
			return std::make_unique<compress_integer_elias_gamma_bitwise>();
		if (codex == &elias_delta_bitwise)
			return std::make_unique<compress_integer_elias_delta_bitwise>();
	#endif

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
		JASS_assert(parameters[0] == true);
		JASS_assert(name(parameters) == compressors[0].description);
//		JASS_assert(&compressor(parameters) == compressors[0].codex);

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
//		JASS_assert(&compressor(parameters) == compressors[default_compressor].codex);

		puts("compress_integer_all::PASSED");
		}
	}
