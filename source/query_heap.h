//#define SIMD_JASS_GROUP_ADD_RSV 1
#define SIMD_JASS 1

/*
	QUERY_HEAP.H
	------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query using a heap to store the top-k
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "simd.h"
#include "heap.h"
#include "query.h"
#include "accumulator_2d.h"
#include "accumulator_counter.h"
#include "accumulator_counter_interleaved.h"

namespace JASS
	{
	/*
		CLASS QUERY_HEAP
		----------------
	*/
	/*!
		@brief Everything necessary to process a query (using a heap) is encapsulated in an object of this type
	*/
	class query_heap : public query
		{
		public:
			/*
				CLASS QUERY_HEAP::ITERATOR
				--------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				/*
					CLASS QUERY_HEAP::ITERATOR::DOCID_RSV_PAIR()
					--------------------------------------------
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
							QUERY_HEAP::ITERATOR::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
							------------------------------------------------------
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

				public:
					query_heap &parent;	///< The query object that this is iterating over
					size_t where;			///< Where in the results list we are

				public:
					/*
						QUERY_HEAP::ITERATOR::ITERATOR()
						--------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query_heap &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY_HEAP::ITERATOR::OPERATOR!=()
						----------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param with [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/
					bool operator!=(const iterator &with) const
						{
						return with.where != where;
						}

					/*
						QUERY_HEAP::ITERATOR::OPERATOR++()
						----------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					iterator &operator++(void)
						{
						where++;
						return *this;
						}

					/*
						QUERY_HEAP::ITERATOR::OPERATOR*()
						---------------------------------
					*/
					/*!
						@brief Return a reference to the <document_id,rsv> pair at the current location.
						@details This method uses ppointer arithmatic to work out the document id from a pointer to the rsv, and
						having done so it constructs an orderer pair <document_id,rsv> to return to the caller.
						@return The current object.
					*/
					docid_rsv_pair operator*()
						{
						size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where]);

						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators[id]);
						}
					};

		private:
			ACCUMULATOR_TYPE zero;														///< Constant zero used for pointer dereferenced comparisons
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
//			accumulator_counter<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 8> accumulators;	///< The accumulators, one per document in the collection
//			accumulator_counter<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 4> accumulators;	///< The accumulators, one per document in the collection
//			accumulator_counter_interleaved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 8> accumulators;	///< The accumulators, one per document in the collection
//			accumulator_counter_interleaved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 4> accumulators;	///< The accumulators, one per document in the collection
			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k
			heap<ACCUMULATOR_TYPE *, typename query::add_rsv_compare> top_results;			///< Heap containing the top-k results
			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())
			DOCID_TYPE d1_cumulative_sum;
#ifdef SIMD_JASS_GROUP_ADD_RSV
			__m256i lowest_in_heap;														///< vector of the smallest values in the heap
