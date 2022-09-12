#ifdef JASS_HAS_EXTERNAL_CONFIGURATION
	#define QUOTEME(x) QUOTEME_1(x)
	#define QUOTEME_1(x) #x
	#define INCLUDE_FILE(x) QUOTEME(x)
	#include INCLUDE_FILE(JASS_HAS_EXTERNAL_CONFIGURATION)
	#error "You should not be doing this"
#else
	#define QUERY_HEAP
	#define ACCUMULATOR_STRATEGY_2D
	#define JASS_TOPK_SORT

	/*
		On Windows the use of std::sort() appears to be the best trade-off (that is, CPP_SORT)
	*/
#endif

/*
	Which top-k algorithm to use.  There is no default, one of these MUST be defined

	QUERY_HEAP uses the heap to store the rsvs
	QUERY_BUCKETS uses the bucket apprach to the top-k, the alternative is the query_heap
	QUERY_MAXBLOCK uses the max-block approach to the top-k, the alternative is the query_heap
	QUERY_MAXBLOCK_HEAP uses the max-block approach to the top-k (in a heap), the alternative is the query_heap
*/
//#define QUERY_HEAP
//#define QUERY_BUCKETS
//#define QUERY_MAXBLOCK
//#define QUERY_MAXBLOCK_HEAP

/*
	Which accmulator allocator strategy to use

	ACCUMULATOR_STRATEGY_2D uses the ATIRE 2D accumulator dirty pages strategy
	ACCUMULATOR_COUNTER_8 uses an 8-bit counter per accumulator incremented with each query, in they do not match then zero just that one accumulator
	ACCUMULATOR_COUNTER_4 uses an 4-bit counter per accumulator incremented with each query, in they do not match then zero just that one accumulator
	ACCUMULATOR_COUNTER_INTERLEAVED_8 same as ACCUMULATOR_COUNTER_8, but interleaved the flags with the accumulators, and uses 21 accumulators per chunk (so flag + accumulators approximately equals cachline size)
	ACCUMULATOR_COUNTER_INTERLEAVED_8_1 same as ACCUMULATOR_COUNTER_8, but interleaved the flags with the accumulators, one flag per accumulator
	ACCUMULATOR_COUNTER_INTERLEAVED_4 same as ACCUMULATOR_COUNTER_4, but interleaved the flags with the accumulators, and uses 25 accumulators per chunk (so flag + accumulators approximately equals cachline size)
*/
//#define ACCUMULATOR_STRATEGY_2D
//#define ACCUMULATOR_COUNTER_8
//#define ACCUMULATOR_COUNTER_4
//#define ACCUMULATOR_COUNTER_INTERLEAVED_8
//#define ACCUMULATOR_COUNTER_INTERLEAVED_8_1
//#define ACCUMULATOR_COUNTER_INTERLEAVED_4

/*
	ACCUMULATOR_POINTER_BEAP uses a beap of pointers (rather than a heap of pointers) to store the top-k in the query_heap code.
	The other query_ classes don't change values in a heap and so a beap will always be slower (as heap is O(log(n)) but beap is O(sqrt(N)))
*/
//#define ACCUMULATOR_POINTER_BEAP 1

/*
	ACCUMULATOR_64s uses the 64-bit unsigned integer encoding of the RSV and DocID - useful for faster sorting (see AVX512_SORT)
	The alternative is to use pointers to accumulators and sort the pointers.
*/
//#define ACCUMULATOR_64s 1

/*
	Which sort algorithm to use.  There is no default, one of these MUST be defined

	JASS_TOPK_SORT use the JASS top_k_sort() which is a custom median of three medians quick sort
	CPP_TOPK_SORT use the C++ std::partial_sort() method
	CPP_SORT do a full sort using C++ std::sort()
	AVX512_SORT use the AVX512 sort Sort512_uint64_t::Sort() on 64-bit integers
*/
//#define JASS_TOPK_SORT
//#define CPP_TOPK_SORT
//#define CPP_SORT
//#define AVX512_SORT

/*
	PRE_SIMD is used with the heap to make the cumulative sum code work without SIMD instructions
*/
//#define PRE_SIMD

/*
	SIMD_ADD_RSV_AFTER_CUMSUM uses AVX instructions to process the postings in set_rsv() after the d1 decoding has already been done
	the alternativce is to process each posting one at a time.
*/
//#define SIMD_ADD_RSV_AFTER_CUMSUM 1

/*
	SIMD_JASS uses the decompressor that calls add_rsv() directly (single pass) rather than decompressng then processing (in 2 passes)
*/
//#define SIMD_JASS 1

