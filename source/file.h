/*
	FILE.H
	------
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
			static size_t read_entire_file(std::string &filename, std::string &into);
			static bool write_entire_file(std::string &filename, std::string &buffer);
			static void buffer_to_list (std::vector<uint8_t *> &line_list, std::string &buffer);
			static bool is_directory(std::string &filename);

			/*
				rvalue reference versions
			*/
			static bool write_entire_file(std::string &&filename, std::string &buffer) { return write_entire_file(filename, buffer); }
			static bool write_entire_file(std::string &&filename, std::string &&buffer) { return write_entire_file(filename, buffer); }
			static size_t read_entire_file(std::string &&filename, std::string &into) { return read_entire_file(filename, into); }
			static bool is_directory(std::string &&filename) { return is_directory(filename); }
			
			/*
				unit tests
			*/
			static void unittest(void);
		} ;

	}