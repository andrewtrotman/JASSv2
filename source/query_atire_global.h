
#pragma once

#include <stdint.h>

#include "heap.h"
#include "top_k_qsort.h"




namespace JASS
	{
	struct add_rsv_compare
		{
		__attribute__((always_inline)) inline int operator() (uint16_t *a, uint16_t *b) const { return *a > *b ? 1 : *a < *b ? -1 : (a > b ? 1 : a < b ? -1 : 0); }
		};

	allocator_pool memory;									///< All memory allocation happens in this "arena"

	uint8_t *clean_flags;
	uint16_t *accumulators;
	uint16_t **accumulator_pointers;
	uint32_t accumulators_shift;
	uint32_t accumulators_width;
	uint32_t accumulators_height;
	uint32_t results_list_length;

	ANT_heap<uint16_t *, add_rsv_compare> *heap;

	parser_query *parser;										///< Parser responsible for converting text into a parsed query
	query_term_list *parsed_query;							///< The parsed query
	const std::vector<std::string> *primary_keys;	///< A vector of strings, each the primary key for the document with an id equal to the vector index
	uint32_t top_k;												///< The number of results to track.
	size_t documents;


	template <typename ACCUMULATOR_TYPE>
	class query_atire_global
		{
		public:
			class iterator
				{
				class docid_rsv_pair
					{
					public:
						size_t document_id;					///< The document identifier
						const std::string &primary_key;			///< The external identifier of the document (the primary key)
						ACCUMULATOR_TYPE rsv;							///< The rsv (Retrieval Status Value) relevance score

					public:
						docid_rsv_pair(size_t document_id, const std::string &key, ACCUMULATOR_TYPE rsv) :
							document_id(document_id),
							primary_key(key),
							rsv(rsv)
							{
							/* Nothing */
							}
					};

				public:
					const query_atire_global<ACCUMULATOR_TYPE> *parent;
					size_t where;

				public:
					iterator(const query_atire_global<ACCUMULATOR_TYPE> *parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					bool operator!=(const iterator &with) const
						{
						return with.where != where;
						}
					iterator &operator++(void)
						{
						where++;
						return *this;
						}
					docid_rsv_pair operator*()
						{
						size_t id = parent->accumulator_pointers[where] - parent->accumulators;
						return docid_rsv_pair(id, parent->primary_keys[id], parent->accumulators[id]);
						}
					};

		public:
			query_atire_global(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10)
				{
				accumulators_shift = log2(sqrt((double)documents));
  				accumulators_width = 1 << accumulators_shift;
				accumulators_height = (documents + accumulators_width) / accumulators_width;
				accumulators = reinterpret_cast<ACCUMULATOR_TYPE *>(memory.malloc(sizeof(ACCUMULATOR_TYPE) * accumulators_width * accumulators_height));
				accumulator_pointers = reinterpret_cast<ACCUMULATOR_TYPE **>(memory.malloc(sizeof(ACCUMULATOR_TYPE *) * top_k));
				clean_flags = reinterpret_cast<uint8_t *>(memory.malloc(accumulators_height));
				heap = new ANT_heap<uint16_t *, add_rsv_compare>(*accumulator_pointers, top_k);
				parser = new parser_query(memory);
				parsed_query = nullptr;
				JASS::primary_keys =  &primary_keys;
				JASS::top_k = top_k;
				JASS::documents = documents;

				rewind();
				}

		void sort(void)
			{
			top_k_qsort(accumulator_pointers, results_list_length, top_k);
			}


		auto begin(void) const
			{
	      sort();
			return iterator(this, 0);
			}

		auto end(void) const
			{
			return iterator(this, results_list_length);
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
				parser->parse(*parsed_query, query);
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


			void rewind(void)
				{
				results_list_length = 0;
		      memset(clean_flags, 0, accumulators_height);
		      delete parsed_query;
				parsed_query = new query_term_list(memory);
				}

			__attribute__((always_inline)) inline static void add_rsv(uint32_t docid, uint16_t score)
				{
				uint16_t old_value;
				uint16_t *which = accumulators + docid;
				static add_rsv_compare cmp;

				/*
					Make sure the accumulator exists
				*/
				if (clean_flags[docid >> accumulators_shift] == 0)
					{
					clean_flags[docid >> accumulators_shift] = 1;
					memset(accumulators + (accumulators_width * (docid >> accumulators_shift)), 0, accumulators_width * sizeof(uint16_t));
					}

				/*
					Maintain a heap
				*/
				if (results_list_length < top_k)
					{
					/*
						We haven't got enough to worry about the heap yet, so just plonk it in
					*/
					old_value = *which;
					*which += score;

					if (old_value == 0)
						accumulator_pointers[results_list_length++] = which;

					if (results_list_length == top_k)
						heap->build_min_heap();
					}
				else if (cmp(which, accumulator_pointers[0]) >= 0)
					{
					/*
						We were already in the heap, so update
					*/
					*which +=score;
					heap->min_update(which);
					}
				else
					{
					/*
						We weren't in the heap, but we could get put there
					*/
					*which += score;
					if (cmp(which, accumulator_pointers[0]) > 0)
						heap->min_insert(which);
					}
				}
		};
	}
