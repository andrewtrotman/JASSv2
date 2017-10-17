/*
	JASS_ANYTIME.CPP
	----------------
	Written by Andrew Trotman and Jimmy Lin
*/
#include <stdio.h>
#include <stdlib.h>

#include "file.h"
#include "query16_t.h"
#include "channel_file.h"
#include "JASS_anytime_index.h"
#include "JASS_anytime_stats.h"

#define MAX_QUANTUM 0x0FFF
#define MAX_TERMS_PER_QUERY 1024

size_t CI_results_list_length = 0;
uint8_t *postings;


void trec_dump_results(uint32_t topic_id, FILE *out, uint32_t output_length)
	{
#ifdef NEVER
	for (size_t current = 0; current < (output_length < CI_results_list_length ? output_length : CI_results_list_length); current++)
		{
		size_t id = CI_accumulator_pointers[current] - CI_accumulators;
		fprintf(out, "%d Q0 %s %d %d COMPILED (ID:%u)\n", topic_id, CI_documentlist[id], current + 1, CI_accumulators[id], id);
		}
#endif
	}

class CI_quantum_header
	{
	public:
		uint16_t impact;
		uint64_t offset;
		uint64_t end;
		uint32_t quantum_frequency;
	} __attribute__((packed));

int quantum_compare(const void *a, const void *b)
	{
	CI_quantum_header *lhs = (CI_quantum_header *) ((*(uint64_t *) a) + postings);
	CI_quantum_header *rhs = (CI_quantum_header *) ((*(uint64_t *) b) + postings);

	/*
		sort from highest to lowest impact, but break ties by placing the lowest quantum-frequency first and the highest quantum-drequency last
	*/
	return lhs->impact > rhs->impact ? -1 : lhs->impact < rhs->impact ? 1 :
		lhs->quantum_frequency > rhs->quantum_frequency ? 1 :
		lhs->quantum_frequency == rhs->quantum_frequency ? 0 : -1;
	}



/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
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
	JASS::compress_integer &decoder = index.codex();

	/*
		Create the run file
	*/
	FILE *out;
	if ((out = fopen("ranking.txt", "w")) == NULL)
		exit(printf("Can't open output file.\n"));

	/*
		Allocate the quantum at a time table
	*/
	auto quantum_order = new uint64_t[MAX_TERMS_PER_QUERY * MAX_QUANTUM];

	/*
		Now start searching
	*/
	JASS::channel_file input(query_filename);						// read from here
	std::string query;													// the channel read goes into here

	input.gets(query);
	while (query.size() != 0)
		{
		stats.number_of_queries++;
		
		std::cout << "-" << query << "\n";
		JASS::query16_t jass_query(index.primary_keys(), index.document_count(), 10);	// allocate a JASS query object
		jass_query.parse(query);
		auto &terms = jass_query.terms();
		auto query_id = terms[0];

		size_t term_id = 0;
		for (const auto &term : terms)
			{
			term_id++;
			if (term_id == 1)
				continue;
			std::cout << "TERM:" << term << "\n";
			auto postings = index.get_postings_details(term);
			}
		input.gets(query);
		std::cout << "-\n";
		}

#ifdef NEVER

CI_results_list_length = 0;


max_remaining_impact = 0;
current_quantum = quantum_order;
early_terminate = false;

while ((term = strtok(NULL, SEPERATORS)) != NULL) {
postings_list = (CI_vocab_heap *) bsearch(term, CI_dictionary, CI_unique_terms, sizeof(*CI_dictionary), CI_vocab_heap::compare_string);

// Initializee the SaaT structures
if (postings_list != NULL) {
// Copy this term's pointers to the segments list
memcpy(current_quantum, postings + postings_list->offset, postings_list->impacts * sizeof(*quantum_order));

// Compute the maximum possibe impact score (that is, assume one document has the maximum impact for each term)
max_remaining_impact += ((CI_quantum_header *) (postings + *current_quantum))->impact;

// Advance to the place we want to place the next set of segments
current_quantum += postings_list->impacts;
}
}
/*
NULL termainate the list of quantums
*/
*current_quantum = 0;

/*
Sort the quantum list from highest to lowest
*/
qsort(quantum_order, current_quantum - quantum_order, sizeof(*quantum_order), quantum_compare);

/*
Now process each quantum, one at a time
*/

postings_processed = 0;
for (current_quantum = quantum_order; *current_quantum != 0; current_quantum++) {
current_header = (CI_quantum_header *) (postings + *current_quantum);
if (postings_processed + current_header->quantum_frequency > postings_to_process) {
break;
}

stats_quantum_count++;
(*process_postings_list)(postings + current_header->offset, postings + current_header->end,
current_header->impact, current_header->quantum_frequency);
postings_processed += current_header->quantum_frequency;
}


// Sort the accumulator pointers to put the highest RSV document at the top of the list.
top_k_qsort(CI_accumulator_pointers, CI_results_list_length, CI_top_k - 1);

/*
At this point we know the number of hits (CI_results_list_length) and they can be decode out of the CI_accumulator_pointers array
where CI_accumulator_pointers[0] points into CI_accumulators[] and therefore CI_accumulator_pointers[0] - CI_accumulators is the docid
and *CI_accumulator_pointers[0] is the rsv.
*/
end_timer = chrono::steady_clock::now();
stats_tmp = chrono::duration_cast<chrono::nanoseconds>(end_timer - full_query_without_io_timer);
stats_total_time_to_search_without_io += stats_tmp;


// Dump TREC output
trec_dump_results(query_id, out, CI_top_k - 1);	// Subtract 1 from top_k because we added 1 for the early termination checks.
}
}

fclose(out);
fclose(fp);

end_timer = chrono::steady_clock::now();
stats_total_time_to_search += chrono::duration_cast<chrono::nanoseconds>(end_timer - full_query_timer);
print_os_time();

printf("\nAverages over %llu queries\n", total_number_of_topics);
printf("--------------------------------------------------------\n");
printf("Total time excluding I/O   (per query) : %12llu ns\n", stats_total_time_to_search_without_io.count() / total_number_of_topics);
printf("--------------------------------------------------------\n");
printf("Total running time                     : %12llu ns\n", stats_total_time_to_search.count());
printf("--------------------------------------------------------\n");
printf("Total number of early terminate checks : %12llu\n", stats_quantum_check_count);
printf("Total number of early terminations     : %12llu\n", stats_early_terminations);
printf("Total number of segments processed     : %12llu\n", stats_quantum_count);
printf("--------------------------------------------------------\n");
#endif


return 0;
}
