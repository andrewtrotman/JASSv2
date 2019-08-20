/*
	RANKING_FUNCTION.H
	------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Helper functions for all the rankers
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <memory>

#include "compress_integer.h"
#include "index_postings_impact.h"
#include "ranking_function_atire_bm25.h"

namespace JASS
	{
	/*
		CLASS RANKING_FUNCTION
		----------------------
	*/
	/*!
		@brief Helper functions for rankers
	*/
	template <typename RANKER>
	class ranking_function
		{
		private:
			std::shared_ptr<RANKER>ranker;							///< pointer to the ranker

		public:
			/*
				RANKING_FUNCTION::RANKING_FUNCTION()
				------------------------------------
			*/
			/*!
				@brief Constructor
				@param ranker [in] a pointer to the ranker to use
			*/
			ranking_function(std::shared_ptr<RANKER>ranker) :
				ranker(ranker)
				{
				/* Nothing */
				}

			/*
				RANKING_FUNCTION::RANK()
				------------------------
			*/
			/*!
				@brief Compute the rsv from the ranker (inefficiently)
				@param document_id [in] The ID of the document to score (used for document lengths, etc).
				@param document_frequency [in] The number of documents containing this term.
				@param documents_in_collection [in] The number of documents in the collection.
				@param term_frequency [in] The number of times this term occurs in document document_id.
				@return The result of the ranking function (the rsv).
			*/
			double rank(compress_integer::integer document_id, compress_integer::integer document_frequency, compress_integer::integer documents_in_collection, index_postings_impact::impact_type term_frequency)
				{
				ranker->compute_idf_component(document_frequency, documents_in_collection);
				ranker->compute_tf_component(term_frequency);
				return ranker->compute_score(document_id, term_frequency);
				}

			/*
				RANKING_FUNCTION::UNITTEST()
				----------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				double rsv;

				std::vector<uint32_t> lengths{30, 40, 50, 60, 70};									// the lengths of the documents in this pseudo-index
				std::shared_ptr<ranking_function_atire_bm25>ranker(new ranking_function_atire_bm25(0.9, 0.4, lengths));							// k1=0.9, b=0.4
				ranking_function<ranking_function_atire_bm25> generic_ranker(ranker);		// construct a ranker

				rsv = generic_ranker.rank(1, 2, static_cast<uint32_t>(lengths.size()), 12);													// get the rsv
				JASS_assert(static_cast<uint32_t>(rsv * 1000) == 1635);

				puts("ranking_function::PASSED");
				}
		};
	}
