/*
	COMPILED_INDEX.CPP
	------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include <stdint.h>

#include "allocator_pool.h"
#include "channel_file.h"
#include "parser_query.h"
#include "accumulator_2d.h"
#include "JASS_vocabulary.h"

JASS::accumulator_2d<uint16_t> accumulators(1024);

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	/*
		Sort the dictionary - because it was probably generated in the
		order of the hash-table which isn't alphabetical.
	*/
	std::sort(&dictionary[0], &dictionary[dictionary_length]);

	/*
		Use a JASS channel to read the inpiut query
	*/
	JASS::channel_file input;
	JASS::channel_file output;
	std::string query;

	do
		{
		JASS::allocator_pool memory;
		JASS::parser_query parser(memory);
		JASS::query_term_list parsed_query(memory);

		std::cout << "]";
		input.gets(query);
		output << query;
		parser.parse(parsed_query, query);

		for (const auto &term : parsed_query)
			{
			output << term;
			auto low = std::lower_bound (&dictionary[0], &dictionary[dictionary_length], JASS_ci_vocab(term.token()));

			bool found = (low != &dictionary[dictionary_length] && !(JASS_ci_vocab(term.token()) < *low));

			if (found)
				{
				output << "[" << low->term << "]\n";
				low->method();
				}
			else
				output << "NotFound\n";
			}
		}
	while (query.compare(0, 5, ".quit"));

	return 0;
	}

/*
	ADD_RSV()
	---------
	Add to the accumulators keeping track of the top-k
*/
void add_rsv(size_t document_id, uint16_t weight)
	{
	accumulators[document_id] += weight;
	}
