/*
	FILE.H
	------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase, this code is a re-write using C++ STL classes.

*/
#pragma once

#include <stddef.h>
#include <string>
#include <vector>

namespace JASS
	{
	/*
		CLASS FILE
		----------
	*/
	class file
		{
		private:
			file() {}
			~file() {}
			
		public:
			static size_t read_entire_file(const std::string &filename, std::string &into);
			static bool write_entire_file(const std::string &filename, const std::string &buffer);
			static void buffer_to_list (std::vector<uint8_t *> &line_list, std::string &buffer);
			static bool is_directory(const std::string &filename);
			
			/*
				unit tests
			*/
			static void unittest(void);
		} ;

	}