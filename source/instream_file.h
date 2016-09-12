/*
	INSTREAM_FILE.H
	---------------
*/
#pragma once

#include "file.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_FILE
		-------------------
	*/
	class instream_file : public instream
	{
	private:
		file file;
		char *filename;
		size_t file_length;
		size_t bytes_read;

	public:
		instream_file(allocator &memory, char *filename);
		virtual ~instream_file() {};

		virtual void read(std::vector<uint8_t> &buffer);
	};
}
