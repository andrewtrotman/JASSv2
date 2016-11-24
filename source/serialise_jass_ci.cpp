/*
	SERIALISE_JASS_CI.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
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
			Serialise it.
		*/
		index.iterate(serialise_jass_ci());

		/*
			Checksum it.
		*/
		}
	}
