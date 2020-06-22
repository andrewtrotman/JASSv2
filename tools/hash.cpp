/*
	HASH.CPP
	--------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Generate the checksum of a file.
*/
/*!
	@file
	@brief Compute hash values of files
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#include <stdio.h>
#include <stdlib.h>

#include "checksum.h"

/*
	USAGE()
	-------
*/
/*!
	@brief Explain how to use this tool.
	@param filename [in] The path to this executable.
	@return returns 0.
*/
uint8_t usage(const std::string &filename)
	{
	printf("Usage:%s <filename> [<filename> ...]\n", filename.c_str());
	return 0;
	}

/*
	MAIN()
	------
*/
/*!
	@brief Generate the checksum of a file
	@param argc [in] The number of parameters (filenames) to checksum
	@param argv [in] The list of filenames to checksum
	@return 0 on success, else failuer
*/
int main(int argc, char *argv[])
	{
	try
		{
		if (argc == 0)
			exit(usage(argv[0]));

		puts("Fletcher 16 Checksum");
		for (int parameter = 1; parameter < argc; parameter++)
			{
			auto checksum = JASS::checksum::fletcher_16_file(argv[parameter]);
			printf("0x%04X : %s\n",  checksum, argv[parameter]);
			}
		}
	catch (...)
		{
		puts("Unexpected Exception");
		}

	return 0;
	}
