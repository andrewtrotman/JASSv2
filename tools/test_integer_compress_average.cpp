/*
	TEST_INTEGER_COMPRESS_AVERAGE.CPP
	---------------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*
	test_integer_compress produces files in the format:

	length DecompressTimeInNanoseconds CompressedSizeInBytes
	100 529 320
	101 551 324
	102 546 329
	103 559 335

	Where each column is seperated by a space.

	This program takes a set of input files specified on the command line and outputs the minimum, mean, median, maximum, and raw data of the scores in the input file.
	The output is written to stdout, in the assumption that it will then be redirected to some file elsewhere.

	The format of the output is:

		length Bytes MinNano MeanNano MedNano MaxNano SortedRawNano...
		100 358 348 371 361 416 348 356 366 416
		101 360 342 369 354 428 342 351 357 428
		102 361 340 369 366 405 340 361 371 405
		103 363 338 359 346 406 338 346 347 406

	where Bytes is the size in bytes and all others are times in nanoseconds.  SortedRawNano is the times for each length, sorted from smallest to largest for that row,
	so the columns are not individual runs.
*/
#include <stdio.h>
#include <stdint.h>

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cinttypes>

#include "file.h"

/*
	MAIN_EVENT()
	------------
*/
int main_event(int argc, const char *argv[])
	{
	std::map<size_t, std::vector<size_t>> times_in_nanoseconds;
	std::map<size_t, size_t> size_in_bytes;

	for (int parameter = 1; parameter < argc; parameter++)
		{
		std::string raw_data;
		std::vector<uint8_t *> line_data;
		JASS::file::read_entire_file(argv[parameter], raw_data);
		JASS::file::buffer_to_list(line_data, raw_data);

		/*
			Read in the data, skipping over the first line as that's the header
		*/
		for (const auto &line : line_data)
			{
			/*
				Skip over all the headings.
			*/
			if (!isdigit(*line))
				continue;

			uint64_t length;
			uint64_t nanoseconds;
			uint64_t bytes;

			sscanf(reinterpret_cast<char *>(line), "%" PRId64 "%" PRId64  "%" PRId64, &length, &nanoseconds, &bytes);
			size_in_bytes[length] = bytes;
			times_in_nanoseconds[length].push_back(nanoseconds);
			}
		}

	/*
		Dump out the data
	*/
	std::cout << "length Bytes MinNano MeanNano MedNano MaxNano SortedRawNano...\n";
	for (auto line : times_in_nanoseconds)
		{
		/*
			sort it
		*/
		std::sort(line.second.begin(), line.second.end());

		/*
			Compute the sum (for the mean)
		*/
		size_t total = 0;
		for (auto timing : line.second)
			total += timing;

		std::cout << line.first << ' ' << size_in_bytes[line.first] << ' ' << line.second[0] << ' ' << static_cast<size_t>(total / line.second.size()) << ' ';

		/*
			Compute the median
		*/
		if ((line.second.size() & 0x01) == 0)
			std::cout << ((line.second[line.second.size() / 2 - 1] + line.second[line.second.size() / 2]) >> 1);
		else
			std::cout << line.second[line.second.size() / 2];
		std::cout << ' ' << line.second[line.second.size() - 1];

		/*
			Dump the raw (sorted) data
		*/
		for (auto timing : line.second)
			std::cout << ' ' << timing;

		std::cout << '\n';
		}

	return 0;
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	try
		{
		return main_event(argc, argv);
		}
	catch (...)
		{
		return printf("Unhandled Exception");
		}
	}
