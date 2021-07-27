/*
	JASS_ANYTIME_SEGMENT.H
	----------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A postings list segment as seen by JASS_anytime.
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "query.h"

class JASS_anytime_segment_header
	{
	public:
		JASS::query::ACCUMULATOR_TYPE impact;			///< The impact score
		uint64_t offset;										///< Offset (within the postings file) of the start of the compressed postings list
		uint64_t end;											///< Offset (within the postings file) of the end of the compressed postings list
		JASS::query::DOCID_TYPE segment_frequency;	///< The number of document ids in the segment (not end - offset because the postings are compressed)
	};
