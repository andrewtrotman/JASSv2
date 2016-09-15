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
#include "allocator_cpp.h"

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
		public:
			allocator allocator;
			slice primary_key;	///< The external primary key (e.g. TREC DOCID, or filename) of the document (or empty if that is meaningless)
			slice contents;		///< The contents of the document (or likewise)
			
		public:
			document() :
				allocator(1024),
				name(),
				contents()
				{
				}
		};
	}