/*
	DUMP_CIFF_LIN.CPP
	-----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Read and dump an index in Jimmy Lin's Common Index File Format.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#else
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <iostream>
#include <string.h>

#include "file.h"
#include "maths.h"
#include "ciff_lin.h"
#include "quantize.h"
#include "compress_integer.h"
#include "serialise_jass_v1.h"
#include "index_manager_sequential.h"
#include "ranking_function_atire_bm25.h"

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::string file;
	size_t file_size;

	if (argc != 4)
		exit(printf("Usage:%s <index.pb> <docids.txt> <doclengths.txt>\n", argv[0]));

	/*
		set up the indexer
	*/
	JASS::index_manager_sequential index;
	size_t total_documents = 0;

	/*
		read the primary keys from a text file
	*/
	std::string primary_keys;
	JASS::file::read_entire_file(argv[2], primary_keys);
	std::vector<uint8_t *> line_list;
	JASS::file::buffer_to_list(line_list, primary_keys);
	std::vector<JASS::slice> key_vector;
//	key_vector.push_back("JASS DocID=0: Invalid");		// Postings count from 1, but primary keys count from 0 (so DO NOT EXECUTE THIS LINE).
	for (auto &line : line_list)
		key_vector.push_back(JASS::slice((const char *)line));
	index.set_primary_keys(key_vector);

	/*
		read the document length vector
		the document lengths are <docid>\t<length> formated, counting from 0.  For example:
		0 504
		1 168
		2 600
	*/
	std::vector<JASS::compress_integer::integer> document_length_vector;
	std::string lengths;
	JASS::file::read_entire_file(argv[3], lengths);
	std::vector<uint8_t *> length_line_list;
	JASS::file::buffer_to_list(length_line_list, lengths);
	JASS::compress_integer::integer last_docid = 0;
	document_length_vector.push_back(0);			// docid=0 does not exist in JASSv2.
	for (const auto &line : length_line_list)
		{
		char *line_as_char_star = reinterpret_cast<char *>(line);
		JASS::compress_integer::integer docid = atoll(line_as_char_star);
		docid++;
		if (docid != last_docid + 1)
			{
			std::cout << "Document ID missing where " << docid << " was found.\n";
			exit(1);
			}
		char *length_field = strchr(line_as_char_star, '\t');
		if (length_field == nullptr)
			{
			std::cout << "Broken length file for docid=" << docid << ".\n";
			exit(1);
			}
		while (isspace(*length_field))
			length_field++;
		JASS::compress_integer::integer document_length = atoll(length_field);
		document_length_vector.push_back(document_length);
		last_docid = docid;
		}

	/*
		read the postings lists
	*/
	if ((file_size = JASS::file::read_entire_file(argv[1], file)) == 0)
		exit(printf("Can't read file:%s\n", argv[1]));

	/*
		set up the protobuf reader
	*/
	JASS::ciff_lin source((uint8_t *)&file[0], file_size);

	/*
		go list at a time, adding each one to the index
	*/
	size_t term_count = 0;
	for (auto &posting : source)
		{
		if ((++term_count % 1024) == 0)
			{
			std::cout.write((char *)posting.term.address(), posting.term.size());
			std::cout << " " << posting.document_frequency << " " << posting.collection_frequency << ":\n";
			}

		/*
			CIFF counts from documentID = 0, but JASS indexing counts from documentID = 1, so we increment the fist docid (as the others are d-gaps).
		*/
		posting.postings[0].docid++;
		size_t cumulative_total = 0;

		for (const auto &pair : posting.postings)
			{
			cumulative_total += pair.docid;
			total_documents = JASS::maths::maximum(cumulative_total, total_documents);
			}

		JASS::parser::token term;
		term.set(posting.term);
		index.term(term, posting.postings);
		}

	/*
		check that we got to the end
	*/
	if (source.status == JASS::ciff_lin::FAIL)
		exit(printf("File is not in the correct format\n"));

	/*
		set the length vector
	*/
	index.set_document_length_vector(document_length_vector);

	/*
		free up memory
	*/
	primary_keys = "";
	line_list.clear();
	key_vector.clear();
	document_length_vector.clear();
	length_line_list.clear();
	lengths = "";
	file = "";

	/*
		quantize the index
	*/
	std::shared_ptr<JASS::ranking_function_atire_bm25> ranker(new JASS::ranking_function_atire_bm25(0.9, 0.4, index.get_document_length_vector()));
	JASS::quantize<JASS::ranking_function_atire_bm25> quantizer(total_documents, ranker);
	index.iterate(quantizer);

	/*
		Decode the export formats and encode into a vector before writing the index
	*/
	std::vector<std::unique_ptr<JASS::index_manager::delegate>> exporters;
	exporters.push_back(std::make_unique<JASS::serialise_jass_v1>(total_documents));
	quantizer.serialise_index(index, exporters);

	return 0;
	}
