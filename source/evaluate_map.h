/*
	EVALUATE_MAP.H
	--------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the mean average precision (MAP) of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_MAP
		------------------
	*/
	/*!
		@brief Compute the mean average precision (MAP) given a results
	*/
	class evaluate_map : evaluate
		{
		private:
			evaluate &assessments;				///< The assessments, can be shared with other evaluation metrics
			
		public:
			/*
				EVALUATE_MAP::EVALUATE_MAP()
				----------------------------
			*/
			/*!
				@brief Constructor.
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate_map(evaluate &assessments) :
				assessments(assessments)
				{
				/* Nothing */
				}

			/*
				EVALUATE_MAP::RELEVANCE_COUNT()
				-------------------------------
			*/
			/*!
				@brief Count the number of relevant assessments for this query.
				@param query_id [in] The ID of the query being evaluated.
				@return the number of relevant assessments for this query.
			*/
			size_t relevance_count(const std::string &query_id) const;

			/*
				EVALUATE_MAP::COMPUTE()
				-----------------------
			*/
			/*!
				@brief Compute the Mean Average Precision (MAP).
				@param query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return the MAP of this results list for this query.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = std::numeric_limits<size_t>::max());

			/*
				EVALUATE_PRECISION::UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
