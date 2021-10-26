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
#include "string_cpp.h"
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
		get_highest_document_id().  When subclassing, remember to call this class's methods from the over-ridden methods in the sub-class.
	*/
	class index_manager
		{
		private:
			 compress_integer::integer highest_document_id;								///< The highest document_id seen so far (counts from 1).
			 std::vector<compress_integer::integer> document_length_vector;		///< vector of document lengths.

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
					size_t documents;			///< The number of documents in the collection

				public:
					/*
						INDEX_MANAGER::DELEGATE::DELEGATE()
						------------------------------------
					*/
					/*!
						@brief Destructor.
					*/
					delegate(size_t documents) :
						documents(documents)
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER::DELEGATE::~DELEGATE()
						------------------------------------
					*/
					/*!
						@brief Destructor.
					*/
					virtual ~delegate()
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER::DELEGATE::OPERATOR()()
						-------------------------------------
					*/
					/*!
						@brief The callback function to serialise the postings (given the term) is operator().
						@param term [in] The term name.
						@param postings [in] The postings lists.
						@param document_frequency [in] The document frequency of the term
						@param document_ids [in] An array (of length document_frequency) of document ids.
						@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
					*/
					virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies) = 0;

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

					/*
						INDEX_MANAGER::DELEGATE::FINISH()()
						-----------------------------------
					*/
					/*!
						@brief Finish up any serialising that needs to be done.
					*/
					virtual void finish(void) = 0;
				};

			/*
				CLASS INDEX_MANAGER::QUANTIZING_DELEGATE
				----------------------------------------
			*/
			/*!
				@brief Base class for the callback function called by iterate.
			*/
			class quantizing_delegate
				{
				public:
					/*
						INDEX_MANAGER::DELEGATE::~QUANTIZING_DELEGATE()
						-----------------------------------------------
					*/
					/*!
						@brief Destructor.
					*/
					virtual ~quantizing_delegate()
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER::QUANTIZING_DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief The callback function to serialise the postings (given the term) is operator().
						@brief callback [in] The callback to export once quantised.
						@param term [in] The term name.
						@param postings [in] The postings lists.
						@param document_frequency [in] The document frequency of the term
						@param document_ids [in] An array (of length document_frequency) of document ids.
						@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
					*/
					virtual void operator()(delegate &callback, const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies) = 0;

					/*
						INDEX_MANAGER::QUANTIZING_DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief The callback function to serialise the primary keys (external document ids) is operator().
						@brief callback [in] The callback to export once quantised.
						@param document_id [in] The internal document identfier.
						@param primary_key [in] This document's primary key (external document identifier).
					*/
					virtual void operator()(delegate &callback, size_t document_id, const slice &primary_key) = 0;
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
				highest_document_id(0)				// initialised to 0, this is the number of documents that have (or is) being indexed.
				{
				document_length_vector.reserve(1'000'000);
				document_length_vector.push_back(0);
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
				INDEX_MANAGER::SET_PRIMARY_KEYS()
				---------------------------------
			*/
			/*!
				@brief Add a list of primary keys to the current list.  Normally used to set it without actually indexing (warning)
				@details Normally this method would only be called when an index is being "pushed" into an object
				rather than indexing document at a time.  This method actually adds to the end of the primary key list which
				is assumed to be empty before the method is called, but might not be if some indexing has already happened.
				@param keys [in] The vector of primary keys.
			*/
			virtual void set_primary_keys(const std::vector<slice> &keys)
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
				@param primary_key [in] The primary key (i.e. external dociment identifier) of this document.
			*/
			virtual void begin_document(const slice &primary_key)
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
				/* Nothing */
				}

			/*
				INDEX_MANAGER::TERM()
				---------------------
			*/
			/*!
				@brief Hand a new term with a pre-computed postings list to this object.
				@param term [in] The term from the token stream.
				@param postings_list [in] The pre-computed D1-encoded postings list
			*/
			virtual void term(const parser::token &term, const std::vector<posting> &postings_list)
				{
				/* Nothing */
				}

			/*
				INDEX_MANAGER::END_DOCUMENT()
				-----------------------------
			*/
			/*!
				@brief Tell this object that you've finished with the current document (and are about to move on to the next, or are completely finished).
			*/
			virtual void end_document(compress_integer::integer document_length)
				{
				document_length_vector.push_back(document_length);
				}

			/*
				INDEX_MANAGER::GET_DOCUMENT_LENGTH_VECTOR()
				-------------------------------------------
			*/
			/*!
				@brief Return a reference to the document length vector.
				@return The document length vector. This is only valid for as long as the index_manager object exists.
			*/
			virtual std::vector<compress_integer::integer> &get_document_length_vector(void)
				{
				return document_length_vector;
				}

			/*
				INDEX_MANAGER::SET_DOCUMENT_LENGTH_VECTOR()
				-------------------------------------------
			*/
			/*!
				@brief Replace the document length vector with the one passed to this function (warning).
				@param new_lengths [in] The new document length vectror
				@details It is possble that new_length.size() is different to the current largest document number.  If this is the
				case then the largest document number is set to the number of documents in new_lengths, and future calls to index
				a single document will fail (the alternative is that documents in the middle get lengths of 0).
			*/
			virtual void set_document_length_vector(std::vector<compress_integer::integer> &new_lengths)
				{
				document_length_vector = new_lengths;
				highest_document_id = document_length_vector.size() - 1;		// if size() == 10 then the highest_document_id is 9 (documents have ids 0..9)
				}

			/*
				INDEX_MANAGER::TEXT_RENDER()
				----------------------------
			*/
			/*!
				@brief unimplemented: Dump a human-readable version of the index down the stream.
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
			virtual void iterate(delegate &callback)
				{
				/*
					This method exists simply to increase code coverage. It should never get called in an "ordinary" program.
				*/
				allocator_pool pool;
				index_postings postings(pool);
				callback(slice(), postings, 0, nullptr, nullptr);
				callback(0, slice());
				}
			/*
				INDEX_MANAGER::ITERATE()
				------------------------
			*/
			/*!
				@brief Iterate over the index calling callback.operator() with each postings list.
				@param quantizer [in] The quantizer that will quantize then call the serialiser callback.
				@param callback [in] The callback that the quantizer should call.
			*/
			virtual void iterate(index_manager::quantizing_delegate &quantizer, index_manager::delegate &callback)
				{
				/* Nothing */
				}

			/*
				INDEX_MANAGER::GET_HIGHEST_DOCUMENT_ID()
				----------------------------------------
			*/
			/*!
				@brief Return the number of documents that have been successfully indexed or are in the process of being indexed.
			*/
			 compress_integer::integer get_highest_document_id(void) const
				{
				return highest_document_id;
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
			
				JASS_assert(index.get_highest_document_id() == 0);

				/*
					Add a token
				*/
				parser::token token;
				index.begin_document(slice("id"));
				index.term(token);
				index.end_document(1);

				/*
					Make sure its no longer empty
				*/
				JASS_assert(index.get_highest_document_id() == 1);
				
				/*
					Call the methods that do nothing for code-coverage purposes
				*/
				std::ostringstream into;
				index.text_render(into);
				JASS_assert(into.str().size() == 0);

				struct delegate_test : public delegate
					{
					delegate_test(size_t documents_in_collection) : delegate(documents_in_collection)
						{
						/* Nothing */
						}
					virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
						{
						/* Nothing */
						}
					virtual void operator()(size_t document_id, const slice &primary_key)
						{
						/* Nothing */
						}
					virtual void finish(void)
						{
						/* Nothing */
						}
					};
				delegate_test callback(index.get_highest_document_id());
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
