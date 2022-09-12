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
		index_manager::delegate(documents),
		document(documents + 1)
		{
		}

	/*
		SERIALISE_FORWARD_INDEX::FINISH()
		---------------------------------
	*/
	void serialise_forward_index::finish(void)
		{
		file outfile("JASS_forward.index", "w+b");

		int64_t which_document_id = -1;			// start with -1 so that the first document id == 0
		for (const auto &current : document)
			{
			which_document_id++;

			/*
				If the document is empty them move on to the next one
			*/
			const std::string &one = current.str();
			if (one.size() == 0)
				continue;

			/*
				Output the document
			*/
			char document_id[64];			// will be smaller than a 64 digit number (as must be a 64-bit  integer)
			sprintf(document_id, "%lld", static_cast<long long>(which_document_id));
			outfile.write("<DOC><DOCNO>", 12);
			outfile.write(document_id, strlen(document_id));
			outfile.write("</DOCID>", 8);
			outfile.write(one.c_str(), one.size());
			outfile.write("</DOC>\n", 7);
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
			document[*current_id] << term << ":" << static_cast<int>(*current_tf) << " ";
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
		serialiser.finish();
		}

		/*
			Checksum the index to make sure its correct.
		*/
		auto checksum = checksum::fletcher_16_file("JASS_forward.index");
//		std::cout << "JASS_forward.index " << checksum << '\n';
		JASS_assert(checksum == 24427);

		puts("serialise_forward_index::PASSED");
		}
	}
