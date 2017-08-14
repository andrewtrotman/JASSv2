/*
	CHANNEL_FILE.CPP
	----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>

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
		allocator_pool memory;
		allocator_cpp<char> allocator(memory);

		auto filename = file::mkstemp("jass");

		/*
			create an output channel and write to it
		*/
		do
			{
			channel_file outfile(filename);
			outfile << "bytes:";
			outfile << 7;
			outfile << ((const unsigned char *)"\nLine2\n");
			outfile.puts(std::string("1234567"));

			allocator_cpp<char> allocator(memory);
			outfile << JASS::string("Three\n", allocator);
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

			/*
				JASS strings
			*/
			JASS::string into(allocator);
			into.resize(8);
			infile.gets(into);
			JASS_assert(into == "1234567\n");

			/*
				read past eof should return up to eof.
			*/
			infile.read(answer);
			JASS_assert(answer.size() == 6);
			JASS_assert(answer == "Three\n");
			}
		while (0);

		/*
			stdin / stdout
		*/
		channel_file stdio;

		/*
			Delete the file.  Cast to void to remove Coverity warning if remove() fails.
		*/
		(void)remove(filename.c_str());

		/*
			Try gets past end of file
		*/
		/*
			create an output channel and write to it
		*/
		do
			{
			channel_file outfile(filename);
			outfile << "word";
			}
		while (0);

		do
			{
			channel_file infile(filename);
			JASS::string into(allocator);
			into.resize(8);
			infile.gets(into);
			JASS_assert(into == "word");

			infile.block_read(const_cast<char *>(into.c_str()), into.size());
			}
		while (0);

		/*
			Delete the file.  Cast to void to remove Coverity warning if remove() fails.
		*/
		(void)remove(filename.c_str());

		::puts("channel_file::PASSED");
		}
	}
