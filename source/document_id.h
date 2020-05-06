/*
	DOCUMENT_ID.H
	-------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A type defining a document id (was compress_integer::integer)
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once
namespace JASS
	{
	class document_id
		{
		public:
			typedef uint32_t integer;									///< This class and descendants will work on integers of this size.  Do not change without also changing JASS_COMPRESS_INTEGER_BITS_PER_INTEGER
			#define JASS_COMPRESS_INTEGER_BITS_PER_INTEGER 32	///< The number of bits in compress_integer::integer (either 32 or 64). This must remain in sync with compress_integer::integer (and a hard coded value to be used in \#if statements)
		};
	}
