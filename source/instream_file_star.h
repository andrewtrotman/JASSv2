/*
	INSTREAM_FILE_STAR.H
	--------------------
*/
#pragma once

#include <stdio.h>
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_FILE_STAR
		------------------------
	*/
	class instream_file_star : public instream
	{
	private:
		FILE *file;
		size_t bytes_read;

	public:
		instream_file_star(FILE *file) :
			instream(),
			file (infile),
			bytes_read(0)
			{
			}
		virtual ~instream_file_star()
			{
			}

		virtual long long read(unsigned char *buffer, long long bytes);
			{
			bytes_read += bytes;
			return fread(buffer, (size_t)bytes, 1, file);
			}
	} ;
}
