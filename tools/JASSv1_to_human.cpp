/*
	JASSV1_TO_HUMAN.CPP
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@brief Dump a human-readable version of a JASS v1 index to standard out.
*/

#include <stdio.h>
#include <stdint.h>

#include <iostream>

#include "file.h"

/*
	STRUCT VOCAB_TRIPPLE
	--------------------
*/
/*!
	@brief A tripple representing the vocabulart structure of JASS v1
*/
#pragma pack(push, 1)
struct vocab_tripple
	{
	uint64_t term;			///< pointer to term in the term file
	uint64_t offset;		///< pointer to the postings list.
	uint64_t impacts;		///< The number of impacts for this term
	};
#pragma pack(pop)

/*
	STRUCT IMPACT_HEADER
	--------------------
*/
/*!
	@brief The layout (in memory) of each impact header.
*/
#pragma pack(push, 1)
struct impact_header
	{
	uint16_t impact_score;			///< The impact score for this sequence.
	uint64_t start;					///< Pointer (from start of file) to the postings for this impact.
	uint64_t finish;					///< Pointer (from start of file) to the end of the postings for this term.
	uint32_t documents;					///< Number of documents with this impact score
	};
#pragma pack(pop)

/*
	DUMP_DOCLIST()
	--------------
*/
/*!
	@brief Dump the list of external document IDs (i.e. Primary Keys). 
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
	DUMP_POSTINGS_LIST()
	--------------------
*/
/*!
	@brief Dump the postings list in a human readable format.
	@param file [in] The in-memory index file.
	@param offset [in] The location (from the start of the file) of the postings list.
*/
void dump_postings_list(const char *file, size_t offset, size_t number_of_impacts)
	{
	auto header_offset = *reinterpret_cast<const uint64_t *>(file + offset);
	auto header = reinterpret_cast<const impact_header *>(file + header_offset);

	for (size_t current = 0; current < number_of_impacts; current++)
		{
		std::cout << header->impact_score << ":";
		for (const uint32_t *document_id = reinterpret_cast<const uint32_t *>(file + header->start); reinterpret_cast<const char *>(document_id) < file + header->finish; document_id++)
			{
			std::cout << *document_id << ' ';
			}
		header++;
		}
	}

/*
	MAIN()
	------
*/
/*!
	@brief Dump a human-readable version of a JASS v1 index to standard out.
*/
int main(void)
	{
	std::string vocab_buffer;
	JASS::file::read_entire_file("CIvocab.bin", vocab_buffer);
	const vocab_tripple *vocab = reinterpret_cast<const vocab_tripple *>(vocab_buffer.c_str());
	auto vocab_length = vocab_buffer.size() / sizeof(vocab_tripple);

	std::string postings;
	JASS::file::read_entire_file("CIpostings.bin", postings);


	std::string strings;
	JASS::file::read_entire_file("CIvocab_terms.bin", strings);

	std::cout << "POSTINGS LISTS\n--------------\n";
	for (size_t term = 0; term < vocab_length; term++)
		{
		std::cout << strings.c_str() + vocab[term].term << "->";
		dump_postings_list(postings.c_str(), vocab[term].offset, vocab[term].impacts);
		std::cout << '\n';
		}

	std::cout << "\nPRIMARY KEY LIST\n----------------\n";
	dump_doclist();

	return 0;
	}
