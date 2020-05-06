/*
	EXAMINE_INDEX.CPP
	-----------------
	Written by Andrew Trotman.

	Pass over an index and compute statistis for it.
*/
#include <maths.h>

#include <map>
#include <iostream>

#include "maths.h"
#include "deserialised_jass_v1.h"

std::map<size_t, size_t> global_frequencies;
std::map<size_t, size_t> mean_magnitude;
std::map<size_t, size_t> median_magnitude;
std::map<size_t, std::map<size_t, size_t>> median_with_exceptions;
size_t total_integers = 0;
double percentile = 90.0;

template <typename DECODER>
void process_one_list(const JASS::slice &term, JASS::deserialised_jass_v1 &index, const JASS::deserialised_jass_v1::segment_header &header, DECODER &postings)
	{
	/*
		Walk the postings list computing the width of each posting.
	*/
	std::map<size_t, size_t> local_frequencies;

//	std::cout << term << "->" << header.impact;

//	char seperator = ':';
	size_t count = 0;
	size_t sum_of_bitness = 0;
	for (const auto docid : postings)
		{
		/*
			Docid 0 takes 1 bit to store
		*/
		size_t bitness;

		if (docid == 0)
			bitness = 1;
		else
			bitness = JASS::maths::ceiling_log2(docid);
		local_frequencies[bitness]++;
		sum_of_bitness += bitness;
		count++;
//		std::cout << seperator << docid;
//		seperator = ',';
		}
//	std::cout << '\n';

	total_integers += count;

	if (count == 0)
		exit(printf("Found an invalid list of length 0!"));

	/*
		add the the frequency count of lists of this length
	*/
	global_frequencies[count]++;

	/*
		Compute the mean width in this list (rounded up to the nearest integer).
	*/
	mean_magnitude[static_cast<size_t>(ceil(static_cast<double>(sum_of_bitness)/static_cast<double>(count)))]++;

	/*
		Compute the median
	*/
	size_t largest = 0;
	size_t sum = 0;
	size_t exception = 0;
	for (const auto &value : local_frequencies)
		{
		sum += value.second;
		if (value.second > local_frequencies[largest])
			largest = value.first;
		if (sum >= (percentile / 100.0) * count)
			exception = value.first;
		}

//	if (count > 128)
		{
		median_magnitude[largest]++;
		if (exception > largest)
			median_with_exceptions[largest][exception]++;
		}
	}

template <typename DECODER>
void process(JASS::deserialised_jass_v1 &index, JASS::compress_integer &decompressor)
	{
	auto decoder = std::make_unique<DECODER>(index.document_count() + 4096);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	/*
		Iterate over the vocabularay
	*/
	for (const auto &metadata : index)
		{
		/*
			Iterate over each segment of the postings list
		*/
		for (size_t impact = 0; impact < metadata.impacts; impact++)
			{
			const uint64_t *current_impact = (reinterpret_cast<const uint64_t *>(metadata.offset) + impact);
			const JASS::deserialised_jass_v1::segment_header &header = *reinterpret_cast<const JASS::deserialised_jass_v1::segment_header *>(index.postings() + *current_impact);

			/*
				Decompress the postings
			*/
			decoder->decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);

			/*
				Process the postings segment
			*/
			process_one_list<DECODER>(metadata.term, index, header, *decoder);
			}
		}
	}

void dump_tables(void)
	{
//	std::cout << "List Lengths\n";
//	for (const auto element : global_frequencies)
//		std::cout << element.first << ":" << element.second << "\n";

	std::cout << "\nMeans\n";
	for (const auto &element : mean_magnitude)
		std::cout << element.first << ":" << element.second << "\n";

	std::cout << "\nMedians\n";
	for (const auto &element : median_magnitude)
		std::cout << element.first << ":" << element.second << "\n";


	std::cout << "\nMedians (row) and exceptions (column)\n";

	size_t max_bitness = 18;
	printf("   ");
	for (size_t median = 0; median <= max_bitness; median++)
		printf("%7zu ", median);
	std::cout << "\n";

	for (size_t median = 0; median <= max_bitness; median++)
		{
		printf("%2zu:", median);
		for (size_t exception = 0; exception <= max_bitness; exception++)
			printf("%7lu ", (unsigned long)median_with_exceptions[median][exception]);

		std::cout << "\n";
		}
	}

int main(void)
	{
	/*
		Read the index
	*/
	JASS::deserialised_jass_v1 index(true);
	index.read_index();

	std::string codex_name;
	int32_t d_ness;
	JASS::compress_integer &decompressor = index.codex(codex_name, d_ness);
	std::cout << codex_name << "\n";

#ifdef NEVER
	auto d_ness = codex_name == "None" ? 0 : 1;
	if (d_ness == 0)
		process<JASS::decoder_d0>(index, decompressor);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow
	else
		process<JASS::decoder_d0>(index, decompressor);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow
#else
		process<JASS::decoder_d0>(index, decompressor);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow
#endif

	dump_tables();
	return 0;
	}
