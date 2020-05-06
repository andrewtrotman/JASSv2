/*
	POSTING.H
	---------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Tuple for a posting in a traditions <d,tf> postings list
*/

#pragma once

#include"compress_integer.h"

namespace JASS
	{
	/*
		CLASS POSTING
		-------------
	*/
	/*!
		@brief A <docid, tf> tuple.
	*/
	class posting
		{
		public:
			compress_integer::integer docid;					///< The Document identifier.
			compress_integer::integer term_frequency;		///< The number of times the term occurs in document with id docid.
		public:
			posting() :
				docid(0),
				term_frequency(0)
				{
				/*
					Nothing
				*/
				}
		} ;
	}
