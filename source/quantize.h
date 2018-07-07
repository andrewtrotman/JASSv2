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
	*/
	template <typename RANKER>
	class quantize : public index_manager::delegate, public index_manager::quantizing_delegate
		{
		private:
			double largest_rsv;												///< The largest score seen for any document/term pair.
			double smallest_rsv;												///< The smallest score seen for any document/term pair.
			std::shared_ptr<RANKER> ranker;								///< The ranker to use for quantization.
			compress_integer::integer documents_in_collection;		///< The number of documents in the collection.

			allocator_pool memory;											///< Memory used to store the postings lists.
			compress_integer::integer *document_ids;					///< The re-used buffer storing decoded document ids.
			index_postings_impact::impact_type *term_frequencies;	///< The re-used buffer storing the term frequencies.
			size_t temporary_size;											///< The number of bytes in temporary.
			uint8_t *temporary;												///< Temporary buffer - cannot be used to store anything between calls.

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
				largest_rsv(std::numeric_limits<decltype(largest_rsv)>::min()),
				smallest_rsv(std::numeric_limits<decltype(smallest_rsv)>::max()),
				ranker(ranker),
				documents_in_collection(documents),
				memory(1024 * 1024),
				document_ids((decltype(document_ids))memory.malloc(documents * sizeof(*document_ids))),
				term_frequencies((decltype(term_frequencies))memory.malloc(documents * sizeof(*term_frequencies))),
				temporary_size(documents * (sizeof(*document_ids) / 7 + 1) * sizeof(*temporary)),
				temporary((decltype(temporary))memory.malloc(temporary_size))			// enough space to decompress variable-byte encodings
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
			~quantize()
				{
				std::cout << "RSVmin:" << smallest_rsv << '\n';
				std::cout << "RSVmax:" << largest_rsv << '\n';
				}

			/*
				QUANTIZE::OPERATOR()()
				----------------------
			*/
			/*!
				@brief The callback function for each postings list is operator().
				@param term [in] The term name.
				@param postings [in] The postings list.
			*/
			virtual void operator()(const slice &term, const index_postings &postings)
				{
				/*
					Serialise and decompress the postings themselves
				*/
				auto document_frequency = postings.linearize(temporary, temporary_size, document_ids, term_frequencies, documents_in_collection);

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
					auto score = ranker->compute_score(*current_id - 1, *current_tf);

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
				@param term [in] The term name.
				@param postings [in] The postings list.
			*/
			virtual void operator()(index_manager::delegate &writer, const slice &term, const index_postings &postings)
				{
				writer(term, postings);
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
				@param serialiser [in] The serialiser that writes out in the desired format.
			*/
			void serialise_index(index_manager &index, std::vector<std::unique_ptr<index_manager::delegate>> &serialisers)
				{
				for (auto &outputter : serialisers)
					index.iterate(*this, *outputter);
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
				JASS_assert(static_cast<int>(largest) == 6);

				puts("quantize::PASSED");
				}
		};
	}
