/*
	TEST_INTEGER_COMPRESS.CPP
	-------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*
	Run through a file of postings lists making sure we can compress and decompress it into the same sequebce.
	The format of the file is:

		length (4-byte integer)
		posting (length * 4-byte integers)
	repreated until end of file

	An example file is Lemire's dump of .gov2 which can be found here: https://lemire.me/data/integercompression2014.html
*/
#include <limits>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "file.h"
#include "timer.h"
#include "commandline.h"
#include "compress_integer_all.h"

/*
	The test set must contain no more that this number of documents.
*/
#define NUMBER_OF_DOCUMENTS (1024 * 1024 * 20)

std::vector<uint32_t> postings_list;				// Buffer to hold the contents of the current postings list.  This is way too large, but only allocated once.
std::vector<uint32_t> compressed_postings_list;		// Buffer to hold the compressed postings list
std::vector<uint32_t> decompressed_postings_list;	// Buffer to hold the decompressed postings list - after compression then decompression this should equal postings_list[]
std::vector<uint64_t> decompress_time;				// Buffer holding the sum of times to decompress postings lists of this length
std::vector<uint64_t> decompress_count;				// Buffer holding the number of postings list of this length
std::vector<uint64_t> compressed_size;				// Buffer holding the size (in bytes) of each compressed string
std::vector<uint64_t> compressed_count;				// Buffer holding the number of postings list of this length, should be identical to decompress_count

/*
	DRAW_HISTOGRAM()
	----------------
	Write out the number-of-integers, time-to-decompress and the compressed-size averaged over the input data.
*/
void draw_histogram(void)
	{
	std::cout << "length DecompressTimeInNanoseconds CompressedSizeInBytes\n";
	for (uint32_t index = 0; index < NUMBER_OF_DOCUMENTS; index++)
		if (decompress_time[index] != 0)
			std::cout << index << " " << decompress_time[index] / decompress_count[index] << " " << compressed_size[index] / compressed_count[index] << "\n";
	}

/*
	GENERATE_DIFFERENCES()
	----------------------
	Compute the d1-gap delta's of the postings list (consequetive differences)
*/
void generate_differences(std::vector<uint32_t> &postings_list)
	{
	uint32_t previous = 0;
	size_t length = postings_list.size();
	
	for (size_t current = 0; current < length; current++)
		{
		uint32_t was = postings_list[current];

		postings_list[current] -= previous;
		previous = was;
		}
	}

/*
	GENERATE_EXAMPLE()
	------------------
	Generate a simple sample set of test data.
*/
void generate_example(const std::string &filename)
	{
	JASS::file output(filename, "w+b");

	for (uint32_t length = 1; length < 11; length++)
		{
		output.write(&length, sizeof(length));
		for (uint32_t value = 0; value < length; value++)
			output.write(&value, sizeof(value));
		}
	}

