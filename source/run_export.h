/*
	RUN_EXPORT.H
	------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Export a run in a standard run format
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/

#pragma once

#include "run_export_trec.h"

namespace JASS
	{
	/*
		CLASS RUN_EXPORT
		----------------
	*/
	/*!
		@brief Export a run in an evaluation forum format.
		@tparam QUERY_ID [in] the ID of the query, an alphanumeric sequence, normally a positive integer
		@tparam QUERY [in] The type of the query, nornally a JASS::query type
		@tparam NAME [in] the name of the run, normally a std::string or a char *
	*/
	class run_export
		{
		public:
			/*
				ENUM RUN_EXPORT::FORMAT
				-----------------------
			*/
			/*!
				@enum format
				@brief List of possible export formats
			*/
			enum format
				{
				TREC = 0					///< Export in TREC ad hoc format
				};

		public:
			/*
				RUN_EXPORT::RUN_EXPORT()
				------------------------
			*/
			/*!
				@brief Export a run in TREC run format for evaluation using trec_eval
				@param forum [in] which format to export as (see enum run_export::format for options)
				@param stream [in] The stream to write the run to
				@tparam QUERY_ID [in] the ID of the query, an alphanumeric sequence, normally a positive integer
				@param topic_id [in] The ID of this topic (can be alphanumeric, but no whitespace)
				@tparam QUERY [in] The type of the query, normally a JASS::query type
				@param result [in] The result set to export
				@tparam NAME [in] the name of the run, normally a std::string or a char *
				@param run_name [in] The name of the run
				@param include_internal_ids [in] if true then this method will include the internal document ids as part of the run name (default = false)
			*/
			template <typename QUERY_ID, typename QUERY, typename NAME>
			run_export(format forum, std::ostream &stream, const QUERY_ID &topic_id, QUERY &result, const NAME &run_name, bool include_internal_ids)
				{
				switch (forum)
					{
					case TREC:
						run_export_trec(stream, topic_id, result, run_name, include_internal_ids);
						break;
					default:
						JASS_assert(false);		//LCOVE_EXCL_LINE		// should never happen
						break;						//LCOVE_EXCL_LINE		// should never happen
					}
				}

			/*
				RUN_EXPORT::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<uint32_t>integer_sequence = {1, 1, 1, 1, 1, 1};
				std::vector<std::string>primary_keys = {"zero", "one", "two", "three", "four", "five", "six"};
				compress_integer_none identity;
				query<uint16_t, 100, 100> query(primary_keys, 10, 5);
				std::ostringstream result;

				decoder_d1 decoder(20);
				decoder.decode(identity, integer_sequence.size(), integer_sequence.data(), sizeof(integer_sequence[0]) * integer_sequence.size());
				decoder.process(1, query);

				run_export(run_export::TREC, result, "qid", query, "unittest", true);

				std::string correct_answer =
					"qid Q0 six 1 1 unittest (ID:6)\n"
					"qid Q0 five 2 1 unittest (ID:5)\n"
					"qid Q0 four 3 1 unittest (ID:4)\n"
					"qid Q0 three 4 1 unittest (ID:3)\n"
					"qid Q0 two 5 1 unittest (ID:2)\n";

				JASS_assert(result.str() == correct_answer);
				puts("run_export::PASSED");
				}
		};
	}
