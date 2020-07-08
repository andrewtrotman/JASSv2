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

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_ALL
		--------------------------
	*/
	/*!
		@brief A container holding all the integer compression schemes known by JASS
		@details Add a new compressor by creating an instance of it in compress_integer_all.cpp, then creating
		an new row in the table compressors, then incrementing compressors_size
	*/
	class compress_integer_all
		{
		public:
			static constexpr size_t compressors_size = 26;					///< There are currently this many compressors known to JASS
			static constexpr size_t default_compressor = 6;					///< The default one to use is at this position in the compressors array

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
					std::string shortname;					///< The short command line parameter.
					std::string longname;					///< The long command line parameter.
					std::string description;				///< The name of the scheme, in command line use other stuff is wrapped around this.
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

			/*
				COMPRESS_INTEGER_ALL::REPLICATE()
				---------------------------------
			*/
			/*!

				@brief Turn a static reference to an obect into a dynamically allocated object
				@param codex [in] one of the objects in compress_integer_all::compressors[]
				@return A dynamically allocated object of the same type
			*/
			static std::unique_ptr<compress_integer> replicate(std::string &shortname);

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
				@return A pointer to an integer compressor (caller to free), or the default integer compressor if each member of option is set to false.
			*/
			static std::unique_ptr<compress_integer> compressor(const std::array<bool, compressors_size> &option)
				{
				for (size_t which = 0; which < compressors_size; which++)
					if (option[which])
						return replicate(compressors[which].shortname);
					
				return replicate(compressors[default_compressor].shortname);
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
				@return A poionter to a compressor (caller to free) that can encode and decode data using the codex of the given name (or a "null" compressor on error)
			*/
			static std::unique_ptr<compress_integer> get_by_name(const std::string &name)
				{
				for (size_t which = 0; which < compressors_size; which++)
					if (compressors[which].description == name)
						return replicate(compressors[which].shortname);

				return replicate(compressors[0].shortname);
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
