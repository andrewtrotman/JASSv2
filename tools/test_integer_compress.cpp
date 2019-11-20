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

std::vector<uint32_t> postings_list;					//< Buffer to hold the contents of the current postings list.  This is way too large, but only allocated once.
std::vector<uint32_t> compressed_postings_list;		//< Buffer to hold the compressed postings list
std::vector<uint32_t> decompressed_postings_list;	//< Buffer to hold the decompressed postings list - after compression then decompression this should equal postings_list[]
std::vector<uint64_t> decompress_time;					//< Buffer holding the sum of times to decompress postings lists of this length
std::vector<uint64_t> decompress_count;				//< Buffer holding the number of postings list of this length
std::vector<uint64_t> compressed_size;					//< Buffer holding the size (in bytes) of each compressed string
std::vector<uint64_t> compressed_count;				//< Buffer holding the number of postings list of this length, should be identical to decompress_count

bool data_counts_from_zero = false;						///< If the postings count from 0 then add 1 to each document ID to avoid compressing 0s (because Elias gamma and Elias delta cannot encode 0s)

/*
	CLASS POSTINGS_LIST
	-------------------
*/
class postings_list
	{
	uint32_t size;
	uint32_t *data;
	};

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
			{
			if (compressed_count[index] == 0)
				std::cout << "COMPRESSED_COUNT[" << index << "] == 0\n";
			if (decompress_count[index] == 0)
				std::cout << "DECOMPRESS_COUNT[" << index << "] == 0\n";

			if (compressed_count[index] != 0 && decompress_count[index] != 0)
				std::cout << index << " " << decompress_time[index] / decompress_count[index] << " " << compressed_size[index] / compressed_count[index] << "\n";
			}
	std::cout << "DONE" << std::endl;
	fflush(stdout);
	}

/*
	GENERATE_DIFFERENCES()
	----------------------
	Compute the d1-gap delta's of the postings list (consequetive differences)
*/
void generate_differences(std::vector<uint32_t> &postings_list)
	{
	uint32_t previous = postings_list[0];
	size_t length = postings_list.size();
	
	for (size_t current = 1; current < length; current++)
		{
		uint32_t was = postings_list[current];

		postings_list[current] -= previous;
		previous = was;
		}

	/*
		Some codexes cannot encode zeros (e.g. Elias gamma and Elias delta) so if the data counts document IDs from 0 then we have to add 1.
		This works because everything is delta encoded from the first ID so we just add 1 to the first ID.
	*/
	if (data_counts_from_zero)
		postings_list[0]++;
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
	bool verify = false;
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
			JASS::commandline::parameter("-z", "--has-zeros", "The postings file counts from 0, so add 1 to avoid compressing 0s", data_counts_from_zero),
			JASS::commandline::parameter("-v", "--verify", "verify the decoded sequence matches the original sequence", verify),

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
		Read the postings list file
	*/
	std::string entire_file;
	JASS::file::read_entire_file(filename, entire_file);
	if (entire_file.size() == 0)
		exit(printf("cannot open %s\n", filename.c_str()));

	/*
		Iterate through each postings list in the file and create a vector pointing to each list
	*/
	uint32_t length;
	uint32_t term_count = 0;
	uint32_t *file_pointer = reinterpret_cast<uint32_t *>(entire_file.data());
	uint32_t *end_of_file = file_pointer + entire_file.size() / sizeof(uint32_t);

	while (file_pointer < end_of_file)
		{
		length = *file_pointer++;
		term_count++;
		
		/*
			Read one postings list (and make sure we did so successfully)
		*/
		std::copy(file_pointer, file_pointer + length, &postings_list[0]);
		file_pointer += length;

//printf("Length:%u\n", (unsigned)length);
//fflush(stdout);

		/*
			convert into d1-gaps
		*/
		postings_list.resize(length);
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
		if (verify)
			if (memcmp(&postings_list[0], &decompressed_postings_list[0], length * sizeof(postings_list[0])) != 0)
				{
				std::cout << "Fail on list " << term_count << "\n";
				for (uint32_t pos = 0; pos < length; pos++)
					if (postings_list[pos] != decompressed_postings_list[pos])
						std::cout << "Fail at pos:" << pos << "[" <<  decompressed_postings_list[pos] << "!=" << postings_list[pos] << "]\n";

				for (uint32_t pos = 10789632; pos < length; pos++)
					std::cout << postings_list[pos] << ", ";

				exit(0);
				}

		/*
			Notify
		*/
		if (term_count % report_every == 0)
			std::cout << "Terms processed:" << term_count << std::endl;
		}

	std::cout << "Total terms:" << term_count << "\n";
	draw_histogram();

	return 0;
	}
