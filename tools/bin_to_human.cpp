/*
	BIN_TO_HUMAN.CPP
	----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Run through the Lemire bin formatted index and dump in a human readable format
		length (4-byte integer)
		posting (length * 4-byte integers)
	repreated until end of file
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NUMBER_OF_DOCUMENTS (1024 * 1024 * 128)

/*
	Buffer to hold the contents of the current postings list.  This is way too large, but only allocated once.
*/
static uint32_t *postings_list;

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	const char *filename;

	/*
		Check parameters
	*/
	if (argc == 2)
		filename = argv[1];											// else the first parameter is the filename to use
	else
		exit(printf("Usage::%s <binfile>", argv[0]));

	/*
		Initialise by setting the count buffers to 0
	*/
	postings_list = new uint32_t[NUMBER_OF_DOCUMENTS];

	/*
		Open the postings list file
	*/
	printf("Using:%s\n", filename);
	FILE *fp;
	if ((fp = fopen(filename, "rb")) == NULL)
		exit(printf("Cannot open %s\n", filename));

	/*
		Iterate through each postings list in the file
	*/
	uint32_t length;
	while (fread(&length, sizeof(length), 1, fp)  == 1)
		{
		if (length > NUMBER_OF_DOCUMENTS * sizeof(*postings_list))
			exit(printf("Too big"));
			
		/*
			Read one postings list (and make sure we did so successfully)
		*/
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("i/o error\n"));

		printf("%u:", (unsigned)length);
		for (uint32_t *where = postings_list; where < postings_list + length; where++)
			printf("%u ", (unsigned)*where);
		printf("\n");
		}

	fclose(fp);

	return 0;
	}
