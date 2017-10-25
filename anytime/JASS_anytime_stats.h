/*
	JASS_ANYTIME_STATS.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Looad and deserialise a JASS v1 index
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

/*
	CLASS ANYTIME_STATS
	-------------------
*/
/*!
	@brief Runtime statistics for the anytime search engine.
*/
class anytime_stats
	{
	public:
		size_t number_of_queries;					///< The number of queries that have been processed

	public:
		/*
			ANYTIME_STATS::ANYTIME_STATS()
			------------------------------
		*/
		/*!
			@brief Constructor
		*/
		anytime_stats() :
			number_of_queries(0)
			{
			/* Nothing */
			}
	};
