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
	CLASS JASS_ANYTIME_STATS
	------------------------
*/
/*!
	@brief Runtime statistics for the anytime search engine.
*/
class JASS_anytime_stats
	{
	public:
		size_t threads;								///< The number of threads (mean queries per thread = number_of_queries/threads)
		size_t number_of_documents;				///< The number of documents in the collection
		size_t number_of_queries;					///< The number of queries that have been processed
		size_t wall_time_in_ns;						///< Total wall time to do all the search (in nanoseconds)
		size_t sum_of_CPU_time_in_ns;				///< Sum of the indivivual thread total timers (multi-threaded can be larger than wall_time_in_ns)
		size_t total_run_time_in_ns;				///< includes I/O and everything (start main() to end of main()).

	public:
		/*
			JASS_ANYTIME_STATS::JASS_ANYTIME_STATS()
			----------------------------------------
		*/
		/*!
			@brief Constructor
		*/
		JASS_anytime_stats() :
			threads(0),
			number_of_documents(0),
			number_of_queries(0),
			wall_time_in_ns(0),
			sum_of_CPU_time_in_ns(0),
			total_run_time_in_ns(0)
			{
			/* Nothing */
			}
	};

/*
	JASS_ANYTIME_STATS::OPERATOR<<()
	--------------------------------
*/
/*!
	@brief Dump a human readable version of the data down an output stream.
	@param stream [in] The stream to write to.
	@param data [in] The data to write.
	@return The stream once the data has been written.
*/
static inline std::ostream &operator<<(std::ostream &output, JASS_anytime_stats &data)
	{
	output << "-------------------\n";
	output << "Threads                                          : " << data.threads << '\n';
	output << "Queries                                          : " << data.number_of_queries << '\n';
	output << "Documents                                        : " << data.number_of_documents << '\n';
	output << "Total wall time for all queries (main loop time) : " << data.wall_time_in_ns << " ns\n";
	output << "Total CPU wall time searching (sum of threads)   : " << data.sum_of_CPU_time_in_ns << " ns\n";
	output << "Total time excluding I/O (per query)             : " << data.sum_of_CPU_time_in_ns / ((data.number_of_queries == 0) ? 1 : data.number_of_queries) << " ns\n";
	output << "Total wall clock run time (inc I/O and search)   : " << data.total_run_time_in_ns << " ns\n";
	output << "-------------------\n";
	return output;
	}
