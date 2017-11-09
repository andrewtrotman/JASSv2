/*
	RUN_EXPORT_TREC.H
	-----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Export a run in TREC run format
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "decode_d1.h"

namespace JASS
	{
	/*
		CLASS RUN_EXPORT_TREC
		---------------------
	*/
	/*!
		@brief Export a JASS run in TREC format
		@details TREC format is 6-column, each seperated by a space, see: http://www-nlpir.nist.gov/projects/t01v/trecvid.tools/trec_eval_video/A.README
		The format is a series of rows (one per result in the results list) of <topic-id query-iteration primary-key, rank, retrieval-status-value, run-name>
		for example:
		703 Q0 WSJ870918-0107 1 130 RUNNAME
	*/
	class run_export_trec
		{
		public:
			/*
				RUN_EXPORT_TREC::OPERATOR()()
				-----------------------------
			*/
			/*!
				@brief Export a run in TREC run format for evaluation using trec_eval
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
			run_export_trec(std::ostream &stream, const QUERY_ID &topic_id, QUERY &result, const NAME &run_name, bool include_internal_ids = false)
				{
				size_t current = 0;
				for (const auto &document : result)
					{
					current++;
					stream << topic_id << " Q0 "<< document.primary_key << ' ' << current << ' ' << document.rsv << ' ' << run_name;

					/*
						Optionally include the internal document id for debugging purposes.
					*/
					if (include_internal_ids)
						stream << " (ID:" << document.document_id << ")";

					stream << '\n';
					}
				}

			/*
				RUN_EXPORT_TREC::UNITTEST()
				---------------------------
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

				run_export_trec(result, "qid", query, "unittest", true);

				std::string correct_answer =
					"qid Q0 six 1 1 unittest (ID:6)\n"
					"qid Q0 five 2 1 unittest (ID:5)\n"
					"qid Q0 four 3 1 unittest (ID:4)\n"
					"qid Q0 three 4 1 unittest (ID:3)\n"
					"qid Q0 two 5 1 unittest (ID:2)\n";

				JASS_assert(result.str() == correct_answer);
				puts("run_export_trec::PASSED");
				}
		};
	}
