/*
	CHANNEL_FILE.CPP
	---------------
*/
#include <stdio.h>

#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#include "channel_file.h"
#include "instream_file_star.h"

namespace JASS
	{
	/*
		CHANNEL_FILE::CHANNEL_FILE()
		----------------------------
	*/
	channel_file::channel_file() :
		filename("<stdin>"),
		infile(new instream_file_star(stdin)),			// use stdin
		outfile(stdout),										// use stdout
		eof(false)
		{
		/* Nothing */
		}

	/*
		CHANNEL_FILE::CHANNEL_FILE()
		----------------------------
	*/
	channel_file::channel_file(const std::string &filename) :
		filename(filename),
		infile(new instream_file(filename)),			// use the named file
		outfile(nullptr),
		eof(false)
		{
		/* Nothing */
		}

	/*
		CHANNEL_FILE::~CHANNEL_FILE()
		-----------------------------
	*/
	channel_file::~channel_file()
		{
		if (outfile != stdout && outfile != nullptr)
			::fclose(outfile);
		}

	/*
		CHANNEL_FILE::BLOCK_WRITE()
		---------------------------
	*/
	size_t channel_file::block_write(const void *buffer, size_t length)
	{
	/*
		Use delayed open (for append) so that we don't create a zero-length file if there is no output.
	*/
	if (outfile == nullptr)
		{
		outfile = ::fopen(filename.c_str(), "a+b");
		rewind(outfile);
		}
	/*
		write
	*/
	return ::fwrite(buffer, length, 1, outfile);
	}

	/*
		CHANNEL_FILE::BLOCK_READ()
		--------------------------
	*/
	size_t channel_file::block_read(void *into, size_t length)
	{
	size_t bytes_read;

	/*
		at end of file so fail
	*/
	if (eof)
		return 0;

	/*
		read from the file
	*/
	if ((bytes_read = infile->fetch(into, length)) != length)
		eof = true;

	/*
		return the number of bytes read
	*/
	return bytes_read;
	}

	/*
		CHANNEL_FILE::UNITTEST()
		------------------------
	*/
	void channel_file::unittest(void)
		{
		char filename[11];
		strcpy(filename, "jassXXXXXX");

		/*
			Create a temporary filename.  Windows does not appear to have mkstemp so we use _mktemp().
		*/
		#ifdef WIN32
			_mktemp(filename);
		#else
			umask(umask(0));				// This sets the umask to its current value, and prevents Coverity from producing a warning
			int file_descriptor = mkstemp(filename);
			if (file_descriptor >= 0)
				close(file_descriptor);
		#endif

		/*
			create an output channel and write to it
		*/
		do
			{
			channel_file outfile(filename);
			outfile << "bytes:";
			outfile << "7";
			outfile << ((const unsigned char *)"\nLine2\n");
			}
		while (0);

		/*
			create an input channel for the same file and read from it
		*/
		do
			{
			channel_file infile(filename);
			std::string answer;
			answer.resize(8);
			infile.read(answer);
			JASS_assert(answer == "bytes:7\n");
			infile.gets(answer);
			JASS_assert(answer == "Line2\n");
			}
		while (0);

		/*
			Delete the file.  Cast to void to remove Coverity warning if remove() fails.
		*/
		(void)remove(filename);

		::puts("channel_file::PASS");
		}
}
