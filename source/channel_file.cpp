/*
	CHANNEL_FILE.CPP
	---------------
*/
#include "memory.h"
#include "channel_file.h"
#include "instream_file.h"

namespace JASS
	{
	/*
		CHANNEL_FILE::CHANNEL_FILE()
		----------------------------
	*/
	channel_file::channel_file() :
		filename("<stdin>")
	{
	if (filename.size() == 0)
		{
		outfile = stdout;
		infile = new ANT_instream_file_star(stdin);
		eof = false;
		}
	}

	/*
		CHANNEL_FILE::CHANNEL_FILE()
		----------------------------
	*/
	channel_file::channel_file(const std::string &filename) :
		filename(filename)
	{
		{
		outfile = NULL;
		infile = NULL;
		eof = true;
		}
	}

	/*
		CHANNEL_FILE::~CHANNEL_FILE()
		-----------------------------
	*/
	channel_file::~channel_file()
	{
	if (outfile != stdout && outfile != NULL)
		fclose(outfile);

	delete infile;
	}

	/*
		ANT_CHANNEL_FILE::BLOCK_WRITE()
		-------------------------------
	*/
	long long ANT_channel_file::block_write(char *source, long long length)
	{
	if (outfile == NULL)
		{
		outfile = fopen(filename, "a+b");		// open for append
		fseek(outfile, 0L, SEEK_SET);
		}
	return fwrite(source, (size_t)length, 1, outfile);
	}

	/*
		ANT_CHANNEL_FILE::BLOCK_READ()
		------------------------------
	*/
	char *ANT_channel_file::block_read(char *into, long long length)
	{
	if (infile == NULL)
		{
		memory = new ANT_memory(1024 * 1024);		// use a 1MB buffer;

		if (strrcmp(filename, ".gz") == 0)
			infile = new ANT_instream_deflate(memory, new ANT_instream_file(memory, filename));
		else if (strrcmp(filename, ".zip") == 0)
			infile = new ANT_instream_pkzip(memory, filename);
		else
			infile = new ANT_instream_file(memory, filename);
		eof = false;
		}

	if (eof)
		return NULL;

	if (infile->read(into, (size_t)length) == length)
		return into;

	eof = true;
	return NULL;
	}

	/*
		ANT_CHANNEL_FILE::GETSZ()
		-------------------------
	*/
	char *ANT_channel_file::getsz(char terminator)
	{
	char *buffer = NULL;
	long bytes_read, buffer_length, used, old_length, block_size = 1024;
	char next, *got;

	bytes_read = buffer_length = used = 0;

	buffer = new char [old_length = block_size + 2];
	*buffer = '\0';
	/*
		Else we do a gets() and stop when we hit the terminator
	*/
	while ((got = block_read(&next, 1)) != NULL)
		{
		bytes_read++;
		if (next == terminator)
			break;
		if  (got == NULL)
			if (used == 0)
				{
				delete [] buffer;
				return NULL;
				}
			else
				break;
			
		if (used >= buffer_length)
			{
			old_length = buffer_length;
			buffer_length += block_size;
			buffer = strrenew(buffer, old_length, buffer_length + 2);		// +1 for terminator and +1 for the '\0' on the end
			}
		buffer[used] = (char)next;
		used++;
		}

	if (buffer != NULL)
		{
		/*
			We got line of stuff so terminate it with the terminator then null terminate that.
		*/
		buffer[used] = (char)next;
		buffer[used + 1] = '\0';
		}

	if (bytes_read == 0)
		{
		delete [] buffer;
		return NULL;
		}

	return buffer;
	}
}