#endif

		public:
			/*
				QUERY_HEAP::QUERY_HEAP()
				------------------------
			*/
			/*!
				@brief Constructor
			*/
			query_heap() :
				query(),
				zero(0),
				top_results(*accumulator_pointers, top_k)
				{
				rewind();
				}


			/*
				QUERY_HEAP::~QUERY_HEAP()
				-------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query_heap()
				{
				}

			/*
				QUERY_HEAP::INIT()
				------------------
			*/
			/*!
				@brief Initialise the object. MUST be called before first use.
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			virtual void init(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10)
				{
				query::init(primary_keys, documents, top_k);
				accumulators.init(documents);
				top_results.set_top_k(top_k);
				}

			/*
				QUERY_HEAP::BEGIN()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto begin(void)
				{
				sort();
				return iterator(*this, needed_for_top_k);
				}

			/*
				QUERY_HEAP::END()
				-----------------
			*/
			/*!
				@brief Return an iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto end(void)
				{
				return iterator(*this, this->top_k);
				}

			/*
				QUERY_HEAP::REWIND()
				--------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				sorted = false;
				accumulator_pointers[0] = &zero;
				accumulators.rewind();
				needed_for_top_k = this->top_k;
				query::rewind(largest_possible_rsv);
				d1_cumulative_sum = 0;
#ifdef SIMD_JASS_GROUP_ADD_RSV
				lowest_in_heap = _mm256_setzero_si256();
#endif
				}

			/*
				QUERY_HEAP::SORT()
				------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				if (!sorted)
					{
					top_k_qsort::sort(accumulator_pointers + needed_for_top_k, this->top_k - needed_for_top_k, this->top_k, query::final_sort_cmp);
					sorted = true;
					}
				}

			/*
				QUERY_HEAP::ADD_RSV()
				---------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(DOCID_TYPE document_id, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.

				/*
					By doing the add first its possible to reduce the "usual" path through the code to a single comparison.  The JASS v1 "usual" path took three comparisons.
				*/
				*which += score;
				if (this->cmp(which, accumulator_pointers[0]) >= 0)			// ==0 is the case where we're the current bottom of heap so might need to be promoted
					{
#ifdef SIMD_JASS
DOCID_TYPE SENTINAL = documents;
if (document_id >= SENTINAL)
	return;
#endif
					/*
						We end up in the top-k, now to work out why.  As this is a rare occurence, we've got a little bit of time on our hands
					*/
					if (needed_for_top_k > 0)
						{
						/*
							the heap isn't full yet - so change only happens if we're a new addition (i.e. the old value was a 0)
						*/
						if (*which == score)
							{
							accumulator_pointers[--needed_for_top_k] = which;
							if (needed_for_top_k == 0)
								top_results.make_heap();
							}
						}
					else
						{
						*which -= score;
						int prior_compare = this->cmp(which, accumulator_pointers[0]);
						*which += score;

						if (prior_compare < 0)
							top_results.push_back(which);				// we're not in the heap so add this accumulator to the heap
						else
							top_results.promote(which);				// we're already in the heap so promote this document
						}
					}
				}

			/*
				QUERY_HEAP::INIT_ADD_RSV()
				--------------------------
			*/
			/*!
				@brief Set the d1_cumulative_sum to zero
			*/
			void init_add_rsv()
				{
//std::cout << "init_add_rsv: d1_cumulative_sum = 0\n";
				d1_cumulative_sum = 0;
				}

			/*
				QUERY_HEAP::INSERT_INTO_HEAP()
				------------------------------
			*/
			/*!
				@brief Knowing that the docid most probably should enter the heap, add it if it should.
			*/
			void insert_into_heap(DOCID_TYPE document_id, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE *which = &accumulators.accumulator[document_id];

				if (this->cmp(which, accumulator_pointers[0]) >= 0)			// ==0 is the case where we're the current bottom of heap so might need to be promoted
					{
					/*
						We end up in the top-k, now to work out why.  As this is a rare occurence, we've got a little bit of time on our hands
					*/
					if (needed_for_top_k > 0)
						{
						/*
							the heap isn't full yet - so change only happens if we're a new addition (i.e. the old value was a 0)
						*/
						if (*which == score)
							{
							accumulator_pointers[--needed_for_top_k] = which;
							if (needed_for_top_k == 0)
								top_results.make_heap();
							}
						}
					else
						{
						*which -= score;
						int prior_compare = this->cmp(which, accumulator_pointers[0]);
						*which += score;

						if (prior_compare < 0)
							top_results.push_back(which);				// we're not in the heap so add this accumulator to the heap
						else
							top_results.promote(which);				// we're already in the heap so promote this document
						}
					}
				}


void dump(__m256i val, const char *message = "")
{
uint32_t *values = (uint32_t *)&val;

std::cout << message;
std::cout << values[0] << " ";
std::cout << values[1] << " ";
std::cout << values[2] << " ";
std::cout << values[3] << " ";
std::cout << values[4] << " ";
std::cout << values[5] << " ";
std::cout << values[6] << " ";
std::cout << values[7] << "\n";
}

			/*
				QUERY_HEAP::ADD_RSV()
				---------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(__m256i document_ids)
				{
//std::cout << "list cumulative sum at start of block: " << d1_cumulative_sum << "\n";
				/*
					Compute the cumulative sum using SIMD (and add the previous cumulative sum)
				*/
//dump(document_ids, "deltas:");
				document_ids = simd::cumulative_sum(document_ids);
//dump(document_ids, "lo sum:");
				__m256i cumsum = _mm256_set1_epi32(d1_cumulative_sum);
//dump(cumsum, "cumsum:");
				document_ids = _mm256_add_epi32(document_ids, cumsum);
//dump(document_ids, "docids:");

#ifdef SIMD_JASS_GROUP_ADD_RSV
				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm256_extract_epi32(document_ids, 7);
