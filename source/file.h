/*
	FILE.H
	------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
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
		@details This class exists in order to abstract file I/O which has, in the past, been different on different platforms.  For example,
		64-bit file I/O under Windows (using Win32) is awkward because the Win32 functions do not take 64-bit parameters.
		
		This file class is based on Resource Allocation Is Initialisation (RAII).  That is, the file is opened when the object is constructed
		and closed when the object is destroyed.
	*/
	class file
		{
		protected:
			FILE *fp;				///< The underlying representation is a FILE *  from C (as they appear to be fast).
			
		private:
			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Private constructor to prevent the construction of an object without opening a file.
			*/
			file()
				{
				/* Nothing */
				}
			
		public:
			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor used for opening files.
				@param filename [in] the name of the file.
				@param mode [in] The file open mode.  See C's fopen() for details on possible modes.
			*/
			file(const char *filename, const char *mode = "w+b")
				{
				/*
					Open the given file in the given mode
				*/
				#if defined(__STDC_LIB_EXT1__)
					fopen_s(&fp, filename, mode);
				#else
					fp = fopen(filename, mode);
				#endif
				}

			/*
				FILE::~FILE()
				-------------
			*/
			/*!
				@brief Destructor that closes any open file
			*/
			~file()
				{
				fclose(fp);
				}
			
			/*
				FILE::READ()
				------------
			*/
			/*!
				@brief Read buffer.size() bytes from the give file into the buffer.  If at end of file then this method will resize buffer to the number of bytes read from the file.
				@param buffer [in, out] Read buffer.size() bytes into buffer, calling buffer.resize() on failure.
			*/
			void read(std::vector<uint8_t> &buffer);
			
			/*
				FILE::READ()
				------------
			*/
			/*!
				@brief Read bytes number of bytes from the give file into the buffer.
				@param buffer [out] Buffer large enough to hold bytes number of bytes of data which are written into the memory pointed to by buffer.
				@param bytes [in] The number of bytes of data to read.
				@return The number of bytes of data that were read and written into buffer.
			*/
			size_t read(void *buffer, size_t bytes)
				{
				return ::fread(buffer, 1, bytes, fp);
				}

			/*
				FILE::WRITE()
				-------------
			*/
			/*!
				@brief Write bytes number of bytes to the give file at the current cursor position.
				@param buffer [in] the byte sequence to write.
				@param bytes [in] The number of bytes of data to write.
				@return The number of bytes of data that were written to the file.
			*/
			size_t write(const void *buffer, size_t bytes)
				{
				return ::fwrite(const_cast<void *>(buffer), 1, bytes, fp);
				}

			/*
				FILE::SIZE()
				------------
			*/
			/*!
				@brief Return the length of the file as it currently stands
				@return File size in bytes.
			*/
			size_t size(void) const;

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
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
	}