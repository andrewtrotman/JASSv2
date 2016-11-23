/*
	SERIALISE_JASS_CI.H
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Serialise an index in the experimental JASS-CI format used in the RIGOR workshop.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "file.h"
#include "slice.h"
#include "index_postings.h"
#include "index_manager.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_JASS_CI
		-----------------------
	*/
	/*!
		@brief Serialise an index in the experimental JASS-CI format used in the RIGOR workshop.
		@details The original version of JASS was an experimental hack in reducing the complexity of the ATIRE search engine,
		that resulted in an index that was large, but easy to process.  The intent was to go back and "fix" the index to be smaller
		and faster.  That never happened.  Instead it was used as the basis of other work.  In an effort to bring up this re-write of
		ATIRE and JASS, compatibility with the hack (known as JASS CI) is maintained so that the indexer can be checked without writing
		the search engine itself (i.e. this JASS is being bootstrapped from JASS CI).

		The paper compairing JASS CI to other search engines (including ATIRE) is here:
		J. Lin, M. Crane, A. Trotman, J. Callan, I. Chattopadhyaya, J. Foley, G. Ingersoll, C. Macdonald, S. Vigna (2016),
		Toward Reproducible Baselines: The Open-Source IR Reproducibility Challenge, Proceedings of the European Conference on 
		Information Retrieval (ECIR 2016), pp. 408-420.

		The JASS CI index in made up of 4 files:  CIvocab_terms.bin, CIvocab.bin, CIpostings.bin, and CIdoclist.bin

		CIvocab_terms.bin: This is a list of all the unique terms in the collection (the closure of the vocabulary).  It is stored as a
		sequence of '\0' terminated UTF-8 strings.  So, if the vocabularty contains three terms, "a", "bb" and "cc",
		then the contents of CIvocab_terms.bin will be "a\0bb\0cc\0".
	*/
	class serialise_jass_ci : public index_manager::delegate
		{
		private:
			file vocabulary_strings;					///< The concatination of UTS-8 encoded unique tokens in the collection.
			file vocabulary;							///< Details about the term (including a pointer to the term, a pointer to the postings, and the quantum count.
			file postings;								///< The postings lists.

		public:
			/*
				SERIALISE_JASS_CI::SERIALISE_JASS_CI()
				--------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			serialise_jass_ci() :
				vocabulary("CIvocab.bin"),
				vocabulary_strings("CIvocab_terms.bin"),
				postings("CIpostings.bin")
				{
				/*
					Nothing
				*/
				}

			/*
				SERIALISE_JASS_CI::~SERIALISE_JASS_CI()
				--------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~serialise_jass_ci()
				{
				/*
					Nothing
				*/
				}
			/*
				SERIALISE_JASS_CI::~OPERATOR()()
				--------------------------------
			*/
			/*!
				@brief Callback called when serialising an index using an index_manager object.
			*/
			virtual void operator()(const slice &term, const index_postings &postings);

			/*
				SERIALISE_JASS_CI::~UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}