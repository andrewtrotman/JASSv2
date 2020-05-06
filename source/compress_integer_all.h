/*
	COMPRESS_INTEGER_ALL.H
	----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A container holding all the integer compression schemes known by JASS.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <array>
#include <tuple>
#include <string>

#include "commandline.h"
#include "compress_integer.h"
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
		CLASS COMPRESS_INTEGER_ALL
		--------------------------
	*/
	/*!
		@brief A container holding all the integer compression schemes known by JASS
		@details Add a new compressor by creating an instance of it in compress_integer_all.cpp, then creating
		an new row in the table compressors, then incr
	*/
	class compress_integer_all
		{
		public:
			static constexpr size_t compressors_size = 25;					///< There are currently this many compressors known to JASS
			static constexpr size_t default_compressor = 8;					///< The default one to use is at this position in the compressors array

		private:
			/*
				CLASS COMPRESS_INTEGER_ALL::DETAILS
				-----------------------------------
			*/
			/*!
				Each compressor is represented by the command line details and an instance of that compressor
			*/
			class details
				{
				public:
					const char *shortname;					///< The short command line parameter.
					const char *longname;					///< The long command line parameter.
					const char *description;				///< The name of the scheme, in command line use other stuff is wrapped around this.
				};

		private:
			static std::array<details, compressors_size> compressors;			///< The array of known compressor schemes
			
		private:
			/*
				COMPRESS_INTEGER_ALL::MAKE_COMMANDLINE()
				----------------------------------------
			*/
			/*!
				@brief Turn the array into a tupple for use with JASS::commandline
				@param option [in] The boolean array used to represent whether or not the parameter has been selected on the command line
				@return A tuple of this scheme and all after it in the compressors array.
			*/
			template<std::size_t I, typename std::enable_if<I == 0, size_t>::type = I>
			static auto make_commandline(std::array<bool, compressors_size> &option)
				{
				return std::make_tuple(commandline::parameter(compress_integer_all::compressors[I].shortname, compress_integer_all::compressors[I].longname, compress_integer_all::compressors[I].description, option[I]));
				}

			/*
				COMPRESS_INTEGER_ALL::MAKE_COMMANDLINE()
				----------------------------------------
			*/
			/*!
				@brief Turn the array into a tupple for use with JASS::commandline
				@param option [in] The boolean array used to represent whether or not the parameter has been selected on the command line
				@return A tuple of this scheme and all after it in the compressors array.
			*/
			template<std::size_t I, typename std::enable_if<I != 0, size_t>::type = I>
			static auto make_commandline(std::array<bool, compressors_size> &option)
				{
				return std::tuple_cat(make_commandline<I - 1>(option), std::make_tuple(commandline::parameter(compress_integer_all::compressors[I].shortname, compress_integer_all::compressors[I].longname, compress_integer_all::compressors[I].description, option[I])));
				}
		
		public:
			/*
				COMPRESS_INTEGER_ALL::PARAMETERLIST()
				-------------------------------------
			*/
			/*!
				@brief Get a std::tuple of the compressors ready for use with JASS::commandline.
				@param option [in] An array (one per compressor, each pre-set to false) that will be set to true when the user selects a compressor.
				@return A std::tuple for use with JASS::commandline.
			*/
			static auto parameterlist(std::array<bool, compressors_size> &option)
				{
				return make_commandline<compressors_size - 1>(option);
				}
			
			/*
				COMPRESS_INTEGER_ALL::COMPRESSOR()
				----------------------------------
			*/
			/*!
				@brief Turn the first selected compressor (according to option) into a compressor
				@param option [in] An array (one per compressor) with (preferably) one set to true.
				@return An integer compressor, or the default integer compressor if each member of option is set to false.
			*/
			static compress_integer_t &compressor(const std::array<bool, compressors_size> &option)
				{
				return get_by_name(name(option));
				}
			
			/*
				COMPRESS_INTEGER_ALL::NAME()
				----------------------------
			*/
			/*!
				@brief Get the name of the first selected compressor (according to option).
				@param option [in] An array (one per compressor) with (preferably) one set to true.
				@return The name of the first selected compressor.
			*/
			static const std::string name(const std::array<bool, compressors_size> &option)
				{
				for (size_t which = 0; which < compressors_size; which++)
					if (option[which])
						return compressors[which].description;
					
				return compressors[default_compressor].description;
				}

			/*
				COMPRESS_INTEGER_ALL::GET_BY_NAME()
				-----------------------------------
			*/
			/*!
				@brief Given the name of a compressor, return a reference to an object that is that kind of compressor.
				@param name [in] The name of the compressor.
				@return A reference to a compressor that can encode and decode data using the codex of the given name (or a "null" compressor on error)
			*/
			static compress_integer_t &get_by_name(const std::string &name)
				{
				std::vector<std::string> vec;
				return get<uint16_t, 1, 1>(name, vec, 1, 1);
				}

			/*
				COMPRESS_INTEGER_ALL::GET_BY_NAME()
				-----------------------------------
			*/
			template <typename ACCUMULATOR_TYPE, size_t MAX_DOCUMENTS, size_t MAX_TOP_K>
			static compress_integer<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> &get(const std::string &name, const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10)
				{
				if (name == "Carryover-12") return new compress_integer_carryover_12<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
#ifdef NEVER
				else if (name == "Carry-8b") return new compress_integer_carry_8b<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Elias delta") return new compress_integer_elias_delta<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Elias delta with bit instuctions (slow)") return new compress_integer_elias_delta_bitwise<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Group Elias Delta SIMD") return new compress_integer_elias_delta_simd<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Group Elias Gamma SIMD") return new compress_integer_elias_gamma_simd<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Elias gamma") return new compress_integer_elias_gamma<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Elias gamma with bit instuctions (slow)") return new compress_integer_elias_gamma_bitwise<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "None") return new compress_integer_none<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Optimal Packed Simple-9") return new compress_integer_simple_9_packed<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Optimal Packed Simple-16") return new compress_integer_simple_16_packed<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Optimal Packed Simple-8b") return new compress_integer_simple_8b_packed<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Relative-10") return new compress_integer_relative_10<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Simple-9") return new compress_integer_simple_9<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Simple-16") return new compress_integer_simple_16<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Simple-8b") return new compress_integer_simple_8b<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Variable Byte") return new compress_integer_variable_byte<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Stream VByte") return new compress_integer_stream_vbyte<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "QMX Improved") return new compress_integer_qmx_improved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "QMX Original") return new compress_integer_qmx_original<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "QMX JASS v1") return new compress_integer_qmx_jass_v1<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Binpack into 128-bit SIMD integers") return new compress_integer_bitpack_128<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Binpack into 256-bit SIMD integers") return new compress_integer_bitpack_256<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Binpack into 32-bit integers with 8 selectors") return new compress_integer_bitpack_32_reduced<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				else if (name == "Binpack into 64-bit integers") return new compress_integer_bitpack_64<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
