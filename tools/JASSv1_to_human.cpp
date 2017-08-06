/*
	JASSV1_TO_HUMAN.CPP
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/

#include <stdio.h>
#include <stdint.h>

#include <iostream>

#include "file.h"

/*
	STRUCT VOCAB_TRIPPLE
	--------------------
*/
struct vocab_tripple
	{
	uint64_t term;
	uint64_t offset;
	uint64_t impacts;
	};

/*
	DUMP_DOCLIST()
	--------------
*/
void dump_doclist(void)
	{
	std::string doclist;
	uint64_t unique_documents;

	JASS::file::read_entire_file("CIdoclist.bin", doclist);
	unique_documents = *reinterpret_cast<const uint64_t *>(doclist.c_str() + doclist.size() - sizeof(uint64_t));

	const uint64_t *offset_base = reinterpret_cast<const uint64_t *>(doclist.c_str() + doclist.size() - (unique_documents * sizeof(uint64_t) + sizeof(uint64_t)));

	for (uint64_t id = 0; id < unique_documents; id++)
		std::cout << doclist.c_str() + offset_base[id] << '\n';
	}

/*
	MAIN()
	------
*/
int main(void)
	{
	std::string vocab_buffer;
	JASS::file::read_entire_file("CIvocab.bin", vocab_buffer);
	const vocab_tripple *vocab = reinterpret_cast<const vocab_tripple *>(vocab_buffer.c_str());
	auto vocab_length = vocab_buffer.size() / sizeof(vocab_tripple);


	std::string strings;
	JASS::file::read_entire_file("CIvocab_terms.bin", strings);

	std::cout << "POSTINGS LISTS\n--------------\n";
	for (auto term = 0; term < vocab_length; term++)
		std::cout << strings.c_str() + vocab[term].term << '\n';

	std::cout << "\nPRIMARY KEY LIST\n----------------\n";
	dump_doclist();

	return 0;
	}
