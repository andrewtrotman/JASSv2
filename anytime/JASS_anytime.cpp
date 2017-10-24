/*
	JASS_ANYTIME.CPP
	----------------
	Written bu Andrew Trotman.
	Based on JASS v1, which was written by Andrew Trotman and Jimmy Lin
*/
#include <stdio.h>
#include <stdlib.h>

#include <limits>
#include <fstream>
#include <algorithm>

#include "file.h"
#include "query16_t.h"
#include "decode_d0.h"
#include "run_export.h"
#include "channel_file.h"
#include "compress_integer.h"
#include "JASS_anytime_index.h"
#include "JASS_anytime_stats.h"

#define MAX_QUANTUM 0x0FFF
#define MAX_TERMS_PER_QUERY 1024

/*
	CLASS SEGMENT_HEADER
	--------------------
*/
#pragma pack(push, 1)
class segment_header
	{
	public:
		uint16_t impact;					///< The impact score
		uint64_t offset;					///< Offset (within the postings file) of the start of the compressed postings list
		uint64_t end;						///< Offset (within the postings file) of the end of the compressed postings list
		uint32_t segment_frequency;	///< The number of document ids in the segment (not end - offset because the postings are compressed)
	};
#pragma pack(pop)

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	try
		{
		anytime_stats stats;
		std::string query_filename;

		/*
			Check that we have a query file
		*/
		if (argc > 1)
			query_filename = argv[1];
		else
			exit(printf("query file must be specified (as parameter 1)\n"));

		/*
			Open the index
		*/
		anytime_index index(true);
		index.read_index();

		/*
			Extract the compession scheme as a decoder
		*/
		JASS::compress_integer &decompressor = index.codex();
		JASS::decoder_d0 decoder(index.document_count() + 1024);			// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

		/*
			Set the Anytime stopping criteria
		*/
		size_t postings_to_process = (std::numeric_limits<size_t>::max)();

		/*
			Create the run file
		*/
		std::ofstream output;
		output.open("ranking.txt");

		/*
			Allocate the Score-at-a-Time table
		*/
		uint64_t *segment_order = new uint64_t [MAX_TERMS_PER_QUERY * MAX_QUANTUM];
		uint64_t *current_segment = segment_order;

		/*
			Now start searching
		*/
		JASS::channel_file input(query_filename);		// read from here
		std::string query;									// the channel read goes into here

		input.gets(query);
		while (query.size() != 0)
			{
			stats.number_of_queries++;

			std::cout << "-" << query << "\n";
			JASS::query16_t jass_query(index.primary_keys(), index.document_count(), 10);	// allocate a JASS query object
			jass_query.parse(query);
			auto &terms = jass_query.terms();
			auto query_id = terms[0];

			/*
				Parse the query and extract the list of impact segments
			*/
			current_segment = segment_order;
			size_t term_id = 0;
			for (const auto &term : terms)
				{
				/*
					Count which term we're on (and ignore the first as its the TREC topic ID)
				*/
				term_id++;
				if (term_id == 1)
					continue;

	std::cout << "TERM:" << term << "\n";

				/*
					Get the metadata for this term (and if this term isn't in the vocab them move on to the next term)
				*/
				anytime_index::metadata metadata;
				if (!index.postings_details(metadata, term))
					continue;

				/*
					Add to the list of imact segments that need to be processed
				*/
	std::cout << "    Impacts:" << metadata.impacts << "\n";
				std::copy((uint64_t *)(metadata.offset), (uint64_t *)(metadata.offset) + metadata.impacts, current_segment);
				current_segment += metadata.impacts;
				}

			/*
				Sort the segments from highest impact to lowest impact
			*/
			std::sort
				(
				segment_order,
				current_segment,
				[postings = index.postings()](uint64_t first, uint64_t second)
					{
					segment_header *lhs = (segment_header *)(postings + first);
					segment_header *rhs = (segment_header *)(postings + second);

					/*
						sort from highest to lowest impact, but break ties by placing the lowest quantum-frequency first and the highest quantum-drequency last
					*/
					if (lhs->impact < rhs->impact)
						return false;
					else if (lhs->impact > rhs->impact)
						return true;
					else			// impact scores are the same, so break on the length of the segment
						return lhs->segment_frequency < rhs->segment_frequency;
					}
				);

			/*
				0 terminate the list of segments
			*/
			*current_segment = 0;

			/*
				Process the segments
			*/
			size_t postings_processed = 0;
			for (uint64_t *current = segment_order; current < current_segment; current++)
				{
	std::cout << "Process Segment->(" << ((segment_header *)(index.postings() + *current))->impact << ":" << ((segment_header *)(index.postings() + *current))->segment_frequency << ")\n";
				const segment_header &header = *reinterpret_cast<const segment_header *>(index.postings() + *current);
				/*
					The anytime algorithms basically boils down to this... have we processed enough postings yet?  If so then stop
				*/
				if (postings_processed + header.segment_frequency > postings_to_process)
					break;
				postings_processed += header.segment_frequency;

				/*
					Process the postings
				*/
				uint16_t impact = header.impact;
				decoder.decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
				decoder.process(impact, jass_query);
				}

			JASS::run_export(JASS::run_export::TREC, output, (char *)query_id.token().address(), jass_query, "COMPILED", true);
			input.gets(query);
			std::cout << "-\n";
			}

	output.close();
	}
catch (...)
	{
	std::cout << "Exception";
	}

return 0;
}
