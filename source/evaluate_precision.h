/*
	EVALUATE_PRECISION.H
	--------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the precision of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_PRECISION
		------------------------
	*/
	/*!
		@brief Compute the precision given a results (where precision is found_and_relevant / found)
	*/
	class evaluate_precision : evaluate
		{
		private:
			evaluate &assessments;				///< The assessments, can be shared with other evaluation metrics
			
		public:
			/*
				EVALUATE_PRECISION::EVALUATE_PRECISION()
				----------------------------------------
			*/
			/*!
				@brief Constructor.
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate_precision(evaluate &assessments) :
				assessments(assessments)
				{
				}

			/*
				EVALUATE_PRECISION::COMPUTE()
				-----------------------------
			*/
			/*!
				@brief Compute the precision as found_and_relevant / found.
				@pram query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return the precison of this results list for this wuery.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = 1000);

			/*
				EVALUATE::UNITTEST()
				--------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);

		};
	}
