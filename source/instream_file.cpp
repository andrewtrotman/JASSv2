/*
	INSTREAM_FILE.CPP
	-----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#ifdef WIN32
	#include <io.h>
#endif

#include "assert.h"
#include "instream_file.h"

namespace JASS
	{
	/*
		INSTREAM_FILE::INSTREAM_FILE()
		------------------------------
	*/
	instream_file::instream_file(const std::string &filename) :
		file(filename.c_str(), "rb")				// use RAII to open and close the file
		{
		/*
			Get the file length and note that we've not read any of it yet
		*/
		file_length = file.size();
		bytes_read = 0;
		}

	/*
		INSTREAM_FILE::READ()
		---------------------
	*/
	void instream_file::read(document &document)
	{
	/*
		Make sure we're not past EOF.
	*/
	if (bytes_read >= file_length)
		{
		/*
			At EOF so nothing to read
		*/
		document.contents = slice();
		return;
		}

	/*
		Make sure we're not trying to read past EOF and if we are then only read to EOF
	*/
	if (bytes_read + document.contents.size() > file_length)
		document.contents.resize(file_length - bytes_read);

	/*
		Do the read and note how many bytes we're read.
	*/
	file.read(&document.contents[0], document.contents.size());
	bytes_read += document.contents.size();
	}
	
	/*
		INSTREAM_FILE::UNITTEST()
		-------------------------
	*/
	void instream_file::unittest(void)
		{
		/*
			Write to a disk file using the file class
		*/
		const char *example_file = "123456789012345678901234567890";			// sample to be written and read back
		
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
			#ifdef WIN32
				_mktemp(filename);
			#else
				mktemp(filename);
			#endif
		#endif

		/*
			write to the file
		*/
		file::write_entire_file(filename, example_file);
		
		/*
			create an instream_file
		*/
		instream_file reader(filename);
		document document;
		document.contents = slice(document.contenst_allocator, 15);
		
		/*
			read twice from it making sure we got what we should have
		*/
		reader.read(document);
		JASS_assert(document.contents.size() == 15);
		for (size_t index = 0; index < document.contents.size(); index++)
			JASS_assert(document.contents[index] == example_file[index]);
		
		reader.read(document);
		JASS_assert(document.contents.size() == 15);
		for (size_t index = 0; index < document.contents.size(); index++)
			JASS_assert(document.contents[index] == example_file[index + 15]);

		reader.read(document);
		JASS_assert(document.contents.size() == 0);

		/*
			Yay, we passed
		*/
		puts("instream_file::PASSED");
		}
}