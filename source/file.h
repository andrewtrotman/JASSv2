/*
	FILE.H
	------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase, this code is a re-write using C++ STL classes.
*/
/*!
	@file
	@brief Partial file and whole file based I/O methods.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
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
	/*!
		@brief File based I/O methods including whole file and partial files.
	*/
	class file
		{
		public:
			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor.
			*/
			file() {}

			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Destructor.
			*/
			~file() {}
			
			/*
				FILE::READ_ENTIRE_FILE()
				------------------------
			*/
			/*!
				@brief Read the contents of file filename into the std::string into.
				@details Because into is a string it is naturally '\0' terminated by the C++ std::string class.
				@param filename [in] The path of the file to read.
				@param into [out] The std::string to write into.  This string will be re-sized to the size of the file.
				@return The size of the file in bytes
			*/
			static size_t read_entire_file(const std::string &filename, std::string &into);
			
			/*
				FILE::WRITE_ENTIRE_FILE()
				-------------------------
			*/
			/*!
				@brief Write the contents of buffer to the file specified in filenane.
				@details If the file does not exist it is created.  If it does already exist it is overwritten.
				@param filename [in] The path of the file to write to.
				@param buffer [in] The data to write to the file.
				@return True if successful, false if unsuccessful
			*/
			static bool write_entire_file(const std::string &filename, const std::string &buffer);
			
			/*
				FILE::BUFFER_TO_LIST()
				----------------------
			*/
			/*!
				@brief Turn a single std::string into a vector of uint8_t * (i.e. "C" strings).
				@details Note that these pointers are in-place.  That is,
				they point into buffer so any change to the uint8_t or to buffer effect each other.  This method removes blank lines from buffer and
				changes buffer by inserting '\0' characters at the end of each line.
				@param line_list [out] The vector to write into
				@param buffer [in, out] the string to decompose
			*/
			static void buffer_to_list(std::vector<uint8_t *> &line_list, std::string &buffer);
		
			/*
				FILE::IS_DIRECTORY()
				--------------------
			*/
			/*!
				@brief Determines whether the given file system object is a directoy or not.
				@param filename [in] The path to the file system object to check.
				@return True if the given path is a directory, false if it is not (or does not exist).
			*/
			static bool is_directory(const std::string &filename);
			
			/*
				FILE::UNITTEST()
				----------------
			*/
			/*!
				@brief Static method that performs a unittest.
				@details .
			*/
			static void unittest(void);
		} ;

	}