std::cout << "save cumulative sum: " << d1_cumulative_sum << "\n";

				/*
					Add to the accumulators
				*/
				__m256i values = accumulators[document_ids];			// set the dirty flags and gather() the rsv values
dump(values, "values:");
				values = _mm256_add_epi32(values, impacts);			// add the impact scores

dump(impacts, "impact:");
dump(values, "i+v   :");

				simd::scatter(&accumulators.accumulator[0], document_ids, values);		// write them back to the accumulators

				/*
					Compare and turn that into a bit patern.. If a >= b then 0xFFFF else 0x0000
				*/
				__m256i cmp_greater = _mm256_cmpgt_epi32(values, lowest_in_heap);
				__m256i cmp_equal = _mm256_cmpeq_epi32(values, lowest_in_heap);
				__m256i cmp = _mm256_or_si256(cmp_greater, cmp_equal);

std::cout << ".";

				/*
					Check that we got all zeros.  If not then we need to do an insert.
				*/
				if (!_mm256_testz_si256(cmp,cmp))
					{
					do
						{
						DOCID_TYPE SENTINAL = documents;

						DOCID_TYPE docid;
						ACCUMULATOR_TYPE score = _mm256_extract_epi32(impacts, 0);

						if ((docid = _mm256_extract_epi32(document_ids, 0)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 1)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 2)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 3)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 4)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 5)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 6)) >= SENTINAL) break; else insert_into_heap(docid, score);
						if ((docid = _mm256_extract_epi32(document_ids, 7)) >= SENTINAL) break; else insert_into_heap(docid, score);
						}
					while (false);

					lowest_in_heap = _mm256_set1_epi32(*accumulator_pointers[0]);
dump(lowest_in_heap, "loheap:");
					}
#else
				add_rsv(_mm256_extract_epi32(document_ids, 0), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 1), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 2), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 3), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 4), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 5), impact);
				add_rsv(_mm256_extract_epi32(document_ids, 6), impact);
				d1_cumulative_sum = _mm256_extract_epi32(document_ids, 7);
				add_rsv(d1_cumulative_sum, impact);
#endif
				}

			/*
				QUERY_HEAP::DECODE_WITH_WRITER()
				--------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			virtual void decode_with_writer(size_t integers, const void *compressed, size_t compressed_size)
				{
				auto buffer = decompress_buffer.data();
				decode(buffer, integers, compressed, compressed_size);

				/*
					D1-decode inplace with SIMD instructions the process one at a time
				*/
				simd::cumulative_sum(buffer, integers);
				DOCID_TYPE *end = buffer + integers;
				for (auto *current = buffer; current < end; current++)
					add_rsv(*current, impact);
				}

			/*
				QUERY_HEAP::DECODE_WITH_WRITER()
				--------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@details Typically used to export an index, not used to process queries.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			template <typename WRITER>
			void decode_with_writer(WRITER &writer, size_t integers, const void *compressed, size_t compressed_size)
				{
				auto buffer = decompress_buffer.data();
				decode(buffer, integers, compressed, compressed_size);

				DOCID_TYPE id = 0;
				DOCID_TYPE *end = buffer + integers;
				for (auto *current = buffer; current < end; current++)
					{
					id += *current;
					writer.add_rsv(id, impact);
					}
				}

			/*
				QUERY_HEAP::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				query_heap *query_object = new query_heap;
				query_object->init(keys, 1024, 2);
				std::ostringstream string;

				/*
					Check the rsv stuff
				*/
				query_object->add_rsv(2, 10);
				query_object->add_rsv(3, 20);
				query_object->add_rsv(2, 2);
				query_object->add_rsv(1, 1);
				query_object->add_rsv(1, 14);

				for (const auto rsv : *query_object)
					string << "<" << rsv.document_id << "," << rsv.rsv << ">";
				JASS_assert(string.str() == "<3,20><1,15>");

				/*
					Check the parser
				*/
				size_t times = 0;
				query_object->parse(std::string("one two three"));
				for (const auto &term : query_object->terms())
					{
					times++;
					if (times == 1)
						JASS_assert(term.token() == "one");
					else if (times == 2)
						JASS_assert(term.token() == "two");
					else if (times == 3)
						JASS_assert(term.token() == "three");
					}

				puts("query_heap::PASSED");
				}
		};
	}
