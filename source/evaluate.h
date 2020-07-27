/*
	EVALUATE.H
	----------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Base class for metrics (precision, recall, MAP, etc).
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "unittest_data.h"

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
		friend std::ostream &operator<<(std::ostream &stream, const evaluate &object);

		public:
			/*
				CLASS EVALUATE::JUDGEMENT
				-------------------------
			*/
			/*!
				@brief A (presumably human) assessment of a query / document pair.
			*/
			class judgement
				{
				friend bool operator<(evaluate::judgement &a, evaluate::judgement &b);

				public:
					std::string query_id;					///< the query id is NOT assumed to be an integer.
					std::string document_id;				///< the document id is assumed to be a string.
					double score;								///< the score is normally 0 or 1, but we might have graded assessmens.
					
				public:
					/*
						EVALUATE::JUDGEMENT::JUDGEMENT()
						--------------------------------
					*/
					judgement() :
						score(0)
						{
						/* Nothing */
						}
						
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
							sort on query id first
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

			/*
				CLASS JUDGEMENT_LESS_THAN
				-------------------------
			*/
			/*!
				@brief Comparison object for lower_bound() calls
				@details Visual Studio 2019 appears to need this, g++ and clang can use judgement::operator<()
			*/
			class judgement_less_than
				{
				public:
					/*
						JUDGEMENT_LESS_THAN::OPERATOR()
						-------------------------------
					*/
					/*!
						@brief Compare two objects for less than.
						@param left [in] the left hand side.
						@param right [in] the right hand side.
						@return true if left < right, else false.
					*/
					bool operator() (const judgement &left, const judgement &right)
						{
						return left.operator<(right);
						}
				};
				
			static const judgement_less_than judgement_less_than_comparator;				///< the object used for comparison

		protected:
			std::vector<judgement> assessments_store;			///< The assessments once loaded from disk.
			std::vector<judgement> prices_store;				///< The assessments once loaded from disk.
			std::shared_ptr<evaluate> assessments_pointer;	///< The shared pointer used to create this object
			std::shared_ptr<evaluate> prices_pointer;			///< The shared pointer used to create this object

		public:
			std::vector<judgement> &assessments;				///< The assessments.
			std::vector<judgement> &prices;						///< The prices used for eCommerce metrics.
			judgement judgement_not_found;						///< The judgement to return if a search fails.

		public:
			/*
				EVALUATE::EVALUATE()
				--------------------
			*/
			/*!
				@brief Constructor.
			*/
			evaluate() :
				assessments(assessments_store),
				prices(prices_store),
				judgement_not_found("", "", 0)
				{
				/* Nothing */
				}

			/*
				EVALUATE::EVALUATE()
				--------------------
			*/
			/*!
				@brief Constructor.
				@param store [in] an evaluation object with assessments already loaded.
			*/
			evaluate(std::shared_ptr<evaluate> store) :
				assessments_pointer(store),
				prices_pointer(store),
				assessments(store->assessments),
				prices(store->prices),
				judgement_not_found("", "", 0)
				{
				/* Nothing */
				}

			/*
				EVALUATE::EVALUATE()
				--------------------
			*/
			/*!
				@brief Constructor.
				@details  As all possible prices are valid prices (0 == "free", -1 == "I'll pay for you to take it away), the
				assessments are split into two seperate parts. Ther prices of the items and the relevance of the items.  Each
				of these two are stored in trec_eval format:

			 	1 0 AP880212-0161 1

			 	where the first column is the query id, the second is ignored, the third is the document ID, and the fourth is the
			 	relevance.  The prices use a query id of "PRICE" and the relevance coulmn is the price of the item.  The assessments
			 	are the usual trec_eval format where a relevance of 1 means releance, but a relefvance of 0 is not-relevant.

				@param prices [in] An assessments object which holds the price of each item
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate(std::shared_ptr<evaluate> prices, std::shared_ptr<evaluate> assessment_store) :
				assessments_pointer(assessment_store),
				prices_pointer(prices),
				assessments(assessment_store->assessments_store),
				prices(prices->assessments_store),
				judgement_not_found("", "", 0)
				{
				/* Nothing */
				}

			/*
				EVALUATE::~EVALUATE()
				---------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~evaluate()
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
			 @return A pointer to the first judgement for the given query (or the next query if none exist for the query)
			*/
			auto find_first(const std::string &query_id) const
				{
				judgement looking_for(query_id, "", 0);

				#ifdef _MSC_VER
					return std::lower_bound(assessments.begin(), assessments.end(), looking_for, judgement_less_than_comparator);
				#else
					return std::lower_bound(assessments.begin(), assessments.end(), looking_for);
				#endif
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

				#ifdef _MSC_VER
					auto found = std::lower_bound(assessments.begin(), assessments.end(), looking_for, judgement_less_than_comparator);
				#else
					auto found = std::lower_bound(assessments.begin(), assessments.end(), looking_for);
				#endif

				if (found == assessments.end())
					return judgement_not_found;
				else if (found->document_id == document_id && found->query_id == query_id)
					return *found;
				else
					return judgement_not_found;
				}

			/*
				EVALUATE::FIND_PRICE()
				----------------------
			*/
			/*!
				@brief Find and return the price of a document
				@param document_id [in] The ID of the document.
				@return Either the judgement or a reference to a not-relevant (0) judgement if the pair has not been assessed.
			*/
			const judgement &find_price(const std::string &document_id) const
				{
				judgement looking_for("PRICE", document_id, 0);

				#ifdef _MSC_VER
					auto found = std::lower_bound(prices.begin(), prices.end(), looking_for, judgement_less_than_comparator);
				#else
					auto found = std::lower_bound(prices.begin(), prices.end(), looking_for);
				#endif

				if (found == prices.end())
					return judgement_not_found;
				else if (found->document_id == document_id)
					return *found;
				else
					return judgement_not_found;
				}


			/*
				EVALUATE::COMPUTE()
				-------------------
			*/
			/*!
				@brief Compute the metric.
				@param query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return The score for this results list and for this query.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const
				{
				return 0;
				}

			/*
				EVALUATE::RELEVANCE_COUNT()
				---------------------------
			*/
			/*!
				@brief Count the numner of relevant results for this query
				@param query_id [in] the ID of the query
			*/
			size_t relevance_count(const std::string &query_id) const
				{
				size_t relevant = 0;

				for (auto assessment = find_first(query_id); assessment != assessments.end(); assessment++)
					if (query_id == assessment->query_id && assessment->score != 0)
						relevant++;

				return relevant;
				}

			/*
				EVALUATE::QUERY_COUNT()
				-----------------------
			*/
			/*!
				@brief Count the number of queries in the assessments
			*/
			size_t query_count() const
				{
				std::string previous_query_id;
				size_t queries = 0;

				for (const auto &assessment : assessments)
					if (assessment.query_id != previous_query_id)
						{
						queries++;
						previous_query_id = assessment.query_id;
						}

				return queries;
				}

			/*
				EVALUATE::UNITTEST_ONE()
				------------------------
			*/
			/*!
				@brief Run a unit test on a commmon set of data
				@param true_score [in] the expected result for the standard test
				@tparam TYPE [in] The type of evaluation to perform (the metric)
			*/
			template <typename TYPE>
			static void unittest_one(double true_score)
				{
				/*
					Example results list with one relevant document
				*/
				std::vector<std::string> results_list =
					{
					"AP880217-0026",
					"AP880216-0139",			// RELEVANT (all others are non-relevant).
					"AP880212-0161",
					"AP880216-0169",
					"AP880217-0030",
					};

				/*
					Load the sample data
				*/
				std::shared_ptr<evaluate> container(new evaluate);
				std::string copy = unittest_data::five_trec_assessments;
				container->decode_assessments_trec_qrels(copy);

				/*
					Evaluate the results list
				*/
				TYPE calculator(container);
				double calculated_score = calculator.compute("1", results_list);

				/*
					Compare to 5 decimal places
				*/
				JASS_assert(std::round(calculated_score * 10000) == std::round(true_score * 10000));
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

	/*
		OPERATOR<()
		-----------
	*/
	/*!
		@brief comparison operator
		@param left [in] left hand side
		@param right [in] right hand side
		@return true if left < right, else false
	*/
	inline bool operator<(evaluate::judgement &left, evaluate::judgement &right)
		{
		return left.operator<(right);
		}

	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of an object down an output stream
		@param stream [in] The stream to write to.
		@param tree [in] The object to write.
		@return The stream once the tree has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const evaluate &set)
		{
		/*
			Output in trec_eval format.
		*/
		for (const auto &[query_id, document_id, score] : set.assessments)
			stream << query_id << " 0 " << document_id << " " << score << "\n";
		return stream;
		}
	}
