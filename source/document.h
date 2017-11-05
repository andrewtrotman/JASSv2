/*
	DOCUMENT.H
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A document withing the indexing pipeline
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "slice.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*
		CLASS DOCUMENT
		--------------
	*/
	/*!
		@brief Container class representing a document through the indexing pipeline.
		
		@details An example tying documents, instreams, and parsing to count the number of document and non-unique symbols is:
		
		@include parser_use.cpp
	*/
	class document
		{
		private:
			static const size_t default_allocation_size = 8192;		///< The default size of the allocation unit within the document.
			
		protected:
			allocator_pool default_allocator;		///< If a document is created without a specified allocator then use a pool allocator

		public:
			allocator &primary_key_allocator;		///< If memory is needed for the primary key then allocate from here.
			allocator &contents_allocator;			///< If memory is needed for the document contents then allocate from here.
			slice primary_key;							///< The external primary key (e.g. TREC DOCID, or filename) of the document (or empty if that is meaningless).
			slice contents;								///< The contents of the document (or likewise).
			
		public:
			/*
				DOCUMENT::DOCUMENT()
				--------------------
			*/
			/*!
				@brief Constructor using an allocator local to this object (useful when the object needs to contain its own memory)
			*/
			document() :
				default_allocator(default_allocation_size),
				primary_key_allocator(default_allocator),
				contents_allocator(default_allocator)
				{
				/*
					Nothing
				*/
				}
	
			/*
				DOCUMENT::DOCUMENT()
				--------------------
			*/
			/*!
				@brief Constructor using an allocator specified (useful when the object needs to allocate memory in a specific location)
			*/
			document(class allocator &memory_source) :
				primary_key_allocator(default_allocator),
				contents_allocator(memory_source)
				{
				/*
					Nothing
				*/
				}
			
			/*
				DOCUMENT::ISEMPTY()
				-------------------
			*/
			/*!
				@brief Check to see whether this is an empty document.
				@return true if the document has no contents, else false;
			*/
			bool isempty(void)
				{
				return contents.size() == 0;
				}
			
			/*
				DOCUMENT::REWIND()
				------------------
			*/
			/*!
				@brief Free up all resources assocuated with this object and make it ready for re-use.
			*/
			void rewind(void)
				{
				primary_key = slice();
				contents = slice();
				contents_allocator.rewind();
				primary_key_allocator.rewind();
				}

			/*
				DOCUMENT::UNITTEST()
				--------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				document document1;
				allocator_pool pool;
				document document2(pool);
				
				puts("document::PASSED");
				}
		};
	}
