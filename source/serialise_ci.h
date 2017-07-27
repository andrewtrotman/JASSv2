/*
	SERIALISE_CI.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
 */
/*!
	@file
	@brief Serialise an index in source code
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
 */
#pragma once

#include "file.h"
#include "index_manager.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_CI
		------------------
	*/
	/*!
		@brief Serialse an index into source files for use with JASS_compiled_index.cpp
		@details  Andrew Trotman (University of Otago) and Jimmy Lin (University of Waterloo) proposed serialising the index into
		source code and then the index and the search engine are all the same single file.  This is an implementaiton of this, indexing
		into a postings file (and header), a vocabulary file.
	*/
	class serialise_ci : public index_manager::delegate
		{
		private:
			file postings_file;						///< The postings file
			file postings_header_file;				///< The header file for the postings file (so that the vocab can point to the methods)
			file vocab_file;						///< The vocabulary file (also know as the dictionary file)
			file primary_key_file;					///< The list of primary keys.
			uint64_t terms;							///< The number of terms in the vocabulary file.

		public:
			/*
				SERIALISE_CI::SERIALISE_CI()
				----------------------------
			*/
			/*!
				Constructor
			*/
			serialise_ci();

			/*
				SERIALISE_CI::~SERIALISE_CI()
				-----------------------------
			*/
			/*!
				Destructor
			*/
			~serialise_ci();

			/*
				SERIALISE_CI::DELEGATE::OPERATOR()()
				------------------------------------
			*/
			/*!
				@brief The callback function to serialise the postings (given the term) is operator().
				@param term [in] The term name.
				@param postings [in] The postings lists.
			*/
			virtual void operator()(const slice &term, const index_postings &postings);

			/*
				SERIALISE_CI::DELEGATE::OPERATOR()()
				------------------------------------
			*/
			/*!
				@brief The callback function to serialise the primary keys (external document ids) is operator().
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key);

			/*
				SERIALISE_CI::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
