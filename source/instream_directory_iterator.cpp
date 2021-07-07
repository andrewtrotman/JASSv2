/*
	INSTREAM_DIRECTORY_ITERATOR.CPP
	-------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include "assert.h"
#include "instream_directory_iterator.h"

namespace JASS
	{
	/*
		INSTREAM_DIRECTORY_ITERATOR::INSTREAM_DIRECTORY_ITERATOR()
		----------------------------------------------------------
	*/
	instream_directory_iterator::instream_directory_iterator(const std::string &directory_name) :
		source(directory_name)
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
#ifdef NEVER
	if (reader == nullptr)

	std::shared_ptr<JASS::instream> deflater(new JASS::instream_deflate(file));
	data_source = new JASS::instream_document_unicoil_json(deflater);

#endif
	if (current != last)
		{
		std::cout << current->path() << "\n";
		current++;
		}
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
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);
		source.read(blob);

		/*
			Yay, we passed!
		*/
		puts("instream_directory_iterator::PASSED");
		}
	}


