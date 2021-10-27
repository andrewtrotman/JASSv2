/*
	SERIALISE_JASS_V2.H
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Serialise an index in the format used by JASS version 2.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "serialise_jass_v1.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_JASS_V2
		-----------------------
	*/
	/*!
		@brief Serialise an index in the format used by JASS version 2 (a better compressed JASS v1 format).
		@details See description for serialise_jass_v1.
	*/
	class serialise_jass_v2 : public serialise_jass_v1
		{
		protected:
			std::vector<slice, allocator_cpp<slice>> compressed_headers;

		protected:
			/*
				SERIALISE_JASS_V2::WRITE_POSTINGS()
				-----------------------------------
			*/
			/*!
				@brief Convert the postings list to the JASS v2 format and serialise it to disk.
				@param postings [in] The postings list to serialise.
				@param number_of_impacts [out] The number of distinct impact scores seen in the postings list.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
				@return The location (in CIpostings.bin) of the start of the serialised postings list.
			*/
			virtual size_t write_postings(const index_postings &postings, size_t &number_of_impacts, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies);

		public:
			/*
				SERIALISE_JASS_V2::SERIALISE_JASS_V2()
				--------------------------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] The number of documents in the collection (used to allocate re-usable buffers).
				@param encoder [in] An shared pointer to a codex responsible for performing the compression of postings lists (default = compress_integer_QMX_jass_v1()).
				@param alignment [in] The start address of a postings list is padded to start on these boundaries (needed for compress_integer_QMX_jass_v1 (use 16), and others).  Default = 0.
			*/
			serialise_jass_v2(size_t documents, jass_v1_codex codex = jass_v1_codex::elias_gamma_simd_vb, int8_t alignment = 1) :
				serialise_jass_v1(documents, codex, alignment),
				compressed_headers(allocator)
				{
				/* Nothing */
				}

			/*
				SERIALISE_JASS_V2::~SERIALISE_JASS_V2()
				---------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~serialise_jass_v2()
				{
				/* Nothing */
				}

			/*
				 SERIALISE_JASS_V2::SERIALISE_VOCABULARY_POINTERS()
				--------------------------------------------------
			*/
			/*!
				@brief Serialise the ppointers that point between the vocab and the postings (the CIvocab.bin file).
			*/
			virtual void serialise_vocabulary_pointers(void);

			/*
				 SERIALISE_JASS_V1::SERIALISE_PRIMARY_KEYS()
				--------------------------------------------
			*/
			/*!
				@brief Serialise the primary keys (or any extra stuff at the end of the primary key file).
			*/
			virtual void serialise_primary_keys(void);

			/*
				SERIALISE_JASS_V2::DELEGATE::OPERATOR()()
				-----------------------------------------
			*/
			/*!
				@brief The callback function to serialise the postings (given the term) is operator().
				@param term [in] The term name.
				@param postings [in] The postings lists.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
			*/
			virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies);

			/*
				SERIALISE_JASS_V2::UNITTEST()
				-----------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
