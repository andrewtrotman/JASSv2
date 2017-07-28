/*
	VERISON.H
	---------
	Copyright (c) 2016-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Details about the current executing version of JASS
	@author Andrew Trotman
	@copyright 2016-2017 Andrew Trotman
*/

/*!
	@mainpage The JASS Search Engine

	@section copyright_section Details
	Written by Andrew Trotman (University of Otago, Dunedin, New Zealand) \n
	Copyright (c) 2016-2017 Andrew Trotman\n
	Released under the 2-clause BSD license (See https://en.wikipedia.org/wiki/BSD_licenses).

	@section introduction_section Introduction
	JASS is an experimental search engine written in C++14.  It is still in its early stages
	and it will be some months before it goes into alpha.  Please feel free to contribute.
*/

#pragma once

#include <string>

namespace JASS
	{
	/*!
		@brief Details about the current executing version of JASS
	*/
	class version
		{
		public:
			static constexpr const char *version_string = "0.1";			///< Current version of JASS as a C string
			static constexpr uint64_t version_BCD = 0x0001;			///< Current version of JASS as a BCD string
			static constexpr size_t bitness = sizeof(void *) * 8;	///< Either a 32-bit or 64-bit build
			#if defined(DEBUG) || defined(_DEBUG)
				#ifdef NDEBUG
					#error "NDEBUG is defined in a DEBUG build - Can't proceed without know which is true"
				#else
					static constexpr const char *debugness = "DEBUGGING-build";		///< String declairing whether or not this is a debug build
					static constexpr bool debug = true;							///< Is this a debug build or not?
				#endif
			#else
				static constexpr const char *debugness = "";					///< String declairing whether or not this is a debug build
				static constexpr bool debug = false;							///< Is this a debug build or not?
			#endif

			/*
				VERSION::BUILD()
				----------------
			*/
			/*!
				@brief Return a string containing the build details and authorship
				@return A string 
			*/
			static std::string build(void)
				{
				return std::string("JASS Version ") + std::string(version_string) + std::string(" ") + std::to_string(bitness) + std::string("-bit ") + std::string(debugness) + std::string("Copright (c) 2016-2017 Andrew Trotman, University of Otago");
				}


			/*
				VERSION::CREDITS()
				------------------
			*/
			/*!
				@brief Return a string containing the credits.
				@return String containing the build information and the credits.
			*/
			static std::string credits(void)
				{
				return
					build() + "\n"
					"DESIGN & IMPLEMENTATION\n"
					"-----------------------\n"
					"Andrew Trotman\n";
				}
		};
	}



