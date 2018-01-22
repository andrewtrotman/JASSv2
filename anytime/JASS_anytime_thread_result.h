/*
	JASS_ANYTIME_THREAD_RESULT.H
	----------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The results of a single thread in the Anytime engine.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

/*
	CLASS JASS_ANYTIME_THREAD_RESULT
	--------------------------------
*/
/*!
	@brief Results from a single thread of execution in parallel search.
*/
class JASS_anytime_thread_result
	{
	public:
		std::ostringstream results_list;				///< The results lists from this thread
		size_t queries_executed;						///< The number of queries that this thread executed
		size_t search_time_in_ns;						///< The total time this thread spent searching
		
	public:
		/*
			JASS_anytime_thread_result::JASS_anytime_thread_result()
			--------------------------------------------------------
		*/
		JASS_anytime_thread_result() :
			queries_executed(0),
			search_time_in_ns(0)
			{
			/* Nothing */
			}
	};
