/*
	DESERIALISED_JASS_V1.H
	----------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "file.h"
#include "slice.h"
#include "compress_integer_all.h"
#include "deserialised_jass_v1.h"

namespace JASS
	{
	/*
		DESERIALISED_JASS_V1::READ_PRIMARY_KEYS()
		-----------------------------------------
	*/
	size_t deserialised_jass_v1::read_primary_keys(const std::string &filename)
		{
		/*
			This can take some time so make some noise when we start
		*/
		if (verbose)
			{
			printf("Loading doclist... ");
			fflush(stdout);
			}

		/*
			Read the disk file
		*/
		auto bytes = file::read_entire_file(filename, primary_key_memory);
		if (bytes == 0)
			return 0;					// failed to read the file.

		/*
			Numnber of documents is stored at the end of the file (as a uint64_t)
		*/
		documents = *reinterpret_cast<uint64_t *>(&primary_key_memory[bytes] - sizeof(uint64_t));
		primary_key_list.reserve(documents);

		/*
			The file is in 2 parts, the first is the primary key the second is the poiters to the primary keys
		*/
		uint64_t *offset_base = (uint64_t *) (&primary_key_memory[0] + bytes - (documents * sizeof(uint64_t) + sizeof(uint64_t)));

		/*
			Now work through each primary key adding it to the list of primary keys
		*/
		for (size_t id = 0; id < documents; id++)
			primary_key_list.push_back(&primary_key_memory[0] + offset_base[id]);

		/*
			This can take some time so make some noise when we're finished
		*/
		if (verbose)
			{
			puts("done");
			fflush(stdout);
			}

		/*
			retrurn the number of documents in the collection
		*/
		return documents;
		}

	/*
		DESERIALISED_JASS_V1::READ_VOCABULARY()
		---------------------------------------
	*/
	size_t deserialised_jass_v1::read_vocabulary(const std::string &vocab_filename, const std::string &terms_filename)
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
		char *vocab = &vocabulary_memory[0];

		/*
			Read the file of strings that is the vocabulary
		*/
		auto bytes = file::read_entire_file(terms_filename, vocabulary_terms_memory);
		if (bytes == 0)
			return 0;
		terms = length / (sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t));
		char *vocab_terms = &vocabulary_terms_memory[0];

		/*
			Build the vocabulary
		*/
		vocabulary_list.reserve(terms);
		const char *postings_base = &postings_memory[0];
		for (size_t term = 0; term < terms; term++)
			{
			uint64_t *base = reinterpret_cast<uint64_t *>(vocab + (3 * sizeof(uint64_t)) * term);

			vocabulary_list.push_back(metadata(slice(vocab_terms + base[0]), postings_base + base[1], base[2]));
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
			Return the numner of terms in the collection
		*/
		return terms;
		}

	/*
		DESERIALISED_JASS_V1::READ_POSTINGS()
		-------------------------------------
	*/
	size_t deserialised_jass_v1::read_postings(const std::string &filename)
		{
		/*
			This can take some time so make some noise when we start
		*/
		if (verbose)
			{
			printf("Loading postings... ");
			fflush(stdout);
			}
		/*
			Read the postings
		*/
		auto bytes = file::read_entire_file(filename, postings_memory);
		if (bytes == 0)
			return 0;

		/*
			This can take some time so make some noise when we're finished
		*/
		if (verbose)
			{
			puts("done");
			fflush(stdout);
			}

		/*
			Return the size of the posings (in bytes)
		*/
		return bytes;
		}

	/*
		DESERIALISED_JASS_V1::READ_INDEX()
		----------------------------------
	*/
	size_t deserialised_jass_v1::read_index(const std::string &primary_key_filename, const std::string &vocab_filename, const std::string &terms_filename, const std::string &postings_filename)
		{
		if (read_primary_keys(primary_key_filename) != 0)
			if (read_postings(postings_filename) != 0)
				if (read_vocabulary(vocab_filename, terms_filename) != 0)
					return 1;

		return 0;
		}

	/*
		DESERIALISED_JASS_V1::CODEX()
		-----------------------------
	*/
	compress_integer &deserialised_jass_v1::codex(std::string &name) const
		{
		if (postings_memory.size() == 0)
			{
			name = "None";
			return compress_integer_all::get_by_name("None");
			}
		else
			switch (postings_memory[0])
				{
				case 's':
					name = "None";
					return compress_integer_all::get_by_name("None");
				case 'q':
					name = "QMX JASS v1";
					return compress_integer_all::get_by_name("QMX JASS v1");
				default:
					exit(printf("Unknown index format\n"));
					/*
						// This can't happen because of the exit() above, and cppcheck warns avout it.
						name = "None";
						return compress_integer_all::get_by_name("None");
					*/
				}
		}
	}
