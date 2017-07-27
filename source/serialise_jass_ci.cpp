/*
	SERIALISE_JASS_CI.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "checksum.h"
#include "serialise_jass_ci.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_JASS_CI::OPERATOR()()
		-------------------------------
	*/
	void serialise_jass_ci::operator()(const slice &term, const index_postings &postings)
		{
		/*
			Write the vocabulary term to CIvocab_terms.bin
		*/
		vocabulary_strings.write(term.address(), term.size());
		vocabulary_strings.write("\0", 1);
		}
	/*
		SERIALISE_JASS_CI::DELEGATE::OPERATOR()()
		-----------------------------------------
	*/
	void serialise_jass_ci::operator()(size_t document_id, const slice &primary_key)
		{
		/*
			To do.
		*/
		}


	/*
		SERIALISE_JASS_CI::UNITTEST()
		-----------------------------
	*/
	void serialise_jass_ci::unittest(void)
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
		serialise_jass_ci serialiser;
		index.iterate(serialiser);
		}

		/*
			Checksum the inde to make sure its correct.
		*/
		auto checksum = checksum::fletcher_16_file("CIvocab_terms.bin");
		JASS_assert(checksum == 0x61E1);

		puts("\t\tserialise_jass_ci::INCOMPLETE - not all implemented");
		}
	}
