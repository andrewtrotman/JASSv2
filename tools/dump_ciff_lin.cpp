/*
	DUMP_CIFF_LIN.CPP
	-----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Read and dump an index in Jimmy Lin's Common Index File Format.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>

#include "ciff_lin.h"

/*
		READ_ENTIRE_FILE()
		------------------
*/
size_t read_entire_file(const std::string &filename, std::string &into)
	{
	FILE *fp;
	struct stat details;
	size_t file_length = 0;

	if ((fp = fopen(filename.c_str(), "rb")) != nullptr)
		{
		if (fstat(fileno(fp), &details) == 0)
			if ((file_length = details.st_size) != 0)
				{
				into.resize(file_length);
				if (fread(&into[0], details.st_size, 1, fp) != 1)
					into.resize(0);
				}
		fclose(fp);
		}

	return file_length;
	}
	
/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::string file;
	size_t file_size;

	if (argc != 2)
		exit(printf("Usage:%s <infile.pb>\n", argv[0]));

	if ((file_size = read_entire_file(argv[1], file)) == 0)
		exit(printf("Can't read file:%s\n", argv[1]));

	JASS::ciff_lin source((uint8_t *)&file[0], file_size);

	for (const auto &posting : source)
		{
		std::cout.write((char *)posting.term.start, posting.term.length);
		std::cout << " " << posting.document_frequency << " " << posting.collection_frequency << "\n";
		}
	if (source.status == JASS::ciff_lin::FAIL)
		exit(printf("File is not in the correct format\n"));

	return 0;
	}
