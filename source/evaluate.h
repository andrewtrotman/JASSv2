/*
	EVALUATE.H
	----------
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

#include <vector>
#include <string>

namespace JASS
	{
	/*
		CLASS EVALUATE
		--------------
	*/
	/*!
		@brief Base class for computing evaluaton metrics over a results list.
	*/
	class evaluate
		{
		protected:
			/*
				CLASS EVALUATE::JUDGEMENT
				-------------------------
			*/
			/*!
				@brief A (presumably human) assessment of a query / document pair.
			*/
			class judgement
				{
				public:
					std::string query_id;					///< the query id is NOT assumed to be an integer.
					std::string document_id;				///< the document id is assumed to be a string.
					double score;								///< the score is normally 0 or 1, but we might have graded assessmens.
					
				public:
					/*
						EVALUATE::JUDGEMENT::JUDGEMENT()
						--------------------------------
					*/
					/*!
						@brief Constructor.
						@param query_id [in] The id of the query.
						@param document_id [in] The id of the document.
						@param score [in] How relevant this document is to this query (0 is assumes to be non-relevant)
					*/
					judgement(const std::string &query_id, const std::string &document_id, double score) :
						query_id(query_id),
						document_id(document_id),
						score(score)
						{
						/* Nothing */
						}

					/*
						EVALUATE::JUDGEMENT::OPERATOR<()
						--------------------------------
					*/
					/*!
						@brief Less than comparison operator used in sort().
						@param second [in] a second object to compare to.
						@return true if this is strictly less than second.
					*/
					bool operator<(const judgement &second) const
						{
						/*
							sort of query id first
						*/
						auto compare = query_id.compare(second.query_id);
						if (compare < 0)
							return true;
						else if (compare > 0)
							return false;

						/*
							sort then on docment id
						*/
						compare = document_id.compare(second.document_id);
						if (compare < 0)
							return true;
						else if (compare > 0)
							return false;

						return false;
						}
				};

		public:
			std::vector<judgement> assessments;			///< The assessments once loaded from disk.
			judgement judgement_not_found;				///< The judgement to return if a search fails.

		public:

			evaluate() :
				judgement_not_found("", "", 0)
				{
				/* Nothing */
				}
		/*
			EVALUATE::LOAD_ASSESSMENTS_TREC_QRELS()
			---------------------------------------
		*/
		/*!
			@brief Load and decode a TREC assessment file.
			@param filename [in] The name of the file to load.
			@details
				from: https://trec.nist.gov/data/qrels_eng/index.html

				@verbatim

				The format of a qrels file is as follows:

				TOPIC      ITERATION      DOCUMENT#      RELEVANCY

				where TOPIC is the topic number,
				ITERATION is the feedback iteration (almost always zero and not used),
				DOCUMENT# is the official document number that corresponds to the "docno" field in the documents, and
				RELEVANCY is a binary code of 0 for not relevant and 1 for relevant.

				Sample Qrels File:

				1 0 AP880212-0161 0
				1 0 AP880216-0139 1
				1 0 AP880216-0169 0
				1 0 AP880217-0026 0
				1 0 AP880217-0030 0

				The order of documents in a qrels file is not indicative of relevance or degree of relevance.
				Only a binary indication of relevant (1) or non-relevant (0) is given. Documents not occurring
				in the qrels file were not judged by the human assessor and are assumed to be irrelevant in the
				evaluations used in TREC. The human assessors are told to judge a document relevant if any piece
				of the document is relevant (regardless of how small the piece is in relation to the rest of the
				document).

				@endverbatim
			*/
			void load_assessments_trec_qrels(std::string &filename);

			/*
				EVALUATE::DECODE_ASSESSMENTS_TREC_QRELS()
				-----------------------------------------
			*/
			/*!
				@brief Decode a TREC assessment file once loaded from disk
				@param data [in] The TREC assessment file as a string.
			*/
			void decode_assessments_trec_qrels(std::string &data);

			/*
				EVALUATE::FIND_FIRST()
				----------------------
			*/
			/*!
			 @brief Find and return the first relevant assessment for the given query.
			 @details This is used for eCommerce evaluation where the price of the k lowest priced items is needed
			 @param query_id [in] The ID of the query.
			 @param document_id [in] The ID of the document.
			 @return A pointer to the first judgement for the given query (or the next query if none exist for the query)
			*/
			auto find_first(const std::string &query_id) const
				{
				judgement looking_for(query_id, "", 0);

				return std::lower_bound(assessments.begin(), assessments.end(), looking_for);
				}

			/*
				EVALUATE::FIND()
				----------------
			*/
			/*!
				@brief Find and return the assessment data for the given query/document pair
				@param query_id [in] The ID of the query.
				@param document_id [in] The ID of the document.
				@return Either the judgement or a reference to a not-relevant (0) judgement if the pair has not been assessed.
			*/
			const judgement &find(const std::string &query_id, const std::string &document_id) const
				{
				judgement looking_for(query_id, document_id, 0);

				auto found = std::lower_bound(assessments.begin(), assessments.end(), looking_for);

				if (found->document_id == document_id && found->query_id == query_id)
					return *found;
				else
					return judgement_not_found;
				}

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
