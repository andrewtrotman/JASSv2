/*
	INSTREAM_FILE.CPP
	-----------------
*/
#include <string.h>
#include "instream_file.h"
#include "memory.h"
#include "file.h"

namespace JASS
	{
	/*
		INSTREAM_FILE::INSTREAM_FILE()
		------------------------------
	*/
	instream_file::instream_file(allocator &memory, char *filename) :
		instream(memory),
		file(filename, "rb")
		{
		this->filename = (char *)memory.malloc(strlen(filename) + 1);
		strcpy(this->filename, filename);

		file_length = file.size();
		bytes_read = 0;
		}

	/*
		INSTREAM_FILE::READ()
		---------------------
	*/
	void instream_file::read(std::vector<uint8_t> &buffer)
	{
	if (bytes_read >= file_length)
		{
		buffer.resize(0);
		return;		// at EOF so nothing to read
		}

	if (bytes_read + buffer.size() > file_length)
		buffer.resize(file_length - bytes_read);

	bytes_read += buffer.size();
	file.read(buffer);
	}
}