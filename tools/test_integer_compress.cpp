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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <atomic>
#include <limits>
#include <iostream>
#include <algorithm>

#include "file.h"
#include "maths.h"
#include "timer.h"
#include "commandline.h"
#include "compress_integer_all.h"

std::vector<uint64_t> decompress_time;					//< Buffer holding the sum of times to decompress postings lists of this length
std::vector<uint64_t> decompress_count;				//< Buffer holding the number of postings list of this length
std::vector<uint64_t> compressed_size;					//< Buffer holding the size (in bytes) of each compressed string
std::vector<uint64_t> compressed_count;				//< Buffer holding the number of postings list of this length, should be identical to decompress_count

bool data_counts_from_zero = false;						///< If the postings count from 0 then add 1 to each document ID to avoid compressing 0s (because Elias gamma and Elias delta cannot encode 0s)

uint64_t report_every = (std::numeric_limits<uint64_t>::max)();							// print a message every this number of postings lists
bool verify = false;

/*
	CLASS POSTINGS_LIST_OBJECT
	--------------------------
*/
class postings_list_object
	{
	public:
		enum
			{
			RAW,
			COMPRESSED,
			DECOMPRESSED
			};

	public:
		std::atomic<uint8_t> status;			// 0 = not compressed, 1 = compressed, 2 = decompressed
		uint32_t length;
		uint32_t *raw;
		uint64_t size_in_bytes_once_compressed;
		std::vector<uint32_t> compressed_postings_list;
		size_t decompress_time;

	public:
		/*
			POSTINGS_LIST_OBJECT::POSTINGS_LIST_OBJECT()
			--------------------------------------------
		*/
		postings_list_object() :
			status(RAW),
			length(0),
			raw(nullptr),
			size_in_bytes_once_compressed(0),
			compressed_postings_list(0),
			decompress_time(0)
			{
			/* Nothing */
			}
	};

