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
#include "compress_integer_elias_gamma_simd.h"
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

	if (argc != 2)
		exit(printf("Usage:%s <index.ciff>\n", argv[0]));

	/*
		set up the indexer
	*/
	JASS::index_manager_sequential index;
	size_t total_documents = 0;

	/*
		read the postings lists
	*/
	std::cout << "READ THE CIFF INTO MEMORY\n";
	if ((file_size = JASS::file::read_entire_file(argv[1], file)) == 0)
		exit(printf("Can't read file:%s\n", argv[1]));

	/*
		set up the protobuf reader
	*/
	JASS::ciff_lin source((uint8_t *)&file[0]);

	/*
		go list at a time, adding each one to the index
	*/
	std::cout << "PROCESS THE POSTINGS LISTS\n";
	size_t term_count = 0;
	size_t total_terms = source.get_header().num_postings_lists;
	size_t percent_threshold = total_terms / 100;
	for (auto &posting : source.postings())
		{
		++term_count;
		if (((term_count % (percent_threshold * 5)) == 0) || (term_count == total_terms - 1))
			{
			std::cout << term_count << "/" << total_terms << " " << (term_count * 100) / total_terms << "% : ";
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
		Generate the length vector and the public key vector
	*/
	std::cout << "READ THE DOCUMENT DETAILS\n";
	std::vector<JASS::slice> key_vector;
	std::vector<JASS::compress_integer::integer> document_length_vector;
	document_length_vector.push_back(0);			// docid=0 does not exist in JASSv2.
	JASS::compress_integer::integer last_docid = 0;
	for (auto &docid : source.docrecords())
		{
		document_length_vector.push_back(docid.doclength);
		key_vector.push_back(docid.collection_docid);
		if (last_docid != docid.docid)
			{
			std::cout << "Document IDs must be ordered\n";
			exit(0);
			}
		last_docid = docid.docid + 1;
		}

	/*
		set the length vector and the primary key vector
	*/
	index.set_primary_keys(key_vector);
	index.set_document_length_vector(document_length_vector);

	/*
		quantize the index
	*/
	std::cout << "QUANTIZE THE INDEX\n";
	std::shared_ptr<JASS::ranking_function_atire_bm25> ranker(new JASS::ranking_function_atire_bm25(0.9, 0.4, index.get_document_length_vector()));
	JASS::quantize<JASS::ranking_function_atire_bm25> quantizer(total_documents, ranker);
	index.iterate(quantizer);

	/*
		Decode the export formats and encode into a vector before writing the index
	*/
	std::cout << "WRITE THE INDEX TO DISK\n";
	std::vector<std::unique_ptr<JASS::index_manager::delegate>> exporters;
	exporters.push_back(std::make_unique<JASS::serialise_jass_v1>(total_documents, JASS::serialise_jass_v1::jass_v1_codex::elias_gamma_simd, 1));
	quantizer.serialise_index(index, exporters);

	std::cout << "DONE\n";
	return 0;
	}
