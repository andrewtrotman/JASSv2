/*
	INSTREAM_DIRECTORY_ITERATOR.CPP
	-------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include "assert.h"
#include "instream_file.h"
#include "instream_deflate.h"
#include "instream_directory_iterator.h"

namespace JASS
	{
	/*
		INSTREAM_DIRECTORY_ITERATOR::INSTREAM_DIRECTORY_ITERATOR()
		----------------------------------------------------------
	*/
	instream_directory_iterator::instream_directory_iterator(const std::string &directory_name) :
		source(directory_name),
		reader(nullptr)
		{
		current = begin(source);
		last = end(source);
		}

	/*
		INSTREAM_DIRECTORY_ITERATOR::READ()
		-----------------------------------
	*/
	void instream_directory_iterator::read(document &document)
		{
		/*
			Is this the first time this method is called?
		*/
		if (reader == nullptr)
			if (current != last)
				{
//puts(current->path().string().c_str());
				reader = std::shared_ptr<instream>(new instream_file(current->path().string()));
				if (current->path().string().rfind(".gz") != std::string::npos)
					reader = std::shared_ptr<JASS::instream>(new instream_deflate(reader));
				current++;
				}

		/*
			Now we can get data from the reader and return it, after making sure we're not at EOF.
		*/
		size_t amount_to_read = document.contents.size();
		void *read_into = document.contents.address();

		do
			{
			size_t bytes_read = reader->fetch(read_into, amount_to_read);
			amount_to_read -= bytes_read;

			if (amount_to_read == 0)
				return;

			read_into = ((uint8_t *)read_into) + bytes_read;

			/*
				We're at EOF so move on to the next disk file
			*/
			if (current != last)
				{
//puts(current->path().string().c_str());
				reader = std::shared_ptr<instream>(new instream_file(current->path().string()));
				if (current->path().string().rfind(".gz") != std::string::npos)
					reader = std::shared_ptr<JASS::instream>(new instream_deflate(reader));
				current++;
				}
			else
				{
				document.contents.resize(document.contents.size() - amount_to_read);
				return;				// because we're read past the last file so there is no more data to read
				}
			}
		while (amount_to_read != 0);
		}

	/*
		INSTREAM_DIRECTORY_ITERATOR::UNITTEST()
		---------------------------------------
	*/
	void instream_directory_iterator::unittest(void)
		{
		instream_directory_iterator source(".");

		document blob;
		source.read(blob);
		/*
			Yay, we passed!
		*/
		puts("instream_directory_iterator::PASSED");
		}
	}