/*
	SIMD_JASS_GROUP_ADD_RSV uses the AVX512 version of the processing of the postings list in add_rsv(), the alternative
	is to extract each doc id and process them one at a time
*/
//#define SIMD_JASS_GROUP_ADD_RSV 1

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

#include <limits>

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
//			typedef uint8_t ACCUMULATOR_TYPE;									///< the type of an accumulator (probably a uint16_t)
			typedef uint32_t DOCID_TYPE;										///< the type of a document id (from a compressor)

		public:
			static constexpr size_t MAX_DOCUMENTS = 55000000;					///< the maximum number of documents an index can hold
			static constexpr size_t MAX_TOP_K = 1000;							///< the maximum top-k value
			static constexpr size_t MAX_RSV = (std::numeric_limits<ACCUMULATOR_TYPE>::max)();

		public:
			/*
				CLASS QUERY::DOCID_RSV_PAIR()
				-----------------------------
			*/
			/*!
				@brief Literally a <document_id, rsv> ordered pair.
			*/
			class docid_rsv_pair
				{
				public:
					size_t document_id;							///< The document identifier
					const std::string &primary_key;			///< The external identifier of the document (the primary key)
					ACCUMULATOR_TYPE rsv;						///< The rsv (Retrieval Status Value) relevance score

				public:
					/*
						QUERY::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
						---------------------------------------
					*/
					/*!
						@brief Constructor.
						@param document_id [in] The document Identifier.
						@param key [in] The external identifier of the document (the primary key).
						@param rsv [in] The rsv (Retrieval Status Value) relevance score.
					*/
					docid_rsv_pair(size_t document_id, const std::string &key, ACCUMULATOR_TYPE rsv) :
						document_id(document_id),
						primary_key(key),
						rsv(rsv)
						{
						/* Nothing */
						}
				};

		protected:
			__m512i impacts512;															///< The impact score to be added on a call to add_rsv()
			__m256i impacts256;															///< The impact score to be added on a call to add_rsv()
			ACCUMULATOR_TYPE impact;													///< The impact score to be added on a call to add_rsv()
			DOCID_TYPE d1_cumulative_sum;												///<< The current cumulative sum from d1 decoding

			allocator_pool memory;														///< All memory allocation happens in this "arena"
			std::vector<__m512i> decompress_buffer;								///< The delta-encoded decopressed integer sequence.
			DOCID_TYPE documents;														///< The numnber of documents this index contains

			parser_query parser;															///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;											///< The parsed query
			const std::vector<std::string> *primary_keys;						///< A vector of strings, each the primary key for the document with an id equal to the vector index

		public:
			size_t top_k;																	///< The number of results to track.

		public:
			/*
				QUERY::QUERY()
				--------------
			*/
			/*!
				@brief Constructor
			*/
			query() :
#ifdef __AVX512F__
				impacts512(_mm512_setzero_si512()),
#endif
				impacts256(_mm256_setzero_si256()),
				impact(1),
				d1_cumulative_sum(0),
				documents(0),
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
				@param width [in] The width of the 2-d accumulators (if they are being used).
			*/
			virtual void init(const std::vector<std::string> &primary_keys, DOCID_TYPE documents = 1024, size_t top_k = 10, size_t width = 7)
				{
				this->primary_keys = &primary_keys;
				this->top_k = top_k;
				this->documents = documents;
				decompress_buffer.resize(64 + (documents * sizeof(DOCID_TYPE) + sizeof(decompress_buffer[0]) - 1) / sizeof(decompress_buffer[0]));			// we add 64 so that decompressors can overflow
				rewind(1, 1, 1);
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
			void parse(const STRING_TYPE &query, parser_query::parser_type which_parser = parser_query::parser_type::query)
				{
				parser.parse(*parsed_query, query, which_parser);
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
				@param smallest_possible_rsv [in] No rsv can be smaller than this (other than documents that are not found
				@param top_k_lower_bound [in] No rsv smaller than this can enter the top-k results list
				@param largest_possible_rsv [in] No rsv can be larger than this (but need no one need be this large)
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE top_k_lower_bound = 0, ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				delete parsed_query;
				parsed_query = new query_term_list;
				d1_cumulative_sum = 0;
				impact = 0;
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
				impact = score;
#ifdef SIMD_JASS
	#ifdef __AVX512F__
				impacts512 = _mm512_set1_epi32(impact);
	#else
				impacts256 = _mm256_set1_epi32(impact);
	#endif
#endif
				}

			/*
				QUERY::INIT_ADD_RSV()
				---------------------
			*/
			/*!
				@brief Set the d1_cumulative_sum to zero
			*/
			forceinline void init_add_rsv()
				{
				d1_cumulative_sum = 0;
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
				init_add_rsv();
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
