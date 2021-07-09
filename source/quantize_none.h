/*
	QUANTIZE_NONE.H
	---------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Do not quantize the in-memory index (i.e. generate a tf-ordered index)
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include "quantize.h"

namespace JASS
	{
	/*
		CLASS QUANTIZE_NONE
		-------------------
	*/
	/*!
		@brief Do not quantize the in-memory index (i.e. generate a tf-ordered index)
	*/
	template <typename RANKER>
	class quantize_none : public quantize<RANKER>
		{
		public:
			/*
				QUANTIZE_NONE::QUANTIZE_NONE()
				------------------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] The number of documents in the collection.
				@param ranker [in] The ranking function used for quantization.
			*/
			quantize_none(size_t documents, std::shared_ptr<RANKER> ranker) :
				quantize<RANKER>(documents, ranker)
				{
				/* Nothing. */
				}

			/*
				QUANTIZE_NONE::~QUANTIZE_NONE()
				-------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~quantize_none()
				{
				/* Nothing */
				}

			/*
				QUANTIZE_NONE::OPERATOR()()
				---------------------------
			*/
			/*!
				@brief The callback function for each postings list is operator().
				@param term [in] The term name.
				@param postings [in] The postings list.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
			*/
			virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
				{
				/* Nothing. */
				}

			/*
				QUANTIZE_NONE::OPERATOR()()
				---------------------------
			*/
			/*!
				@brief The callback function for each postings list is operator().
				@param writer [in] The delegate that writes the quantized result to the output media.
				@param term [in] The term name.
				@param postings [in] The postings list.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
			*/
			virtual void operator()(index_manager::delegate &writer, const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
				{
				/*
					Pass the lisat straight to the writer.
				*/
				writer(term, postings, document_frequency, document_ids, term_frequencies);
				}

			/*
				QUANTIZE_NONE::OPERATOR()()
				---------------------------
			*/
			/*!
				@brief The callback function for primary keys (external document ids) is operator(). Not needed for quantization
				@param writer [in] A deligate object to manage the data once quantized.
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(index_manager::delegate &writer, size_t document_id, const slice &primary_key)
				{
				writer(document_id, primary_key);
				}

			/*
				QUANTIZE_NONE::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					As we do nothing, we automatically pass!
				*/
				puts("quantize_none::PASSED");
				}
		};
	}