#endif
				else
					return new compress_integer_none<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K>(primary_keys, documents, top_k);
				}



			/*
				COMPRESS_INTEGER_ALL::UNITTEST()
				--------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}



#ifdef NEVER
/*
	List of known compressors
*/
static compress_integer_none<uint16_t, 1, 1> none;											///< identity compressor
static compress_integer_carry_8b<uint16_t, 1, 1> carry_8b;								///< Carry-8b compressor
static compress_integer_simple_9<uint16_t, 1, 1> simple_9;								///< Simple-9 compressor
static compress_integer_simple_8b<uint16_t, 1, 1> simple_8b;								///< Simple-8b compressor
static compress_integer_simple_16<uint16_t, 1, 1> simple_16;								///< Simple-16 compressor
static compress_integer_bitpack_64<uint16_t, 1, 1> bitpack_64;							///< fixed width bin-packed into 64-bit integers
static compress_integer_elias_gamma<uint16_t, 1, 1> elias_gamma;						///< Elias gamma using bit-extract instructions
static compress_integer_elias_delta<uint16_t, 1, 1> elias_delta;						///< Elias gamma using bit-extract instructions
static compress_integer_qmx_jass_v1<uint16_t, 1, 1> qmx_jass_v1;						///< QMX compressor compatile with JASS v1 (do not use)
static compress_integer_bitpack_128<uint16_t, 1, 1> bitpack_128;						///< fixed width bin-packed into 128-bit integers
static compress_integer_bitpack_256<uint16_t, 1, 1> bitpack_256;						///< fixed width bin-packed into 256-bit integers
static compress_integer_relative_10<uint16_t, 1, 1> relative_10;						///< Relative-10 compressor
static compress_integer_carryover_12<uint16_t, 1, 1> carryover_12;						///< Carryover-12 compressor
static compress_integer_qmx_original<uint16_t, 1, 1> qmx_original;						///< QMX compressor
static compress_integer_qmx_improved<uint16_t, 1, 1> qmx_improved;						///< Improved QMX compressor
static compress_integer_stream_vbyte<uint16_t, 1, 1> stream_vbyte;						///< Stream VByte compressor
static compress_integer_variable_byte<uint16_t, 1, 1> variable_byte;					///< Variable Byte compressor
static compress_integer_simple_9_packed<uint16_t, 1, 1> simple_9_packed;				///< Packed Simple-9 compressor
static compress_integer_elias_delta_simd<uint16_t, 1, 1> elias_delta_simd;			///< Elias delta SIMD
static compress_integer_simple_16_packed<uint16_t, 1, 1> simple_16_packed;			///< Packed Simple-16 compressor
static compress_integer_simple_8b_packed<uint16_t, 1, 1> simple_8b_packed;			///< Packed Simple-8b compressor
static compress_integer_elias_gamma_simd<uint16_t, 1, 1> elias_gamma_simd;			///< elias gamma packed into SIMD regisers
static compress_integer_bitpack_32_reduced<uint16_t, 1, 1> bitpack_32_reduced;		///< fixed width bin-packed into 32-bit integers
static compress_integer_elias_gamma_bitwise<uint16_t, 1, 1> elias_gamma_bitwise;	///< Elias gamma done with bit-wise instructions (slow)
static compress_integer_elias_delta_bitwise<uint16_t, 1, 1> elias_delta_bitwise;	///< Elias delta done with bit-wise instructions (slow)