/*
	DRAW_HISTOGRAM()
	----------------
	Write out the number-of-integers, time-to-decompress and the compressed-size averaged over the input data.
*/
void draw_histogram(uint32_t longest_list)
	{
	std::cout << "length DecompressTimeInNanoseconds CompressedSizeInBytes\n";
	for (uint32_t index = 0; index < longest_list; index++)
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
void generate_differences(uint32_t *postings_list, size_t length)
	{
	uint32_t previous = postings_list[0];

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
	COMPRESS_ALL_LISTS()
	--------------------
*/
void compress_all_lists(uint32_t longest_list, JASS::compress_integer &shrinkerator, std::vector<postings_list_object>  &entire_index)
	{
	size_t term_count = 0;

	for (auto &list : entire_index)
		{
		term_count++;
		/*
			Find a list that hasn't yet been compressed and mark it as done.
		*/
		uint8_t should_be = postings_list_object::RAW;
		if (list.status == should_be)
			if (list.status.compare_exchange_strong(should_be, postings_list_object::COMPRESSED))
				{
				//std::cout << "Length:" << list.length << std::endl;

				/*
					Convert into d1-gaps in-place so that we can verify in decompression later (if needed)
				*/
				generate_differences(list.raw, list.length);

				/*
					Compress
				*/
				list.compressed_postings_list.resize(list.length + 1024);	// to allow space for bad cases where it becomes (a bit) longer.
				list.size_in_bytes_once_compressed = shrinkerator.encode(list.compressed_postings_list.data(), list.compressed_postings_list.size() * sizeof(list.compressed_postings_list[0]), list.raw, list.length);

				/*
					Notify
				*/
				if (term_count % report_every == 0)
					{
					std::ostringstream output;
					output << "Terms Compressed:" << term_count << '\n';
					std::cout << output.str();
					}
				}
		}
	}

/*

	DECOMPRESS_ALL_LISTS()
	----------------------
*/
void decompress_all_lists(uint32_t longest_list, JASS::compress_integer &shrinkerator, std::vector<postings_list_object>  &entire_index, size_t *total_decopress_time)
	{
	std::vector<uint32_t> decompressed_postings_list(longest_list, 0);
	size_t term_count = 0;
	
	for (auto &list : entire_index)
		{
		term_count++;
		/*
			Find a list that hasn't yet been compressed and mark it as done.
		*/
		uint8_t should_be = postings_list_object::COMPRESSED;
		if (list.status == should_be)
			if (list.status.compare_exchange_strong(should_be, postings_list_object::DECOMPRESSED))
				{
				auto timer = JASS::timer::start();
				shrinkerator.decode(decompressed_postings_list.data(), list.length, list.compressed_postings_list.data(), list.size_in_bytes_once_compressed);
				auto took = JASS::timer::stop(timer).nanoseconds();

				list.decompress_time = took;
				*total_decopress_time += took;

				//std::cout << list.length << " integers -> " << list.decompress_time << "ns" << std::endl;

				/*
					Verify
				*/
				if (verify)
					if (::memcmp(list.raw, decompressed_postings_list.data(), list.length * sizeof(*list.raw)) != 0)
						{
						for (uint32_t pos = 0; pos < list.length; pos++)
							if (list.raw[pos] != decompressed_postings_list[pos])
								{
								std::ostringstream output;
								output << "List " << term_count << " fail at pos:" << pos << "[" <<  decompressed_postings_list[pos] << "!=" << list.raw[pos] << "]\n";
								std::cout << output.str();
								}

						exit(0);
						}

				/*
					Notify
				*/
				if (term_count % report_every == 0)
					{
					std::ostringstream output;
					output << "Terms Decompressed:" << term_count << '\n';
					std::cout << output.str();
					}
				}
			}
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	try
		{
		/*
			Set up for parsing the command line
		*/
		uint32_t thread_count = 1;
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
				JASS::commandline::parameter("-N", "--report-every", "<n> Report time and memory every <n> documents.", report_every),
				JASS::commandline::note("\nTHREADS\n-------"),
				JASS::commandline::parameter("-t", "--threads", "<n> The number of concurrent threads doing the compression and decompression", thread_count)
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
		if (filename == "" || thread_count < 1)
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

		std::unique_ptr<JASS::compress_integer> shrinkerator = JASS::compress_integer_all::compressor(selectors);
		std::cout << "Check " << JASS::compress_integer_all::name(selectors) << " on file " << filename << '\n';

		/*
			Read the postings list file
		*/
		std::string entire_file;
		JASS::file::read_entire_file(filename, entire_file);
		if (entire_file.size() == 0)
			exit(printf("cannot open %s\n", filename.c_str()));

		uint32_t *file_pointer = reinterpret_cast<uint32_t *>(entire_file.data());
		uint32_t *end_of_file = file_pointer + entire_file.size() / sizeof(uint32_t);

		std::cout << "Read\n";

		/*
			Iterate through each postings list and create a vector pointing to each list - so that we can compress and decompress in parallel
		*/
		/*
			Count the number of terms
		*/
		uint32_t term_count = 0;
		uint32_t longest_list = 0;
		while (file_pointer < end_of_file)
			{
			longest_list = JASS::maths::maximum(longest_list, *file_pointer);
			file_pointer += 1 + *file_pointer;
			term_count++;
			}

		std::cout << "Counted\n";

		/*
			Allocate space for the postings lists and extract each from the file
		*/
		file_pointer = reinterpret_cast<uint32_t *>(entire_file.data());
		std::vector<postings_list_object> entire_index(term_count);
		term_count = 0;
		while (file_pointer < end_of_file)
			{
			entire_index[term_count].length = *file_pointer;
			entire_index[term_count].raw = file_pointer + 1;

			file_pointer += 1 + *file_pointer;
			term_count++;
			}

		std::cout << "Split\n";

		/*
			Compress the lists
		*/
		const size_t OVERFLOW_AMOUNT = 1024;
		std::vector<std::thread> thread_pool;
		for (size_t which = 0; which < thread_count; which++)
			thread_pool.push_back(std::thread(compress_all_lists, longest_list + OVERFLOW_AMOUNT, std::ref(*shrinkerator), std::ref(entire_index)));

		/*
			Wait until we're done
		*/
		for (auto &thread : thread_pool)
			thread.join();

	std::cout << "Compressed\n";

		/*
			Sync before starting the Deompression
		*/
		std::atomic_thread_fence(std::memory_order_seq_cst);

		/*
			Decompress
		*/
		thread_pool.clear();
		std::vector<size_t> the_thread_time(thread_count);
		for (size_t which = 0; which < thread_count; which++)
			{
			the_thread_time[which] = 0;
			thread_pool.push_back(std::thread(decompress_all_lists, longest_list + OVERFLOW_AMOUNT, std::ref(*shrinkerator), std::ref(entire_index), &the_thread_time[which]));
			}

		/*
			Wait until we're done
		*/
		for (auto &thread : thread_pool)
			thread.join();

	std::cout << "Decompressed\n";

		/*
			Sync before reading the decompresson stats
		*/
		std::atomic_thread_fence(std::memory_order_seq_cst);

		/*
			Compute the longest thread decompression execution time
		*/
		size_t the_longest_time = 0;
		for (size_t which = 0; which < thread_count; which++)
			the_longest_time = JASS::maths::maximum(the_longest_time, the_thread_time[which]);

		/*
			Initialise by setting the count buffers to 0
		*/
		decompress_time.resize(longest_list);
		decompress_count.resize(longest_list);
		compressed_size.resize(longest_list);
		compressed_count.resize(longest_list);

		/*
			Generate the stats
		*/
		for (auto &list : entire_index)
			{
			compressed_size[list.length] += list.size_in_bytes_once_compressed;
			compressed_count[list.length]++;
			decompress_time[list.length] = list.decompress_time;
			decompress_count[list.length]++;
			}

		std::cout << "Total terms:" << term_count << "\n";
		std::cout << "Wall clock decompression time (smaller for more threads):" << the_longest_time << "\n";
		draw_histogram(longest_list);
		}
	catch (...)
		{
		puts("Unexpected exception thrown.");
		exit(1);
		}

	return 0;
	}
