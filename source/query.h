//#define SIMD_JASS_GROUP_ADD_RSV 1
//#define SIMD_JASS 1
/*
	QUERY.H
	-------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query.  Subclassed in order to get add_rsv()
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <immintrin.h>

#include "top_k_qsort.h"
#include "parser_query.h"
#include "query_term_list.h"
#include "allocator_memory.h"

namespace JASS
	{
	/*
		CLASS QUERY
		-----------
	*/
	/*!
		@brief Everything necessary to process a query is encapsulated in an object of this type
	*/
	class query
		{
		public:
			typedef uint16_t ACCUMULATOR_TYPE;									///< the type of an accumulator (probably a uint16_t)
//			typedef uint32_t ACCUMULATOR_TYPE;									///< the type of an accumulator (probably a uint16_t)
			typedef uint32_t DOCID_TYPE;											///< the type of a document id (from a compressor)
			static constexpr size_t MAX_DOCUMENTS = 55'000'000;			///< the maximum number of documents an index can hold
			static constexpr size_t MAX_TOP_K = 1'000;						///< the maximum top-k value

		public:
			/*
				CLASS QUERY::ADD_RSV_COMPARE
				----------------------------
			*/
			/*!
				@brief Functor that does the comparison (looking for a < b, if a == b then pointer compare).
			*/
			class add_rsv_compare
				{
				public:
					/*
						QUERY::ADD_RSV_COMPARE::OPERATOR()
						----------------------------------
					*/
					/*!
						@brief Compare value then pointer for greater, equal, less then.
						@param a [in] first pointer.
						@param b [in] second pointer
						@return 1 if greater, 0 if equal, -1 is less
					*/
					forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const
						{
						/*
							The most likely case is that the value at a is less than the value at b so do that check first.
						*/
						return *a < *b ? -1 : *a > *b ? 1 : a < b ? -1 : a == b ? 0 : 1;
						}
				};
			/*
				CLASS QUERY::SORT_RSV_COMPARE
				-----------------------------
			*/
			/*!
				@brief Functor that does the comparison (looking for a > b, if a == b then pointer compare).
			*/
			class sort_rsv_compare
				{
				public:
					/*
						QUERY::SORT_RSV_COMPARE::OPERATOR()
						----------------------------------
					*/
					/*!
						@brief Compare value then pointer for greater, equal, less then.
						@param a [in] first pointer.
						@param b [in] second pointer
						@return 1 if greater, 0 if equal, -1 is less
					*/
					forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const
						{
						/*
							The most likely case is that the value at a is less than the value at b so do that check first.
						*/
						return *a < *b ? 1 : *a > *b ? -1 : a < b ? 1 : a == b ? 0 : -1;
						}
				};

		protected:
#ifdef SIMD_JASS_GROUP_ADD_RSV
			__m256i impacts;																///< The impact score to be added on a call to add_rsv()
#endif
			ACCUMULATOR_TYPE impact;													///< The impact score to be added on a call to add_rsv()

			allocator_pool memory;														///< All memory allocation happens in this "arena"
			std::vector<uint32_t> decompress_buffer;								///< The delta-encoded decopressed integer sequence.
			DOCID_TYPE documents;														///< The numnber of documents this index contains

			parser_query parser;															///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;											///< The parsed query
			const std::vector<std::string> *primary_keys;						///< A vector of strings, each the primary key for the document with an id equal to the vector index

		public:
			size_t top_k;																	///< The number of results to track.
			add_rsv_compare cmp;															///< Comparison during addition (used to order low to high a min heap)
			sort_rsv_compare final_sort_cmp;											///< Comparison after search (used to order high to low)

		public:
			/*
				QUERY::QUERY()
				--------------
			*/
			/*!
				@brief Constructor
			*/
			query() :
				parser(memory),
				parsed_query(nullptr),
				primary_keys(nullptr),
				top_k(0)
				{
				}

			/*
				QUERY::INIT()
				-------------
			*/
			/*!
				@brief Initialise the object. MUST be called before first use.
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			virtual void init(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10)
				{
				this->primary_keys = &primary_keys;
				this->top_k = top_k;
				this->documents = documents;
				decompress_buffer.resize(documents);
				rewind();
				}

			/*
				QUERY::~QUERY()
				---------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query()
				{
				delete parsed_query;
				}

			/*
				QUERY::PARSE()
				--------------
			*/
			/*!
				@brief Take the given query and parse it.
				@tparam STRING_TYPE Either a std::string or JASS::string.
				@param query [in] The query to parse.
			*/
			template <typename STRING_TYPE>
			void parse(const STRING_TYPE &query)
				{
				parser.parse(*parsed_query, query);
				}

			/*
				QUERY::TERMS()
				--------------
			*/
			/*!
				@brief Return a reference to the parsed query.
				@return A reference to the parsed query.
			*/
			query_term_list &terms(void)
				{
				return *parsed_query;
				}

			/*
				QUERY::REWIND()
				---------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				delete parsed_query;
				parsed_query = new query_term_list(memory);
				}

			/*
				QUERY::SET_IMPACT()
				-------------------
			*/
			/*!
				@brief Set the impact score to use in a push_back().
				@param score [in] The impact score to be added to accumulators.
			*/
			forceinline void set_impact(ACCUMULATOR_TYPE score)
				{
#ifdef SIMD_JASS_GROUP_ADD_RSV
				impacts = _mm256_set1_epi32(score);
#endif
				impact = score;
				}

			/*
				QUERY_HEAP::DECODE_AND_PROCESS()
				--------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param impact [in] The impact score to add for each document id in the list.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			forceinline void decode_and_process(ACCUMULATOR_TYPE impact, size_t integers, const void *compressed, size_t compressed_size)
				{
				set_impact(impact);
//std::cout << "\n[" << impact << "]";
				decode_with_writer(integers, compressed, compressed_size);
				}

			/*
				QUERY::DECODE_WITH_WRITER()
				---------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			virtual void decode_with_writer(size_t integers, const void *compressed, size_t compressed_size)
				{
				}

			/*
				QUERY_HEAP::DECODE()
				--------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(DOCID_TYPE *decoded, size_t integers_to_decode, const void *source, size_t source_length)
				{
				}
		};
	}
