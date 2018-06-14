/*
	SERIALISE_INTEGERS.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
 */
/*!
	@file
	@brief Serialise an index in integer dump format
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
 */
#pragma once

#include "file.h"
#include "index_manager.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_INTEGERS
		------------------------
	*/
	/*!
		@brief Serialise an index in integer dump format
		@details Serialise just the postings lists (without the vocabulary) as a bunch of 4-byte integers (in Intel byte order).  The exact format is:
		\<length\> The number of integers in the list
		\<docid\> The document identifier
		The final index file is, therefore of the form:
		\<length\>\<docid\>...\<docid\>
		\<length\>\<docid\>...\<docid\>
		\<length\>\<docid\>...\<docid\>
		For as many impact segments are there are in the collection.
	*/
	class serialise_integers : public index_manager::delegate
		{
		private:
			file postings_file;									///< The postings file
			allocator_pool memory;								///< Memory used to store the impact-ordered postings list.
			index_postings_impact impact_ordered; 			/// << the impact-ordered postings list
			size_t documents_in_collection;				///< The number of documents in the collection.
			/*
				Each of these buffers is re-used in the serialisation process
			*/
			compress_integer::integer *document_ids;					///< The re-used buffer storing decoded document ids
			index_postings_impact::impact_type *term_frequencies;	///< The re-used buffer storing the term frequencies
			size_t temporary_size;											///< The number of bytes in temporary
			uint8_t *temporary;												///< Temporary buffer - cannot be used to store anything between calls


		public:
			/*
				SERIALISE_INTEGERS::SERIALISE_INTEGERS()
				----------------------------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] the number of documents in the collection
			*/
			serialise_integers(size_t documents) :
				postings_file("postings.bin", "w+b"),
				impact_ordered(documents, memory),
				document_ids((decltype(document_ids))memory.malloc(documents * sizeof(*document_ids))),
				term_frequencies((decltype(term_frequencies))memory.malloc(documents * sizeof(*term_frequencies))),
				temporary_size(documents * (sizeof(*document_ids) / 7 + 1) * sizeof(*temporary)),
				temporary((decltype(temporary))memory.malloc(temporary_size))			// enough space to decompress variable-byte encodings

				{
				/* Nothing. */
				}

			/*
				SERIALISE_INTEGERS::OPERATOR()()
				--------------------------------
			*/
			/*!
				@brief The callback function to serialise the postings (given the term) is operator().
				@param term [in] The term name.
				@param postings [in] The postings lists.
			*/
			virtual void operator()(const slice &term, const index_postings &postings);

			/*
				SERIALISE_INTEGERS::OPERATOR()()
				--------------------------------
			*/
			/*!
				@brief The callback function to serialise the primary keys (external document ids) is operator().
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key)
				{
				/* Nothing. */
				}

			/*
				SERIALISE_INTEGERS::UNITTEST_ONE_COLLECTION()
				---------------------------------------------
			*/
			/*!
				@brief Validate over a single collection - JASS_assert on failure.
				@param document_collection [in] the collection to validate over
				@param expected_checksum [in] The expected checksum
			*/
			static void unittest_one_collection(const std::string &document_collection, uint32_t expected_checksum);

			/*
				SERIALISE_INTEGERS::UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
