/*
	RANKING_FUNCTION_ATIRE_BM25.H
	-----------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The ATIRE verison of the BM25 ranking function
	@details A version of the BM25 ranking function that does not return negative numbers.  Taken from ATIRE where it was written by Andrew Trotman, and derived by Andrew Trotman and Shlomo Geva
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <vector>

#include "forceinline.h"
#include "compress_integer.h"
#include "index_postings_impact.h"


namespace JASS
	{
	/*
		CLASS RANKING_FUNCTION_ATIRE_BM25
		---------------------------------
	*/
	/*!
		@brief The ATIRE verison of BM25
	*/
	class ranking_function_atire_bm25
		{
		private:
			double idf;												///< the IDF of the term being processed
			double top_row;											///< the top-row of the ranking function for the term being processed (tf(td) * (k1 + 1))
			double k1_plus_1;										///< k1 + 1
			double mean_document_length;							///< the mean of the document lengths
			std::vector<float> length_correction;					///< most of the bottom row of BM25 (k1 * ((1 - b) + b * length / mean_document_length)) for the current term being processed

		public:
			/*
				RANKING_FUNCTION_ATIRE_BM25::RANKING_FUNCTION_ATIRE_BM25()
				----------------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param k1 [in] the BM25 k1 parameter, 0.9 is a good value.
				@param b [in] the BM25 b parameter, 0.4 is a good value.
				@param document_lengths [in] a vector holding the length of each document in the collection.
			*/
			ranking_function_atire_bm25(double k1, double b, std::vector<compress_integer::integer> &document_lengths):
				idf(0),
				top_row(0),
				k1_plus_1(k1 + 1.0),
				mean_document_length(0),
				length_correction(document_lengths.size())
				{
				double one_minus_b = 1.0 - b;

				uint64_t sum = 0;
				for (auto length : document_lengths)
					sum += length;

				mean_document_length = static_cast<double>(sum) / static_cast<double>(document_lengths.size() - 1);			// -1 because ID 0 is not used (and should be 0)

				auto correction = &length_correction[0];			// recall that we count from 1, not from 0
				for (auto length : document_lengths)
					*correction++ =  k1 * (one_minus_b + b * static_cast<double>(length) / mean_document_length);
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::~RANKING_FUNCTION_ATIRE_BM25()
				----------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			~ranking_function_atire_bm25()
				{
				/* Nothing */
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::COMPUTE_IDF_COMPONENT()
				----------------------------------------------------
			*/
			/*!
				@brief Called once per term (per query).  Computes the IDF component of the ranking function and stores it internally
				@param document_frequency [in] the number of documents that contain this term.
				@param documents_in_collection [in] the number of documents in the collection.
			*/
			forceinline void compute_idf_component(compress_integer::integer document_frequency, compress_integer::integer documents_in_collection)
				{
				/*
								 N
					IDF = log -
								 n

					This variant of IDF is better than log((N - n + 0.5) / (n + 0.5)) on the 70 INEX 2008 Wikipedia topics.  Also, it is always positive.
				*/
				idf = log((double)documents_in_collection / (double)document_frequency);
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::COMPUTE_TF_COMPONENT()
				---------------------------------------------------
			*/
			/*!
				@brief Compute and store internally the term-frequency based component of the ranking function (useful when postings lists are impact ordered)
				@param term_frequency [in] The number of times the term occurs in the document.
			*/
			forceinline void compute_tf_component(index_postings_impact::impact_type term_frequency)
				{
				top_row = term_frequency * k1_plus_1;
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::COMPUTE_SCORE()
				--------------------------------------------
			*/
			/*!
				@brief Compute BM25 from the given document, assuming pieces have already been computed.
				@details First compute the IDF for the term using compute_idf_component(), then the TF component using compute_tf_component(), then call this.
				@param document_id [in] The ID of the document (used to look up the length)
				@param term_frequency [in] The number of times the term occurs in the document.
			*/
			forceinline double compute_score(compress_integer::integer document_id, index_postings_impact::impact_type term_frequency)
				{
				/*
										tf(td) * (k1 + 1)
					rsv = ----------------------------------- * IDF
																 len(d)
							tf(td) + k1 * (1 - b + b * --------)
																av_len_d

					In this implementation we ignore k3 and the number of times the term occurs in the query.
				*/
				double tf = term_frequency;
				return idf * (top_row / (tf + length_correction[document_id]));
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::UNITTEST()
				---------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				double rsv;
				std::vector<compress_integer::integer> lengths{30, 40, 50, 60, 70};			// the lengths of the documents in this pseudo-index
				ranking_function_atire_bm25 ranker(0.9, 0.4, lengths);	// k1=0.9, b=0.4

				ranker.compute_idf_component(2, static_cast<uint32_t>(lengths.size()));			// this term occurs in 2 of 5 documents
				ranker.compute_tf_component(12);									// it occurs in this document 12 times
				rsv = ranker.compute_score(1, 12);								// it occurs in document 1 a total of 12 times;

				JASS_assert(static_cast<uint32_t>(rsv * 1000) == 1635);
				puts("ranking_function_atire_bm25::PASSED");
				}
		};
	}
