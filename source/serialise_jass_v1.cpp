/*
	SERIALISE_JASS_V1.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "checksum.h"
#include "serialise_jass_v1.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_JASS_V1::~SERIALISE_JASS_V1()
		---------------------------------------
	*/
	serialise_jass_v1::~serialise_jass_v1()
		{
		/*
			Sort then serialise the contents of the CIvocab.bin file.
		*/
		std::sort(index_key.begin(), index_key.end());
		
		/*
			Serialise the contents of CIvocab.bin
		*/
		for (const auto line : index_key)
			{
			vocabulary.write(&line.term, sizeof(line.term));
			vocabulary.write(&line.offset, sizeof(line.offset));
			vocabulary.write(&line.impacts, sizeof(line.impacts));
			}
		}

	/*
		SERIALISE_JASS_V1::OPERATOR()()
		-------------------------------
	*/
	void serialise_jass_v1::operator()(const slice &term, const index_postings &postings)
		{
		uint64_t term_offset = vocabulary_strings.tell();
		/*
			Write the vocabulary term to CIvocab_terms.bin
		*/
		vocabulary_strings.write(term.address(), term.size());
		vocabulary_strings.write("\0", 1);
		
		
		/*
			Keep a copy of the term and the detals of the postings list for later sorting and writeing to CIvocab.bin
		*/
		index_key.push_back(vocab_tripple(term, term_offset, 0, 0));
		}
	/*
		SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
		-----------------------------------------
	*/
	void serialise_jass_v1::operator()(size_t document_id, const slice &primary_key)
		{
		/*
			To do.
		*/
		}

	/*
		SERIALISE_JASS_V1::UNITTEST()
		-----------------------------
	*/
	void serialise_jass_v1::unittest(void)
		{
		/*
			Build an index.
		*/
		index_manager_sequential index;
		index_manager_sequential::unittest_build_index(index);
		
		/*
			Serialise the index.
		*/
		{
		serialise_jass_v1 serialiser;
		index.iterate(serialiser);
		}

		/*
			Checksum the inde to make sure its correct.
		*/
		auto checksum = checksum::fletcher_16_file("CIvocab_terms.bin");
		JASS_assert(checksum == 0x61E1);

		puts("\tserialise_jass_v1::INCOMPLETE - not all implemented");
		}
	}
