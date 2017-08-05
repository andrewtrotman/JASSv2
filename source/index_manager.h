/*
	INDEX_MANAGER.H
	---------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Base class for the indexer object that stored the actual index during indexing.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "parser.h"
#include "strings.h"
#include "index_postings.h"

namespace JASS
	{
	/*
		CLASS INDEX_MANAGER
		-------------------
	*/
	/*!
		@brief Base class for holding the index during indexing.
		@details This class is a base class used to define the interface for different approaches to indexing.  Once an object of this type has been
		declared it is used by calling begin_document() at the beginning of each document, end_document() at the end of each document, and term()
		for each term in the token stream (i.e. "the cat and the dog" is 5 tokens, "the", "cat", "and", "the", "dog".  This class does not stem and
		it does not stop words.  That behaviour is exterior to this class.  To find out how many documents have been indexed up-to a given point call
		get_highest_document_id().  To find out how many tokens have been indexed call get_highest_term_id().  When subclassing, remember to call this
		class's methods from the over-ridden methods in the sub-class.
	*/
	class index_manager
		{
		private:
			size_t highest_document_id;					///< The highest document_id seen so far (counts from 1).
			size_t highest_term_id;							///< The highest term_id seen so far (counts from 1).

		public:
			/*
				CLASS INDEX_MANAGER::DELEGATE
				-----------------------------
			*/
			/*!
				@brief Base class for the callback function called by iterate.
			*/
			class delegate
				{
				public:
					/*
						INDEX_MANAGER::DELEGATE::OPERATOR()()
						-------------------------------------
					*/
					/*!
						@brief Destructor
					*/
					virtual ~delegate()
						{
						/*
							Nothing
						*/
						}
					/*
						INDEX_MANAGER::DELEGATE::OPERATOR()()
						-------------------------------------
					*/
					/*!
						@brief The callback function to serialise the postings (given the term) is operator().
						@param term [in] The term name.
						@param postings [in] The postings lists.
					*/
					virtual void operator()(const slice &term, const index_postings &postings) = 0;

					/*
						INDEX_MANAGER::DELEGATE::OPERATOR()()
						-------------------------------------
					*/
					/*!
						@brief The callback function to serialise the primary keys (external document ids) is operator().
						@param document_id [in] The internal document identfier.
						@param primary_key [in] This document's primary key (external document identifier).
					*/
					virtual void operator()(size_t document_id, const slice &primary_key) = 0;
				};

		public:
			/*
				INDEX_MANAGER::INDEX_MANAGER()
				------------------------------
			*/
			/*!
				@brief Constructor
			*/
			index_manager() :
				highest_document_id(0),				// initialised to 0, this is the number of documents that have (or is) being indexed.
				highest_term_id(0)					// initialised to 0, this is the number of terms that have been seen.
				{
				/*
					Nothing
				*/
				}
			
			/*
				INDEX_MANAGER::~INDEX_MANAGER()
				-------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~index_manager()
				{
				/*
					Nothing
				*/
				}
			
			/*
				INDEX_MANAGER::BEGIN_DOCUMENT()
				-------------------------------
			*/
			/*!
				@brief Tell this object that you're about to start indexing a new object.
				@param external_id [in] The primary key (i.e. external dociment identifier) of this document.
			*/
			virtual void begin_document(const slice &external_id)
				{
				highest_document_id++;
				}
			
			/*
				INDEX_MANAGER::TERM()
				---------------------
			*/
			/*!
				@brief Hand a new term from the token stream to this object.
				@param term [in] The term from the token stream.
			*/
			virtual void term(const parser::token &term)
				{
				highest_term_id++;
				}
			
			/*
				INDEX_MANAGER::END_DOCUMENT()
				-----------------------------
			*/
			/*!
				@brief Tell this object that you've finished with the current document (and are about to move on to the next, or are completely finished).
			*/
			virtual void end_document(void)
				{
				/* Nothing */
				}
			
			/*
				INDEX_MANAGER::TEXT_RENDER()
				----------------------------
			*/
			/*!
				@brief Dump a human-readable version of the index down the stream.
				@param stream [in] The stream to write to.
			*/
			virtual void text_render(std::ostream &stream) const
				{
				/* Nothing */
				}

			/*
				INDEX_MANAGER::ITERATE()
				------------------------
			*/
			/*!
				@brief Iterate over the index calling callback.operator() with each postings list.
				@param callback [in] The callback to call.
			*/
			virtual void iterate(delegate &callback) const
				{
				/*
					This method exists simply to increase code coverage. It should never get called in an "ordinary" program.
				*/
				allocator_pool pool;
				index_postings postings(pool);
				callback(slice(), postings);
				callback(0, slice());
				}

			/*
				INDEX_MANAGER::GET_HIGHEST_DOCUMENT_ID()
				----------------------------------------
			*/
			/*!
				@brief Return the number of documents that have been successfully indexed or are in the process of being indexed.
			*/
			size_t get_highest_document_id(void) const
				{
				return highest_document_id;
				}
			
			/*
				INDEX_MANAGER::GET_HIGHEST_TERM_ID()
				------------------------------------
			*/
			/*!
				@brief Return the number of tokens that have been successfully indexed.
			*/
			size_t get_highest_term_id(void) const
				{
				return highest_term_id;
				}

			/*
				INDEX_MANAGER::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void)
				{
				/*
					This just creates an empty index and makes sure the index is empty
				*/
				index_manager index;
			
				JASS_assert(index.get_highest_term_id() == 0);
				JASS_assert(index.get_highest_document_id() == 0);

				/*
					Add a token
				*/
				parser::token token;
				index.begin_document(slice("id"));
				index.term(token);
				index.end_document();

				/*
					Make sure its no longer empty
				*/
				JASS_assert(index.get_highest_term_id() == 1);
				JASS_assert(index.get_highest_document_id() == 1);
				
				/*
					Call the methods that do nothing for code-coverage purposes
				*/
				std::ostringstream into;
				index.text_render(into);
				JASS_assert(into.str().size() == 0);

				struct delegate_test : public delegate
					{
					virtual void operator()(const slice &term, const index_postings &postings)
						{
						/* Nothing */
						}
					virtual void operator()(size_t document_id, const slice &primary_key)
						{
						/* Nothing */
						}
					};
				delegate_test callback;
				index.iterate(callback);

				/*
					Done.
				*/
				puts("index_manager::PASSED");
				}
		};
		
	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump a human readable version of the index down an output stream.
		@param stream [in] The stream to write to.
		@param data [in] The index to write.
		@return The stream once the index has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const index_manager &data)
		{
		data.text_render(stream);
		return stream;
		}

	}
