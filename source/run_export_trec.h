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
				@param topic_id [in] The ID of this topic (can be alphanumeric, but no whitespace)
				@param result [in] The result set to export
				@param run_name [in] The name of the run
				@param include_internal_ids [in] if true then this method will include the internal document ids as part of the run name
			*/
			template <typename QUERY_ID, typename QUERY, typename NAME>
			operator()(std::ostream &stream, const QUERY_ID &topic_id, QUERY &result, const NAME &run_name, bool include_internal_ids)
				{
				size_t current = 0;
				for (const auto &document : result)
					{
					current++;
					stream << topic_id << " Q0 "<< document.primary_key << " " << current << " " << document.rsv << run_name;

					/*
						Optionally include the internal document id for debugging purposes.
					*/
					if (include_internal_ids)
						stream << " (ID:" << document.document_id << ")";

					stream << '\n';
					}
				}
		};
	}
