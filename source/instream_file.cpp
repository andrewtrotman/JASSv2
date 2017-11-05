/*
	INSTREAM_FILE.CPP
	-----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include "assert.h"
#include "instream_file.h"

namespace JASS
	{
	/*
		INSTREAM_FILE::INSTREAM_FILE()
		------------------------------
	*/
	instream_file::instream_file(FILE *file) :
		instream(),
		disk_file(file),
		file_length((std::numeric_limits<size_t>::max)()),
		bytes_read(0)
		{
		/* Nothing */
		}

	/*
		INSTREAM_FILE::INSTREAM_FILE()
		------------------------------
	*/
	instream_file::instream_file(const std::string &filename) :
		instream(),
		disk_file(filename.c_str(), "rb"),
		file_length(disk_file.size()),
		bytes_read(0)
		{
		/* Nothing */
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
	if (disk_file.read(&document.contents[0], document.contents.size()) > 0)
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
		auto filename = file::mkstemp("jass");

		/*
			write to the file
		*/
		file::write_entire_file(filename, example_file);
		
		/*
			create an instream_file and test it.
			NOTE: The scope is created so that the object is deleted before removal of the temporary file.
		*/
		do
			{
			instream_file reader(filename);
			document document;
			document.contents = slice(document.contents_allocator, 16);
		
			/*
				read twice from it making sure we got what we should have
			*/
			reader.read(document);
			JASS_assert(document.contents.size() == 16);
			for (size_t index = 0; index < document.contents.size(); index++)
				JASS_assert(document.contents[index] == example_file[index]);
		
			reader.read(document);
			JASS_assert(document.contents.size() == 14);
			for (size_t index = 0; index < document.contents.size(); index++)
				JASS_assert(document.contents[index] == example_file[index + 16]);

			reader.read(document);
			JASS_assert(document.contents.size() == 0);
			}
		while (0);
		/*
			Delete the temporary file.
		*/
		(void)remove(filename.c_str());			// delete the file.  Cast to void to remove Coverity warning if remove() fails.
		
		/*
			Yay, we passed
		*/
		puts("instream_file::PASSED");
		}
	}
