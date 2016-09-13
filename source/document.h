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
			allocator c_allocator;
			allocator_cpp<uint8_t> allocator;
			std::vector<uint8_t, allocator_cpp<uint8_t>> name;			///< The name of the document (or empty if that is meaningless)
			std::vector<uint8_t, allocator_cpp<uint8_t>> contents;	///< The contents of the document (or likewise)
			
		public:
			document() :
				c_allocator(1024),
				allocator(c_allocator),
				name(allocator),
				contents(allocator)
				{
				}
		};
	}