/*
	TEST_INTEGER_COMPRESS.CPP
	-------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*
	Run through a file of postings lists making sure we can compress and decompress it into the same sequebce.
	The format of the filw is:

		length (4-byte integer)
		posting (length * 4-byte integers)
	repreated until end of file

	An example file is Lemire's dump of .gov2 which can be found here: https://lemire.me/data/integercompression2014.html
*/
#include <iostream>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "file.h"
#include "timer.h"
#include "compress_integer_all.h"

/*
	The test set must contain no more that this number of documents.
*/
#define NUMBER_OF_DOCUMENTS (1024 * 1024 * 128)

/*
	Buffer to hold the contents of the current postings list.  This is way too large, but only allocated once.
*/
static uint32_t *postings_list;

/*
	Buffer to hold the compressed postings list
*/
static uint32_t *compressed_postings_list;

/*
	Buffer to hold the decompressed postings list - after compression then decompression this should equal postings_list[]
*/
static uint32_t *decompressed_postings_list;

/*
	Buffer holding the sum of times to decompress postings lists of this length
*/
static uint32_t *decompress_time;

/*
	Buffer holding the number of postings list of this length
*/
static uint32_t *decompress_count;

/*
	Buffer holding the size (in bytes) of each compressed string
*/
static uint32_t *compressed_size;

/*
	Buffer holding the number of postings list of this length
*/
static uint32_t *compressed_count;		// should be identical to decompress_count

/*
	DRAW_HISTOGRAM()
	----------------
*/
void draw_histogram(void)
	{
	printf("length DecompressTimeInNanoseconds CompressedSizeInBytes\n");
	for (uint32_t index = 0; index < NUMBER_OF_DOCUMENTS; index++)
		if (decompress_time[index] != 0)
			printf("%u %u %u\n", index, decompress_time[index] / decompress_count[index], compressed_size[index] / compressed_count[index]);
	}

/*
	GENERATE_DIFFERENCES()
	----------------------
*/
void generate_differences(uint32_t *postings_list, uint32_t length)
	{
	uint32_t previous = 0;

	for (uint32_t current = 0; current < length; current++)
		{
		uint32_t was;
		was = postings_list[current];

		postings_list[current] -= previous;
		previous = was;
		}
	}

/*
	USAGE()
	-------
*/
void usage(const char *exename)
	{
	exit(printf("Usage:%s <testfile>\n", exename));
	}

/*
	GENERATE_EXAMPLE()
	------------------
*/
void generate_example(const std::string &filename)
	{
	JASS::file output(filename, "w+b");

	for (uint32_t length = 1; length < 10; length++)
		{
		output.write(&length, sizeof(length));
		for (uint32_t value = 0; value < length; value++)
			output.write(&value, sizeof(value));
		}
	}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	const char *filename;

	JASS::compress_integer *shrinkerator = JASS::compress_integer_all_compressors[0].codex;
	std::cout << JASS::compress_integer_all_compressors[0].description << '\n';

	/*
		Check parameters
	*/
	if (argc <= 1)
		usage(argv[0]);	// filename = "/Volumes/Other/data/gov2.sorted";			// debug from the IDE
	else if (argc == 2)
		filename = argv[1];											// else the first parameter is the filename to use
	else if (argc == 3)
		{
		generate_example(argv[1]);
		return 0;
		}
	else
		usage(argv[0]);

	/*
		Initialise by setting the count buffers to 0
	*/
	postings_list = new uint32_t[NUMBER_OF_DOCUMENTS];
	compressed_postings_list = new uint32_t[NUMBER_OF_DOCUMENTS];
	decompressed_postings_list = new uint32_t[NUMBER_OF_DOCUMENTS];
	decompress_time = new uint32_t[NUMBER_OF_DOCUMENTS];
	decompress_count = new uint32_t[NUMBER_OF_DOCUMENTS];
	compressed_size = new uint32_t[NUMBER_OF_DOCUMENTS];
	compressed_count = new uint32_t[NUMBER_OF_DOCUMENTS];

puts("zero");
	memset(decompress_time, 0, NUMBER_OF_DOCUMENTS  * sizeof(*decompress_time));
	memset(decompress_count, 0, NUMBER_OF_DOCUMENTS  * sizeof(*decompress_count));
	memset(compressed_size, 0, NUMBER_OF_DOCUMENTS  * sizeof(*compressed_size));
	memset(compressed_count, 0, NUMBER_OF_DOCUMENTS  * sizeof(*compressed_count));
puts("done");

	/*
		Open the postings list file
	*/
	printf("Using:%s\n", filename);
	FILE *fp;
	if ((fp = fopen(filename, "rb")) == NULL)
		exit(printf("cannot open %s\n", filename));

	/*
		Iterate through each postings list in the file
	*/
	uint32_t length;
	uint32_t term_count = 0;
	while (fread(&length, sizeof(length), 1, fp)  == 1)
		{
		term_count++;
		/*
			Read one postings list (and make sure we did so successfully)
		*/
		if (fread(postings_list, sizeof(*postings_list), length, fp) != length)
			exit(printf("i/o error\n"));

printf("Length:%u\n", (unsigned)length);
fflush(stdout);

		/*
			convert into d1-gaps
		*/
		generate_differences(postings_list, length);

		/*
			Compress
		*/
		uint64_t size_in_bytes_once_compressed;
		size_in_bytes_once_compressed = shrinkerator->encode(compressed_postings_list, NUMBER_OF_DOCUMENTS * sizeof(*compressed_postings_list), postings_list, length);


		compressed_size[length] += size_in_bytes_once_compressed;
		compressed_count[length]++;

		/*
			Decompress
		*/
		auto timer = JASS::timer::start();
		shrinkerator->decode(decompressed_postings_list, length, compressed_postings_list, size_in_bytes_once_compressed);
		auto took = JASS::timer::stop(timer).nanoseconds();

		decompress_time[length] += took;
		decompress_count[length]++;

printf("%u integers -> %uns\n", (unsigned)length, (unsigned)took);
fflush(stdout);

		/*
			Verify
		*/
		if (memcmp(postings_list, decompressed_postings_list, length * sizeof(*postings_list)) != 0)
			{
			printf("Fail on list %u\n", term_count);
			for (uint32_t pos = 0; pos < length; pos++)
				if (postings_list[pos] != decompressed_postings_list[pos])
					printf("Fail at pos:%d\n", (int)pos);
			exit(0);
			}
		/*
			Notify
		*/
	#ifdef NEVER
		if (term_count % 1000 == 0)
			{
			printf("Terms:%u\n", (unsigned)term_count);
			fflush(stdout);
			}
	#endif

		}

	printf("Total terms:%u\n", term_count);
	draw_histogram();

	return 0;
	}
