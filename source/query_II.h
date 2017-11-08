
#pragma once

#include "heap.h"
#include "top_k_qsort.h"
#include "parser_query.h"
#include "query_term_list.h"
#include "allocator_memory.h"

namespace JASS
	{
	template <typename ACCUMULATOR_TYPE, size_t MAX_DOCUMENTS, size_t MAX_TOP_K>
	class query_II
		{
		public:
			struct add_rsv_compare
				{
				forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const { return *a > *b ? 1 : *a < *b ? -1 : a - b; }
				};

		public:
			class iterator
				{
				class docid_rsv_pair
					{
					public:
						size_t document_id;							///< The document identifier
						const std::string &primary_key;			///< The external identifier of the document (the primary key)
						ACCUMULATOR_TYPE rsv;						///< The rsv (Retrieval Status Value) relevance score

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
					const query_II<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> *parent;
					size_t where;

				public:
					iterator(const query_II<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> *parent, size_t where) :
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


		private:
			allocator_pool memory;									///< All memory allocation happens in this "arena"

			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];
			size_t accumulators_shift;
			size_t accumulators_width;
			size_t accumulators_height;
			ACCUMULATOR_TYPE accumulators[MAX_DOCUMENTS];
			uint8_t clean_flags[MAX_DOCUMENTS];
			size_t results_list_length;

			heap<ACCUMULATOR_TYPE *, add_rsv_compare> top_results;

			parser_query parser;										///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;							///< The parsed query
			const std::vector<std::string> &primary_keys;	///< A vector of strings, each the primary key for the document with an id equal to the vector index
			size_t top_k;												///< The number of results to track.

			add_rsv_compare cmp;

		public:
			query_II(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10) :
				accumulators_shift(log2(sqrt((double)documents))),
  				accumulators_width(1 << accumulators_shift),
				accumulators_height((documents + accumulators_width) / accumulators_width),
				top_results(*accumulator_pointers, top_k),
				parser(memory),
				parsed_query(new query_term_list(memory)),
				primary_keys(primary_keys),
				top_k(top_k)
				{
				memset(clean_flags, 0, accumulators_height);
				}


			void sort(void)
				{
				top_k_qsort(accumulator_pointers, results_list_length, top_k);
				}


			auto begin(void)
				{
				sort();
				return iterator(this, 0);
				}

			auto end(void)
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

			void rewind(void)
				{
				results_list_length = 0;
		      memset(clean_flags, 0, accumulators_height);
		      delete parsed_query;
				parsed_query = new query_term_list(memory);
				}

			forceinline void add_rsv(size_t docid, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE old_value;
				ACCUMULATOR_TYPE *which = accumulators + docid;

				/*
					Make sure the accumulator exists
				*/
				if (clean_flags[docid >> accumulators_shift] == 0)
					{
					clean_flags[docid >> accumulators_shift] = 1;
					memset(accumulators + (accumulators_width * (docid >> accumulators_shift)), 0, accumulators_width * sizeof(ACCUMULATOR_TYPE));
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
						top_results.make_heap();
					}
				else if (cmp(which, accumulator_pointers[0]) >= 0)
					{
					/*
						We were already in the heap, so update
					*/
					*which += score;
					top_results.promote(which);
					}
				else
					{
					/*
						We weren't in the heap, but we could get put there
					*/
					*which += score;
					if (cmp(which, accumulator_pointers[0]) > 0)
						top_results.push_back(which);
					}
				}
		};
	}
