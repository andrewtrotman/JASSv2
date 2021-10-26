/*
	DESERIALISED_JASS_V2.CPP
	------------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "deserialised_jass_v2.h"

namespace JASS
	{
	/*
		DESERIALISED_JASS_V2::READ_VOCABULARY()
		---------------------------------------
	*/
	size_t deserialised_jass_v2::read_vocabulary(const std::string &vocab_filename, const std::string &terms_filename)
		{
		/*
			This can take some time so make some noise when we start
		*/
		if (verbose)
			{
			printf("Loading vocab... ");
			fflush(stdout);
			}
		/*
			Read the file of tripples that are the pointers to the terms (and the postings too)
		*/
		auto length = file::read_entire_file(vocab_filename, vocabulary_memory);
		if (length == 0)
			return 0;
		const uint8_t *vocab;
		vocabulary_memory.read_entire_file(vocab);

		/*
			Read the file of strings that is the vocabulary
		*/
		auto bytes = file::read_entire_file(terms_filename, vocabulary_terms_memory);
		if (bytes == 0)
			return 0;
		const uint8_t *vocab_terms;
		vocabulary_terms_memory.read_entire_file(vocab_terms);

		/*
			Build the vocabulary
		*/
		terms = 0;
		const uint8_t *postings_base = postings();
		const uint8_t *from = vocab;
		while (from < vocab + length)
			{
			uint64_t term_pointer;
			uint64_t postings_pointer;
			uint64_t impact;

			compress_integer_variable_byte::decompress_into(&term_pointer, from);
			compress_integer_variable_byte::decompress_into(&postings_pointer, from);
			compress_integer_variable_byte::decompress_into(&impact, from);

			vocabulary_list.push_back(metadata(slice(reinterpret_cast<const char*>(vocab_terms + term_pointer)), postings_base + postings_pointer, impact));
			terms++;
			}

		/*
			This can take some time so make some noise when we're finished
		*/
		if (verbose)
			{
			puts("done");
			fflush(stdout);
			}

		/*
			Return the number of terms in the collection
		*/
		return terms;
		}
	}
