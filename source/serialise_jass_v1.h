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
		@details The original version of JASS was an experimental hack in reducing the complexity of the ATIRE search 
		engine, that resulted in an index that was large, but easy to process. The intent was to go back and "fix" the 
		index to be smaller and faster. That never happened. Instead it was used as the basis of other work. In an 
		effort to bring up this re-write of ATIRE and JASS, compatibility with the hack (known as JASS version 1) is 
		maintained so that the indexer can be checked without writing the search engine itself (i.e. this JASS is being
		 bootstrapped from JASS version 1)

		The paper comparing JASS version 1 to other search engines (including ATIRE) is here: J. Lin, M. Crane, A. Trotman,
		J. Callan, I. Chattopadhyaya, J. Foley, G. Ingersoll, C. Macdonald, S. Vigna (2016), Toward Reproducible Baselines:
		The Open-Source IR Reproducibility Challenge, Proceedings of the European Conference on Information Retrieval
		(ECIR 2016), pp. 408-420.

		The JASS version 1 index in made up of 4 files: CIvocab_terms.bin, CIvocab.bin, CIpostings.bin, and CIdoclist.bin

		CIdoclist.bin: The list of document identifiers (each '\0' terminated). Then an index to each of the doclents 
		(stored as a table of uint64_t). The final 8 bytes of the file is an uin64_t storing the total numbner of unique 
		documents in the collection.

		CIvocab_terms.bin: This is a list of all the unique terms in the collection (the closure of the vocabulary). It is 
		stored as a sequence of '\0' terminated UTF-8 strings. So, if the vocabularty contains three terms, "a", "bb" and "cc",
		then the contents of CIvocab_terms.bin will be "a\0bb\0cc\0". This file does not need to be sorted in alphabetical (or 
		similar) order.

		CIvocab.bin: This is a list of triples (term, offset, impacts). Term is a pointer to the string in the CIvocab_terms.bin 
		file (i.e. a byte offset within the file). Offset is the offset (in CIpostings.bin) of the start of the postings list. 
		Impacts is the number of impacts in the impact ordered postings list. JASS v1 assumes this file is sorted in alphabetical 
		order by the term string (i.e. where term points to) when using strcmp().

		CIpostings.bin: This file contains all the postings lists compressed using the same codex. This is different from 
		ATIRE which allows each postings list to be encoded using a different codex. The first byte of this file specifies 
		the codex where s=uncompressed, c=VarByte, 8=Simple8, q=QMX, Q=QMX4D, R=QMX0D. This is followed by the postings lists.
		A postings list is: a list of 64-bit pointer to headers. Each header is (uint16_t impact_score, uint64_t start,
		uint64_t end, uint32_t impact_frequency) where impact_score is the impact value, start and end are pointers to the
		compressed docids, and impact_frequency is the number of dociment_ids in the list. The header is terminated with a 
		row of all 0s (i.e. 22 consequitive 0-bytes). This is followed by the list of docid's for each segment - each compressed 
		seperately. These lists do not have the impact score stored at the start and do not have 0 terminators on them. This 
		means score-at-a-time processing is the only paradigm, even if term-at-a-time processing is done score-at-a-time for 
		each term. ATIRE could do either (but it was a compile time flag).
	*/
	class serialise_jass_v1 : public index_manager::delegate
		{
		private:
			/*
				ENUM CLASS JASS_V1_CODEX
				------------------------
			*/
			/*!
				@brief The compression scheme that is active
			*/
			enum class jass_v1_codex
				{
				uncompressed = 's',				///< Postings are not compressed.
				variable_byte = 'c',				///< Postings are compressed using ATIRE's variable byte encoding.
				simple_8 = '8',					///< Postings are compressed using ATIRE's simple-8 encoding.
				qmx = 'q',							///< Postings are compressed using QMX (with difference encoding).
				qmx_d4 = 'Q',						///< Postings are compressed using QMX with Lemire's D4 delta encoding.
				qmx_d0 = 'R'						///< Postings are compressed using QMX without delta encoding.
				};

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
					slice	token;			///< The term as a string (needed for sorting the std::vectorvocab_tripple array later)
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
						token(string),
						term(term),
						offset(offset),
						impacts(impacts)
						{
						/* Nothing */
						}
					
					/*
						 SERIALISE_JASS_V1::VOCAB_TRIPPLE::OPERATOR<()
						----------------------------------------------
					*/
					/*!
						@brief Compare (using strcmp() colaiting sequence) this object with another for less than.
						@param other [in] The object to compare to
						@return true if this < other, else false
					*/
					bool operator<(const vocab_tripple &other) const
						{
						return slice::strict_weak_order_less_than(token, other.token);
						}
				};
			
		private:
			file vocabulary_strings;						///< The concatination of UTS-8 encoded unique tokens in the collection.
			file vocabulary;									///< Details about the term (including a pointer to the term, a pointer to the postings, and the quantum count.
			file postings;										///< The postings lists.
			file primary_keys;								///< The list of external identifiers (document primary keys).
			std::vector<vocab_tripple> index_key;		///< The entry point into the JASS v1 index is CIvocab.bin, the index key.
			std::vector<uint64_t> primary_key_offsets;///< A list of locations (on disk) of each primary key.
			allocator_pool memory;							///< Memory used to store the impact-ordered postings list.

		private:
			/*
				SERIALISE_JASS_V1::WRITE_POSTINGS()
				-----------------------------------
			*/
			/*!
				@brief Convert the postings list to the JASS v1 format and serialise it to disk.
				@param postings [in] The postings list to serialise.
				@param number_of_impacts [out] The number of distinct impact scores seen in the postings list.
				@return The location (in CIpostings.bin) of the start of the serialised postings list.
			*/
			size_t write_postings(const index_postings &postings, size_t &number_of_impacts);

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
				postings("CIpostings.bin", "w+b"),
				primary_keys("CIdoclist.bin", "w+b"),
				memory(1024 * 1024)								///< The allocation block size is currently 1MB, big enough for most postings lists (but it'll grow for larger ones).
				{
				/*
					For the initial bring-up the postings ar not compressed.
				*/
				uint8_t codex = static_cast<uint8_t>(jass_v1_codex::uncompressed);
				postings.write(&codex, 1);
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
