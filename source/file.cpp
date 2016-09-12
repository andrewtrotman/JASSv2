/*
	FILE.CPP
	--------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase, this code is a re-write using C++ STL classes.
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>

#include "file.h"

namespace JASS
	{
	/*
		FILE::READ_ENTIRE_FILE()
		------------------------
		This uses a combination of "C" FILE I/O and C++ strings in order to copy the contents of a file into an internal buffer.
		There are many different ways to do this, but this is the fastest according to this link: http://insanecoding.blogspot.co.nz/2011/11/how-to-read-in-file-in-c.html
		Note that there does not appear to be a way in C++ to avoid the initialisation of the string buffer.
		
		Returns the length of the file in bytes - which is also the size of the string buffer once read.
	*/
	size_t file::read_entire_file(const std::string &filename, std::string &into)
		{
		FILE *fp;							// "C" pointer to the file
		struct stat details;				// file system's details of the file
		size_t file_length = 0;			// length of the file in bytes

		/*
			Fopen() the file then fstat() it.  The alternative is to stat() then fopen() - but that is wrong because the file might change between the two calls.
		*/
		if ((fp = fopen(filename.c_str(), "rb")) != NULL)
			if (fstat(fileno(fp), &details) == 0)
				if ((file_length = details.st_size) != 0)
					{
					into.resize(file_length);
					if (fread(&into[0], details.st_size, 1, fp) != 1)
						into.resize(0);
					}
		fclose(fp);

		return file_length;
		}

	/*
		FILE::WRITE_ENTIRE_FILE()
		-------------------------
		Uses "C" file I/O to write the contents of buffer to the given names file.
		
		Returns true on success, else false.
	*/
	bool file::write_entire_file(const std::string &filename, const std::string &buffer)
		{
		FILE *fp;						// "C" file to write to

		if ((fp = fopen(filename.c_str(), "wb")) == NULL)
			return false;

		size_t success = fwrite(&buffer[0], buffer.size(), 1, fp);

		fclose(fp);

		return success == 1 ? true : false;
		}

	/*
		FILE::BUFFER_TO_LIST()
		----------------------
		Turn a single std::string into a vector of uint8_t * (i.e. "C" Strings). Note that these pointers are in-place.  That is,
		they point into buffer so any change to the uint8_t or to buffer effect each other.
		
		Note: This method removes blank lines from the input file.
	*/
	void file::buffer_to_list(std::vector<uint8_t *> &line_list, std::string &buffer)
		{
		uint8_t *pos;
		size_t line_count = 0;

		/*
			Walk the buffer counting how many lines we think are in there.
		*/
		pos = (uint8_t *)&buffer[0];
		while (*pos != '\0')
			{
			if (*pos == '\n' || *pos == '\r')
				{
				/*
					a seperate line is a consequative set of '\n' or '\r' lines.  That is, it removes blank lines from the input file.
				*/
				while (*pos == '\n' || *pos == '\r')
					pos++;
				line_count++;
				}
			else
				pos++;
			}

		/*
			resize the vector to the right size, but first clear it.
		*/
		line_list.clear();
		line_list.reserve(line_count);

		/*
			Now rewalk the buffer turning it into a vector of lines
		*/
		pos = (uint8_t *)&buffer[0];
		if (*pos != '\n' && *pos != '\r' && *pos != '\0')
			line_list.push_back(pos);
		while (*pos != '\0')
			{
			if (*pos == '\n' || *pos == '\r')
				{
				*pos++ = '\0';
				/*
					a seperate line is a consequative set of '\n' or '\r' lines.  That is, it removes blank lines from the input file.
				*/
				while (*pos == '\n' || *pos == '\r')
					pos++;
				if (*pos != '\0')
					line_list.push_back(pos);
				}
			else
				pos++;
			}
		}

	/*
		FILE::IS_DIRECTORY()
		--------------------
		Determines whether the given file system object is a directoy or not.
	
		Returns true if filename is a directory, else returns false.
	*/
	bool file::is_directory(const std::string &filename)
		{
		struct stat st;				// file system details

		if(stat(filename.c_str(), &st) == 0)
			return S_ISDIR(st.st_mode);		// simply check the S_ISDIR() flag
		return false;
		}

	/*
		FILE::READ()
		------------
	*/
	void file::read(std::vector<uint8_t> &buffer)
		{
		/*
			Read from the file
		*/
		size_t bytes_read = ::fread(&buffer[0], 1, buffer.size(), fp);
		
		/*
			If we got a short read then resize the buffer to signal back to the caller that we failed to read (probably EOF).
		*/
		if (bytes_read != buffer.size())
			buffer.resize(bytes_read);
		}
	
	/*
		FILE::SIZE()
		------------
	*/
	size_t file::size(void)
		{
		/*
			Since we already have a handle to the file, we just remember where we are,
			seek to the end and heck where we are now, and seek back.  This will probably
			be very fast as it doesn't (normally) need to do and I/O to compute the answer
		*/
		off_t current_position = ftello(fp);
		fseeko(fp, 0, SEEK_END);
		off_t file_size = ftello(fp);
		fseeko(fp, current_position, SEEK_SET);
		
		/*
			This will fail in the case where off_t is larger than a size_t.  This is unlikely.
			On the machines this is being developed on both size_t and off_t are 8-byte integers.
		*/
		return file_size;
		}



	/*
		FILE::UNITTEST()
		----------------
	*/
	void file::unittest(void)
		{
		std::vector<uint8_t *> lines;
		std::string example_file;
		std::string reread;

		/*
			CHECK IS_DIRECTORY()
		*/
		/*
			dot must be a directory
		*/
		assert(is_directory("."));
		
		/*
			something we know is not a directory.  In this case we'll use this very file.  Yes, this assumes
			the unit tests are not run when the source code is not available - but I think that's reasonable.
		*/
		assert(!is_directory(__FILE__));

		/*
			CHECK WRITE_ENTIRE_FILE() then READ_ENTIRE_FILE()
		*/
		example_file = "text for example file";			// sample to be written and read back
		
		/*
			create a temporary filename.  There doesn't appear to be a clean way of doing this.
		*/
		char filename[11];
		strcpy(filename, "jassXXXXXX");
		#ifndef __clang_analyzer__
			/*
				The Xcode code analysis tool correctly says:
				"Call to function 'mktemp' is insecure as it always creates or uses insecure temporary file.  Use 'mkstemp' instead"
				However, as we need a way to get the filename to read_entire_file, and you can't turn a file descriptor into filename
				(because it might be a special file like a TCP/IP socket) I don't see a way around this.  The solution appears to be
				to tell the Xcode analysis tool not to look at the line below.  Actually, this should continue to function file within
				this function without this line, but you never know when someone has already taken your filename (so we call mktemp()).
			*/
			mktemp(filename);
		#endif

		/*
			write, read back, and check we didn't lose anything along the way.
		*/
		write_entire_file(filename, example_file);
		read_entire_file(filename, reread);
		assert(example_file == reread);
		remove(filename);								// delete the file once we're done with it
		
		/*
			CHECK BUFFER_TO_LIST()
		*/
		/*
			Empty file is of length 0
		*/
		example_file = "";
		buffer_to_list(lines, example_file);
		assert(lines.size() == 0);

		/*
			File with only blank lines is of length 0
		*/
		example_file = "\r\n";
		buffer_to_list(lines, example_file);
		assert(lines.size() == 0);

		/*
			File without any new lines is of length 1
		*/
		example_file = "one";
		buffer_to_list(lines, example_file);
		assert(lines.size() == 1);
		assert(std::string((char *)lines[0]) == example_file);
		
		/*
			File with a single new line in the middle (none on the end) is of length 2
		*/
		example_file = "one\ntwo";
		buffer_to_list(lines, example_file);
		assert(lines.size() == 2);
		assert(std::string((char *)lines[0]) == "one");
		assert(std::string((char *)lines[1]) == "two");

		/*
			File with tons of blank lines, this one is of length 2
		*/
		example_file = "\n\n\none\r\n\n\rtwo\n\r\n\r\r\r\n\n\n";
		buffer_to_list(lines, example_file);
		assert(lines.size() == 2);
		assert(std::string((char *)lines[0]) == "one");
		assert(std::string((char *)lines[1]) == "two");

		/*
			Yay, we passed
		*/
		puts("file::PASSED");
		}
}