/*
	INSTREAM_MEMORY.CPP
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include "assert.h"
#include "unittest_data.h"
#include "instream_memory.h"

namespace JASS
	{
	/*
		INSTREAM_MEMORY::READ()
		-----------------------
	*/
	void instream_memory::read(document &buffer)
		{
		size_t bytes_to_copy;

		/*
			If we're past EOF then return nothing
		*/
		if (bytes_read >= file_length)
			{
			buffer.contents = slice();
			return;
			}
			
		/*
			If we're going to read past EOF then stop at EOF
		*/
		if (bytes_read + buffer.contents.size() < file_length)
			bytes_to_copy = buffer.contents.size();
		else
			bytes_to_copy = file_length - bytes_read;

		/*
			Do the read and update the amount of data we're read.
		*/
		memmove(&buffer.contents[0], file + bytes_read, bytes_to_copy);
		buffer.contents = slice(&buffer.contents[0], bytes_to_copy);
		bytes_read += bytes_to_copy;
		}

	/*
		INSTREAM_MEMORY::UNITTEST()
		---------------------------
	*/
	void instream_memory::unittest(void)
		{
		const char *example_file = "123456789012345678901234567890";
		instream_memory reader(example_file, strlen(example_file));

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

		/*
			Now read past end of file to make sure it fails
		*/
		reader.read(document);
		JASS_assert(document.contents.size() == 0);

		/*
			Yay, we passed
		*/
		puts("instream_memory::PASSED");
		}
	}