/*!
	@brief Table of known compressors and their command line parameter names and actual names
*/
std::array<compress_integer_all::details, compress_integer_all::compressors_size> compress_integer_all::compressors
	{
		{
		{"-cc",    "--compress_carryover_12", "Carryover-12", &carryover_12},
		{"-cC",    "--compress_carry_8b", "Carry-8b", &carry_8b},
		{"-cd",    "--compress_elias_delta", "Elias delta", &elias_delta},
		{"-cD",    "--compress_elias_delta_bitwise", "Elias delta with bit instuctions (slow)", &elias_delta_bitwise},
		{"-ce",    "--compress_elias_delta_SIMD", "Group Elias Delta SIMD", &elias_delta_simd},
		{"-cE",    "--compress_elias_gamma_SIMD", "Group Elias Gamma SIMD", &elias_gamma_simd},
		{"-cg",    "--compress_elias_gamma", "Elias gamma", &elias_gamma},
		{"-cG",    "--compress_elias_gamma_bitwise", "Elias gamma with bit instuctions (slow)", &elias_gamma_bitwise},
		{"-cn",    "--compress_none", "None", &none},
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
		{"-cZ",    "--compress_qmx_jass_v1", "QMX JASS v1", &qmx_jass_v1},
		{"-c128",  "--compress_128", "Binpack into 128-bit SIMD integers", &bitpack_128},
		{"-c256",  "--compress_256", "Binpack into 256-bit SIMD integers", &bitpack_256},
		{"-c32r",  "--compress_32", "Binpack into 32-bit integers with 8 selectors", &bitpack_32_reduced},
		{"-c64",   "--compress_64", "Binpack into 64-bit integers", &bitpack_64},
		}
	};
#endif
