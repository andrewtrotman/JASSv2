/*
	SERIALISE_FORWARD_INDEX.CPP
	---------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <ostream>

#include "slice.h"
#include "version.h"
#include "checksum.h"
#include "index_postings.h"
#include "serialise_forward_index.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_FORWARD_INDEX::SERIALISE_FORWARD_INDEX()
		--------------------------------------------------
	*/
	serialise_forward_index::serialise_forward_index(size_t documents) :
		documents(documents),
		terms(0),
		document(documents + 1)
		{
		}

	/*
		SERIALISE_FORWARD_INDEX::~SERIALISE_FORWARD_INDEX()
		---------------------------------------------------
	*/
	serialise_forward_index::~serialise_forward_index()
		{
		file outfile("forward.xml", "w+b");

		for (const auto &current : document)
			{
			const std::string &one = current.str();
			outfile.write(one.c_str(), one.size());
			outfile.write("\n", 1);
			}
		}

	/*
		SERIALISE_FORWARD_INDEX::OPERATOR()()
		-------------------------------------
	*/
	void serialise_forward_index::operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
		{
		auto end = document_ids + document_frequency;
		auto current_tf = term_frequencies;
		for (compress_integer::integer *current_id = document_ids; current_id < end; current_id++, current_tf++)
			document[*current_id] << term << ":" << static_cast<int>(*current_tf) << "\n";
		}
		
	/*
		SERIALISE_FORWARD_INDEX::DELEGATE::OPERATOR()()
		-----------------------------------------------
	*/
	void serialise_forward_index::operator()(size_t document_id, const slice &primary_key)
		{
		}

	/*
		SERIALISE_FORWARD_INDEX::UNITTEST()
		-----------------------------------
	*/
	void serialise_forward_index::unittest(void)
		{
		/*
			Build an index.
		*/
		index_manager_sequential index;
		index_manager_sequential::unittest_build_index(index, unittest_data::ten_documents);
		
		/*
			Serialise the index.
		*/
		{
		serialise_forward_index serialiser(index.get_highest_document_id());
		index.iterate(serialiser);
		}

		/*
			Checksum the index to make sure its correct.
		*/
		auto checksum = checksum::fletcher_16_file("forward.xml");
//		std::cout << "forward.xml" << checksum << '\n';
		JASS_assert(checksum == 16005);

		puts("serialise_forward_index::PASSED");
		}
	}