/*
	USAGE()
	-------
	Write out the useage statistics.
*/
template <typename TYPE>
void usage(const char *exename, TYPE &command_line_parameters)
	{
	std::cout << JASS::commandline::usage(exename, command_line_parameters);
	exit(0);
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	/*
		Set up for parsing the command line
	*/
	uint64_t report_every = (std::numeric_limits<uint64_t>::max)();							// print a message every this number of postings lists
	bool generate = false;																// should we generate a sample file (usually false)
	std::string filename = "";															// the name of the postings list file to check with
	std::array<bool, JASS::compress_integer_all::compressors_size> selectors = {};		// which compressor does the user select
	auto command_line = JASS::compress_integer_all::parameterlist(selectors);			// get list of avaiable compressors
	auto all_parameters = std::tuple_cat
		(
		std::make_tuple
			(
			JASS::commandline::note("\nFILENAME PARSING\n----------------"),
			JASS::commandline::parameter("-f", "--filename", "Name of encoded postings list file", filename),
			JASS::commandline::note("\nCOMPRESSORS\n-----------")
			),
		command_line,
		std::make_tuple
			(
			JASS::commandline::note("\nGENERATE\n--------"),
			JASS::commandline::parameter("-G", "--Generate", "generate a sample file for checking (you are unlikely to want to do this)", generate),
			JASS::commandline::note("\nREPORTING\n---------"),
			JASS::commandline::parameter("-N", "--report-every", "<n> Report time and memory every <n> documents.", report_every)
			)
		);

	/*
		parse the command line.
	*/
	std::string error;
	if (!JASS::commandline::parse(argc, argv, all_parameters, error))
		usage(argv[0], all_parameters);

	/*
		Check parameters
	*/
	if (filename == "")
		usage(argv[0], all_parameters);

	/*
		Announce what we're about to do
	*/
	if (generate)
		{
		std::cout << "Generate " << filename << '\n';
		generate_example(filename);
		return 0;
		}
	JASS::compress_integer &shrinkerator = JASS::compress_integer_all::compressor(selectors);
	std::cout << "Check " << JASS::compress_integer_all::name(selectors) << " on file " << filename << '\n';

	/*
		Initialise by setting the count buffers to 0
	*/
	postings_list.resize(NUMBER_OF_DOCUMENTS);
	compressed_postings_list.resize(NUMBER_OF_DOCUMENTS);
	decompressed_postings_list.resize(NUMBER_OF_DOCUMENTS);
	decompress_time.resize(NUMBER_OF_DOCUMENTS);
	decompress_count.resize(NUMBER_OF_DOCUMENTS);
	compressed_size.resize(NUMBER_OF_DOCUMENTS);
	compressed_count.resize(NUMBER_OF_DOCUMENTS);

	/*
		Open the postings list file
	*/
	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == nullptr)
		exit(printf("cannot open %s\n", filename.c_str()));

	/*
		Iterate through each postings list in the file
	*/
	uint32_t length;
	uint32_t term_count = 0;
	while (fread(&length, sizeof(length), 1, fp)  == 1)
		{
		term_count++;
		
		/*
			Coverity points out that length is "tainted" and that bad input can, therefore, be used to attack this program.
			So we make sure length is not too large.
		*/
		if (length > postings_list.size())
			exit(printf("fatal error: NUMBER_OF_DOCUMENTS is smaller than the length of this postings list (%lld vs %lld)", (long long)NUMBER_OF_DOCUMENTS, (long long)length));
			
		/*
			Read one postings list (and make sure we did so successfully)
		*/
		if (fread(&postings_list[0], sizeof(postings_list[0]), length, fp) != length)
			exit(printf("i/o error\n"));

//printf("Length:%u\n", (unsigned)length);
//fflush(stdout);

		/*
			convert into d1-gaps
		*/
		generate_differences(postings_list);

		/*
			Compress
		*/
		uint64_t size_in_bytes_once_compressed;
		size_in_bytes_once_compressed = shrinkerator.encode(&compressed_postings_list[0], compressed_postings_list.size() * sizeof(compressed_postings_list[0]), &postings_list[0], length);


		compressed_size[length] += size_in_bytes_once_compressed;
		compressed_count[length]++;

		/*
			Decompress
		*/
		auto timer = JASS::timer::start();
		shrinkerator.decode(&decompressed_postings_list[0], length, &compressed_postings_list[0], size_in_bytes_once_compressed);
		auto took = JASS::timer::stop(timer).nanoseconds();

		decompress_time[length] += took;
		decompress_count[length]++;

//printf("%u integers -> %uns\n", (unsigned)length, (unsigned)took);
//fflush(stdout);

		/*
			Verify
		*/
		if (memcmp(&postings_list[0], &decompressed_postings_list[0], length * sizeof(postings_list[0])) != 0)
			{
			std::cout << "Fail on list " << term_count << "\n";
			for (uint32_t pos = 0; pos < length; pos++)
				if (postings_list[pos] != decompressed_postings_list[pos])
					std::cout << "Fail at pos:" << pos << "\n";
			exit(0);
			}
			
		/*
			Notify
		*/
		if (term_count % report_every == 0)
			std::cout << "Terms processed:" << term_count << std::endl;
		}

	std::cout << "Total terms:" << term_count << "\n";
	fclose(fp);
	draw_histogram();

	return 0;
	}
