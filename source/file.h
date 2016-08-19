/*
	FILE.H
	------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase, this code is a re-write using C++ STL classes.

*/
/*!
	@header
	file abstract

	file discussion
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
		@class
		@abstract class abstract

		class description
	*/
	class file
		{
		private:
			/*
				FILE::FILE()
				------------
				private constructor (so the class cannot be constructed as all methods are static)
			*/
			file() {}

		public:
			/*
				FILE::READ_ENTIRE_FILE()
				------------------------
				This uses a combination of "C" FILE I/O and C++ strings in order to copy the contents of a file into an internal buffer.
				There are many different ways to do this, but this is the fastest according to this link: http://insanecoding.blogspot.co.nz/2011/11/how-to-read-in-file-in-c.html
				Note that there does not appear to be a way in C++ to avoid the initialisation of the string buffer.
				
				Returns the length of the file in bytes - which is also the size of the string buffer once read.
			*/
			/*!
				@discussion Something
				@param filename filename
				@param into something else
				@return gotcha
			*/
			static size_t									// [out] size of the file in bytes
			read_entire_file
				(
				const std::string &filename,	// [in] path to the file to read.
				std::string &into					// [out] string to write into.  This string will be re-sized to the size of the file
				);
			
			/*
				FILE::WRITE_ENTIRE_FILE()
				-------------------------
				Uses "C" file I/O to write the contents of buffer to the given names file.
				
				Returns true on success, else false.
			*/
			static bool										// [out] true if successful, false if unsuccessful
			write_entire_file
				(
				const std::string &filename,	// [in] name of the file to write to
				const std::string &buffer		// [in] buffer to write to the file
				);
			
			/*
				FILE::BUFFER_TO_LIST()
				----------------------
				Turn a single std::string into a vector of uint8_t * (i.e. "C" Strings). Note that these pointers are in-place.  That is,
				they point into buffer so any change to the uint8_t or to buffer effect each other.
				
				Note: This method removes blank lines from the input file.
			*/
			static void												// no return value
			buffer_to_list
				(
				std::vector<uint8_t *> &line_list,	// [out] the vector to write into
				std::string &buffer						// [in] the string to decompose
				);
		
			/*
				FILE::IS_DIRECTORY()
				--------------------
				Determines whether the given file system object is a directoy or not.
			
				Returns true if filename is a directory, else returns false.
			*/
			static bool										// [out] true if the given path is a directory, false if it is not (or does not exist)
			is_directory
				(
				const std::string &filename	// [in] the path to the file system object to check
				);
			
			/*
				FILE::UNITTEST()
				----------------
			*/
			static void										// no return value
			unittest
				(
				void									// no parameters
				);
		} ;

	}