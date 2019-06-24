/*
	SHARED_INDEX_FORMAT.CPP
	-----------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Convert Jimmy Lin's shared index format into a JASS index.
*/
#include <stdint.h>

#include <iostream>

#include "file.h"
#include "ciff_lin.h"

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::string file;
	size_t file_size = JASS::file::read_entire_file(argv[1], file);
	JASS::ciff_lin source((uint8_t *)&file[0], file_size);

	for (const auto &posting : source)
		{
		std::cout.write((char *)posting.term.start, posting.term.length);
		std::cout << " " << posting.document_frequency << " " << posting.collection_frequency << "\n";
		}

	return 0;
	}
