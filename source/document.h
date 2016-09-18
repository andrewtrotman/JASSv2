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
		@brief Container class representing a document through the indexing pipeline
	*/
	class document
		{
		protected:
			allocator_pool default_allocator;		///< If a document is created without a specified allocator then use a pool allocator

		public:
			allocator &allocator;	///< If memory is needed then allocate from here.
			slice primary_key;		///< The external primary key (e.g. TREC DOCID, or filename) of the document (or empty if that is meaningless).
			slice contents;			///< The contents of the document (or likewise).
			
		public:
			/*
				DOCUMENT::DOCUMENT()
				--------------------
			*/
			/*!
				@brief Constructor using an allocator local to this object (useful when the object needs to contain its own memory)
			*/
			document() :
				default_allocator(1024),
				allocator(default_allocator)
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
				allocator(memory_source)
				{
				/*
					Nothing
				*/
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