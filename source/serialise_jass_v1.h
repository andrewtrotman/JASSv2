/*
	SERIALISE_JASS_V1.H
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Serialise an index in the experimental JASS-CI format used (by JASS version 1) in the RIGOR workshop.
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
		CLASS SERIALISE_JASS_V1
		-----------------------
	*/
	/*!
		@brief Serialise an index in the experimental JASS-CI format used (by JASS version 1) in the RIGOR workshop.
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
		then the contents of CIvocab_terms.bin will be "a\0bb\0cc\0".  This file does not need to be sorted in alphabetical (or similar) order.
		
		CIvocab.bin: This is a list of triples <term, offset, impacts>. Term is a pointer to the string in the CIvocab_terms.bin file (i.e. a
		byte offset within the file). Offset is the offset (in CIpostings.bin) of the start of the postings list. Impacts is the number of impacts
		in the impact ordered postings list.  JASS v1 assumes this file is sorted in alphabetical order by the term string (i.e. where term points to)
		when using strcmp().
	*/
	class serialise_jass_v1 : public index_manager::delegate
		{
		private:
			/*
				CLASS SERIALISE_JASS_V1::VOCAB_TRIPPLE
				--------------------------------------
			*/
			/*!
				@brief The tripple used in CIvocab.bin
			*/
			class vocab_tripple
				{
				public:
					std::string	token;			///< The term as a string (needed for sorting the std::vectorvocab_tripple array later)
					uint64_t term;				///< The pointer to the \0 terminated string in the CI_vovab_terms.bin file.
					uint64_t offset;			///< The pointer to the postings stored in the CIpostings.bin file.
					uint64_t impacts;			///< The number of impacts that exist for this term.
				public:
					/*
						 SERIALISE_JASS_V1::VOCAB_TRIPPLE::VOCAB_TRIPPLE()
						--------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param string [in] The term that that this object represents.
						@param term [in] The location of this term in CIvocab_terms.bin.
						@param offset [in] The offset of the postings list in CIpostings.bin.
						@param impacts [in] The number of impacts in the postings list.
					*/
					vocab_tripple(const slice &string, uint64_t term, uint64_t offset, uint64_t impacts) :
						token(static_cast<char *>(string.address()), string.size()),
						term(term),
						offset(offset),
						impacts(impacts)
						{
						/* Nothing */
						}
					
					bool operator<(const vocab_tripple &other) const
						{
						return token < other.token;
						}
				};
			
		private:
			file vocabulary_strings;					///< The concatination of UTS-8 encoded unique tokens in the collection.
			file vocabulary;							///< Details about the term (including a pointer to the term, a pointer to the postings, and the quantum count.
			file postings;								///< The postings lists.
			std::vector<vocab_tripple> index_key;		///< The entry point into the JASS v1 index is CIvocab.bin, the index key.

		public:
			/*
				SERIALISE_JASS_V1::SERIALISE_JASS_V1()
				--------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			serialise_jass_v1() :
				vocabulary_strings("CIvocab_terms.bin", "w+b"),
				vocabulary("CIvocab.bin", "w+b"),
				postings("CIpostings.bin", "w+b")
				{
				/* Nothing */
				}

			/*
				SERIALISE_JASS_V1::~SERIALISE_JASS_V1()
				--------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~serialise_jass_v1();

			/*
				SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
				-----------------------------------------
			*/
			/*!
				@brief The callback function to serialise the postings (given the term) is operator().
				@param term [in] The term name.
				@param postings [in] The postings lists.
			*/
			virtual void operator()(const slice &term, const index_postings &postings);

			/*
				SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
				-----------------------------------------
			*/
			/*!
				@brief The callback function to serialise the primary keys (external document ids) is operator().
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key);

			/*
				SERIALISE_JASS_V1::~UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
