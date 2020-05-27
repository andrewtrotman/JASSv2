/*
	EVALUATE_MEAN_RECIPROCAL_RANK4K.H
	---------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the mean reciprocal rank of the mean position of k relevant results (the mean of the RR of the top k items)
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_MEAN_RECIPROCAL_RANK4K
		-------------------------------------
	*/
	/*!
		@brief Compute the mean reciprocal rank of the mean position of k relevant results (the mean of the RR of the top k items)
	*/
	class evaluate_mean_reciprocal_rank4k : public evaluate
		{
		private:
			size_t top_k;

		public:
			/*
				EVALUATE_MEAN_RECIPROCAL_RANK4K::EVALUATE_MEAN_RECIPROCAL_RANK4K()
				------------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
				@param top_k [in] The k relevant items to look for (which is not the depth to look).
				@param assessments [in] The assessments to use.
			*/
			evaluate_mean_reciprocal_rank4k(size_t top_k, std::shared_ptr<evaluate> assessments) :
				evaluate(assessments),
				top_k(top_k)
				{
				/* Nothing */
				}

			/*
				EVALUATE_MEAN_RECIPROCAL_RANK4K::COMPUTE()
				------------------------------------------
			*/
			/*!
				@brief Compute the mean reciprocal rank of the mean position of k relevant results (the mean of the RR of the top k items).
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_MEAN_RECIPROCAL_RANK4K::UNITTEST()
				-------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> results_list =
					{
					"AP880217-0026",
					"AP880216-0139",			// RELEVANT
					"AP880212-0161",
					"AP880216-0169",
					"AP880216-0139",			// RELEVANT
					};
				double true_score = (0.5 + 0.2) / 2;

				/*
					Load the sample data
				*/
				std::shared_ptr<evaluate> container(new evaluate);
				std::string copy = unittest_data::five_trec_assessments;
				container->decode_assessments_trec_qrels(copy);

				/*
					Evaluate the results list
				*/
				evaluate_mean_reciprocal_rank4k calculator(2, container);
				double calculated_score = calculator.compute("1", results_list);

				/*
					Compare to 5 decimal places
				*/
				JASS_assert(std::round(calculated_score * 10000) == std::round(true_score * 10000));

				puts("evaluate_mean_reciprocal_rank4k::PASSED");
				}
		};
	}
