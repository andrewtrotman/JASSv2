/*
	INSTREAM_MEMORY.C
	-----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

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
		if (bytes_read > file_length)
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
	}