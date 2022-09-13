/*
	SERIALISE_FORWARD_INDEX.H
	-------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
 */
/*!
	@file
	@brief Serialise an index into a forward index in XML
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
 */
#pragma once

#include "file.h"
#include "index_manager.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_FORWARD_INDEX
		-----------------------------
	*/
	/*!
		@brief Serialse an index into a forward index in XML.
	*/
	class serialise_forward_index : public index_manager::delegate
		{
		private:
			std::vector<std::ostringstream> document;		///< Each document is represented as a string.

		public:
			serialise_forward_index() = delete;
			/*
				SERIALISE_FORWARD_INDEX::SERIALISE_FORWARD_INDEX()
				--------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] The numner of socuments in the collection.
			*/
			serialise_forward_index(size_t documents);

			/*
				SERIALISE_FORWARD_INDEX::~SERIALISE_FORWARD_INDEX()
				---------------------------------------------------
			*/
			/*!
				Destructor
			*/
			~serialise_forward_index()
				{
				/* Nothing */
				}

			/*
				SERIALISE_FORWARD_INDEX::FINISH()
				---------------------------------
			*/
			/*!
				Do any final cleaning up
			*/
			void finish(void);

			/*
				SERIALISE_FORWARD_INDEX::DELEGATE::OPERATOR()()
				-----------------------------------------------
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
				SERIALISE_FORWARD_INDEX::DELEGATE::OPERATOR()()
				-----------------------------------------------
			*/
			/*!
				@brief The callback function to serialise the primary keys (external document ids) is operator().
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key);

			/*
				SERIALISE_FORWARD_INDEX::UNITTEST()
				-----------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
