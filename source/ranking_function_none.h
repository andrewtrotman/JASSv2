/*
	RANKING_FUNCTION_NONE.H
	-----------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Ranking function that returns the TF score - used for term frequency scaling into the quantized range.
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman
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
		CLASS RANKING_FUNCTION_NONE
		---------------------------
	*/
	/*!
		@brief Ranking function that returns the TF score - used for term frequency scaling into the quantized range.
	*/
	class ranking_function_none
		{
		public:
			/*
				RANKING_FUNCTION_NONE::RANKING_FUNCTION_NONE()
				----------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			ranking_function_none()
				{
				/* Nothing */
				}

			/*
				RANKING_FUNCTION_NONE::~RANKING_FUNCTION_NONE()
				-----------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			~ranking_function_none()
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
				/* Nothing */
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
				/* Nothing */
				}

			/*
				RANKING_FUNCTION_ATIRE_BM25::COMPUTE_SCORE()
				--------------------------------------------
			*/
			/*!
				@brief Return the term frequency.
				@param document_id [in] The ID of the document
				@param term_frequency [in] The number of times the term occurs in the document.
				@return The term frequency score
			*/
			forceinline double compute_score(compress_integer::integer document_id, index_postings_impact::impact_type term_frequency)
				{
				return term_frequency;
				}
		};
	}
