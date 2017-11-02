/*
	JASS_ANYTIME_STATS.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Load and deserialise a JASS v1 index
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
		size_t threads;								///< The number of threads (mean queries per thread = number_of_queries/threads)
		size_t number_of_queries;					///< The number of queries that have been processed
		size_t total_search_time_in_ns;			///< Total time to search (in nanoseconds)

	public:
		/*
			ANYTIME_STATS::ANYTIME_STATS()
			------------------------------
		*/
		/*!
			@brief Constructor
		*/
		anytime_stats() :
			threads(0),
			number_of_queries(0),
			total_search_time_in_ns(0)
			{
			/* Nothing */
			}
	};

/*
	OPERATOR<<()
	------------
*/
/*!
	@brief Dump a human readable version of the data down an output stream.
	@param stream [in] The stream to write to.
	@param data [in] The data to write.
	@return The stream once the data has been written.
*/
std::ostream &operator<<(std::ostream &output, anytime_stats &data)
	{
	output << "-------------------\n";
	output << "Threads                                : " << data.threads << '\n';
	output << "Queries                                : " << data.number_of_queries << '\n';
	output << "Total search time                      : " << data.total_search_time_in_ns << " ns\n";
	output << "Total time excluding I/O   (per query) : " << data.total_search_time_in_ns / ((data.number_of_queries == 0) ? 1 : data.number_of_queries) << " ns\n";
	output << "-------------------\n";
	return output;
	}
