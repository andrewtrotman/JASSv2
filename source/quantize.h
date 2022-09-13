/*
	QUANTIZE.H
	----------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Quantize the in-memory index
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <math.h>

#include <iostream>

#include "index_manager.h"
#include "index_manager_sequential.h"
#include "ranking_function_atire_bm25.h"

namespace JASS
	{
	/*
		CLASS QUANTIZE
		--------------
	*/
	/*!
		@brief Quantize an index
		@details Generic quantization class that performs uniform quantization according to the equations in
		V. N. Anh, O. de Kretser, A. Moffat (2001) Vector-space ranking with effective early termination. SIGIR 2001, PP.35-42.
		The ranking function itself is a template parameter, and also passed to the constructor as the ranker
		might need initialisation (BM25 does).

		Uniform quantization if most effedctive for BM25 was BM25 has an exponential decay in the rsv scores and
		so high impact segments are short and low impact scores are long.  The best documents have high impact scores
		for each query term and so have high result list rsvs are rare.  Uniform quantization also does not require
		decoding so the cost of ranking is an integer add!
	*/
	template <typename RANKER>
	class quantize : public index_manager::delegate, public index_manager::quantizing_delegate
		{
		private:
			double largest_rsv;												///< The largest score seen for any document/term pair.
			double smallest_rsv;												///< The smallest score seen for any document/term pair.
			std::shared_ptr<RANKER> ranker;								///< The ranker to use for quantization.
			compress_integer::integer documents_in_collection;		///< The number of documents in the collection.
			static constexpr double impact_range = index_postings_impact::largest_impact - index_postings_impact::smallest_impact; ///< The number of values in the impact ordering range (normally 255).

		public:
			/*
				QUANTIZE::QUANTIZE()
				--------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] The number of documents in the collection.
				@param ranker [in] The ranking function used for quantization.
			*/
			quantize(size_t documents, std::shared_ptr<RANKER> ranker) :
				index_manager::delegate(documents),
				largest_rsv((std::numeric_limits<decltype(largest_rsv)>::min)()),
				smallest_rsv((std::numeric_limits<decltype(smallest_rsv)>::max)()),
				ranker(ranker),
				documents_in_collection(static_cast<compress_integer::integer>(documents))
				{
				/* Nothing. */
				}

			/*
				QUANTIZE::~QUANTIZE()
				---------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~quantize()
				{
//				std::cout << "RSVmin:" << smallest_rsv << '\n';
//				std::cout << "RSVmax:" << largest_rsv << '\n';
				}

			/*
				QUANTIZE::FINISH()
				------------------
			*/
			/*!
				@brief Do any final cleaning up
			*/
			virtual void finish(void)
				{
				/* Nothing */
				}

			/*
				QUANTIZE::OPERATOR()()
				----------------------
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
				/*
					Compute the IDF component
				*/
				ranker->compute_idf_component(document_frequency, documents_in_collection);

				/*
					Compute the document / term score and keep a tally of the smallest and largest (for quantization)
				*/
				auto end = document_ids + document_frequency;
				auto current_tf = term_frequencies;
				for (compress_integer::integer *current_id = document_ids; current_id < end; current_id++, current_tf++)
					{
					/*
						Compute the term / document score
					*/
					ranker->compute_tf_component(*current_tf);
					auto score = ranker->compute_score(*current_id, *current_tf);

					/*
						Keep a running tally of the largest and smallest rsv we've seen so far
					*/
					if (score < smallest_rsv)
						smallest_rsv = score;
					if (score > largest_rsv)
						largest_rsv = score;
					}
				}

			/*
				QUANTIZE::OPERATOR()()
				----------------------
			*/
			/*!
				@brief The callback function for primary keys (external document ids) is operator(). Not needed for quantization
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key)
				{
				/* Nothing. */
				}

			/*
				QUANTIZE::OPERATOR()()
				----------------------
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
					Compute the IDF component
				*/
				ranker->compute_idf_component(document_frequency, documents_in_collection);

				/*
					Compute the document / term score and quantize it.
				*/
				auto end = document_ids + document_frequency;
				auto current_tf = term_frequencies;
				for (compress_integer::integer *current_id = document_ids; current_id < end; current_id++, current_tf++)
					{
					/*
						Compute the term / document score
					*/
					ranker->compute_tf_component(*current_tf);
					double score = ranker->compute_score(*current_id, *current_tf);

					/*
						Quantize using uniform quantization, and write back as the new term frequency (which is now an impact score).
						This uses Uniform Quantiization as defined by Anh et al. in:
						Vo Ngoc Anh, Owen de Kretser, and Alistair Moffat. 2001. Vector-space ranking with effective early termination. In Proceedings of the 24th annual international ACM SIGIR conference on Research and development in information retrieval (SIGIR '01). ACM, New York, NY, USA, 35-42. DOI: https://doi.org/10.1145/383952.383957
					*/
					index_postings_impact::impact_type impact = static_cast<index_postings_impact::impact_type>(((score - smallest_rsv) / (largest_rsv - smallest_rsv)) * impact_range) + index_postings_impact::smallest_impact;
					*current_tf = impact;
					}

				/*
					Pass the quantized list to the writer.
				*/
				writer(term, postings, document_frequency, document_ids, term_frequencies);
				}

			/*
				QUANTIZE::OPERATOR()()
				----------------------
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
				QUANTIZE::GET_BOUNDS()
				----------------------
			*/
			/*!
				@brief Get the smallest and largest term / document influence (should be called after the first round of the quantizer).
				@param smallest [out] This collection's smallest term / document influence.
				@param largest [out] This collection's largest term / document influence.
			*/
			void get_bounds(double &smallest, double &largest)
				{
				smallest = smallest_rsv;
				largest = largest_rsv;
				}

			/*
				QUANTIZE::SERIALISE_INDEX()
				---------------------------
			*/
			/*!
				@brief Given the index and a serialiser, serialise the index to disk.
				@param index [in] The index to serialise.
				@param serialisers [in] The serialiser that writes out in the desired format.
			*/
			void serialise_index(index_manager &index, std::vector<std::unique_ptr<index_manager::delegate>> &serialisers)
				{
				for (auto &outputter : serialisers)
					{
					index.iterate(*this, *outputter);
					outputter->finish();
					}
				}

			/*
				QUANTIZE::UNITTEST()
				--------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Build an index.
				*/
				index_manager_sequential index;
				index_manager_sequential::unittest_build_index(index, unittest_data::ten_documents);

				/*
					Quantize the index.
				*/
				std::shared_ptr<ranking_function_atire_bm25> ranker(new ranking_function_atire_bm25(0.9, 0.4, index.get_document_length_vector()));
				quantize<ranking_function_atire_bm25> quantizer(index.get_highest_document_id(), ranker);
				index.iterate(quantizer);

				double smallest;
				double largest;
				quantizer.get_bounds(smallest, largest);

				JASS_assert(static_cast<int>(smallest) == 0);
				JASS_assert(static_cast<int>(largest) == 2);

				puts("quantize::PASSED");
				}
		};
	}
