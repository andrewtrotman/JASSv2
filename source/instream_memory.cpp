/*
	INSTREAM_MEMORY.C
	-----------------
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

		if (bytes_read > file_length)
			{
			buffer.contents = slice();
			return;
			}
		else if (bytes_read + buffer.contents.size() < file_length)
			bytes_to_copy = buffer.contents.size();
		else
			bytes_to_copy = file_length - bytes_read;

		memmove(&buffer.contents[0], file + bytes_read, bytes_to_copy);

		bytes_read += bytes_to_copy;
		}
	}