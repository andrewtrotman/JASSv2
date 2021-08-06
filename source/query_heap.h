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

#include "beap.h"
#include "heap.h"
#include "simd.h"
#include "query.h"
#include "pointer_box.h"
#include "accumulator_2d.h"
#include "sort512_uint64_t.h"
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
		private:
			typedef pointer_box<ACCUMULATOR_TYPE> accumulator_pointer;

			/*
				CLASS QUERY_HEAP::ITERATOR
				--------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				public:
					query_heap &parent;	///< The query object that this is iterating over
					int64_t where;			///< Where in the results list we are

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
					virtual iterator &operator++(void)
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
#ifdef ACCUMULATOR_64s
							DOCID_TYPE id = parent.sorted_accumulators[where] & 0xFFFF'FFFF;
							ACCUMULATOR_TYPE rsv = parent.sorted_accumulators[where] >> 32;
							return docid_rsv_pair(id, (*parent.primary_keys)[id], rsv);
#else
							size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where].pointer());
							return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators.get_value(id));
#endif
						}
					};

			/*
				CLASS QUERY_HEAP::REVERSE_ITERATOR
				----------------------------------
			*/
			/*!
				@brief Reverse iterate over the top-k
			*/
			class reverse_iterator : public iterator
				{
				public:
					using iterator::iterator;

					/*
						QUERY_HEAP::REVERSE_ITERATOR::OPERATOR++()
						------------------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					virtual iterator &operator++(void)
						{
						where--;
						return *this;
						}
				};

		private:
		
#ifdef ACCUMULATOR_STRATEGY_2D
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
#elif defined(ACCUMULATOR_COUNTER_8)
			accumulator_counter<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 8> accumulators;	///< The accumulators, one per document in the collection
#elif defined(ACCUMULATOR_COUNTER_4)
			accumulator_counter<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 4> accumulators;	///< The accumulators, one per document in the collection
#elif defined(ACCUMULATOR_COUNTER_INTERLEAVED_8)
			accumulator_counter_interleaved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 8> accumulators;	///< The accumulators, one per document in the collection
#elif defined(ACCUMULATOR_COUNTER_INTERLEAVED_8_1)
			accumulator_counter_interleaved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 8, 1> accumulators;	///< The accumulators, one per document in the collection
#elif defined(ACCUMULATOR_COUNTER_INTERLEAVED_4)
			accumulator_counter_interleaved<ACCUMULATOR_TYPE, MAX_DOCUMENTS, 4> accumulators;	///< The accumulators, one per document in the collection
#endif

			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k

#ifdef ACCUMULATOR_64s
			uint64_t sorted_accumulators[MAX_TOP_K];							///< high 32-bits is the rsv, the low 32-bits is the DocID.
			beap<uint64_t> top_results;											///< Heap containing the top-k results
#else
			ACCUMULATOR_TYPE zero;																		///< Constant zero used for pointer dereferenced comparisons
			accumulator_pointer accumulator_pointers[MAX_TOP_K];									///< Array of pointers to the top k accumulators
	#ifdef ACCUMULATOR_POINTER_BEAP
			beap<accumulator_pointer> top_results;		///< Heap containing the top-k results
	#else
			heap<accumulator_pointer> top_results;		///< Heap containing the top-k results
	#endif
#endif

			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())
#ifdef SIMD_JASS_GROUP_ADD_RSV
	#ifdef __AVX512F__
			__m512i lowest_in_heap;														///< vector of the smallest values in the heap
	#else
			__m256i lowest_in_heap;														///< vector of the smallest values in the heap
	#endif
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
#ifdef ACCUMULATOR_64s
				top_results(sorted_accumulators, top_k)
#else
				zero(0),
				top_results(accumulator_pointers, top_k)
#endif
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
				@param width [in] The width of the 2-d accumulators (if they are being used).
			*/
			virtual void init(const std::vector<std::string> &primary_keys, DOCID_TYPE documents = 1024, size_t top_k = 10, size_t width = 7)
				{
				query::init(primary_keys, documents, top_k);
				accumulators.init(documents, width);
#ifdef ACCUMULATOR_64s
				top_results.set_top_k((int64_t)top_k);
#else
				top_results.set_top_k(top_k);
#endif
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
				return iterator(*this, top_k);
				}

			/*
				QUERY_HEAP::RBEGIN()
				--------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto rbegin(void)
				{
				sort();
				return reverse_iterator(*this, top_k - 1);
				}

			/*
				QUERY_HEAP::REND()
				------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto rend(void)
				{
				return reverse_iterator(*this, needed_for_top_k - 1);
				}

			/*
				QUERY_HEAP::REWIND()
				--------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE top_k_lower_bound = 0, ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				sorted = false;
#ifdef ACCUMULATOR_64s
				sorted_accumulators[0] = 0;
#else
				accumulator_pointers[0] = &zero;
#endif
				accumulators.rewind();
				needed_for_top_k = this->top_k;
				query::rewind(largest_possible_rsv);
#ifdef SIMD_JASS_GROUP_ADD_RSV
	#ifdef __AVX512F__
				lowest_in_heap = _mm512_setzero_si512();
	#else
				lowest_in_heap = _mm256_setzero_si256();
	#endif
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
#ifdef ACCUMULATOR_64s
	#ifdef JASS_TOPK_SORT
					// CHECKED
					top_k_qsort::sort(sorted_accumulators + needed_for_top_k, top_k - needed_for_top_k, top_k);
	#elif defined(CPP_TOPK_SORT)
					// CHECKED
					std::partial_sort(sorted_accumulators + needed_for_top_k,  sorted_accumulators + top_k, sorted_accumulators + top_k);
	#elif defined(CPP_SORT)
					// CHECKED
					std::sort(sorted_accumulators + needed_for_top_k, sorted_accumulators + top_k);
	#elif defined(AVX512_SORT)
					// CHECKED
					Sort512_uint64_t::Sort(sorted_accumulators + needed_for_top_k, top_k - needed_for_top_k);
	#endif
#else
	#ifdef JASS_TOPK_SORT
					// CHECKED
					top_k_qsort::sort(accumulator_pointers + needed_for_top_k, top_k - needed_for_top_k, top_k);
	#elif defined(CPP_TOPK_SORT)
					// CHECKED
					std::partial_sort(accumulator_pointers + needed_for_top_k, accumulator_pointers + top_k, accumulator_pointers + top_k);
	#elif defined(CPP_SORT)
					// CHECKED
					std::sort(accumulator_pointers + needed_for_top_k, accumulator_pointers + top_k);
	#elif defined(AVX512_SORT)
					// CHECKED
					assert(false);
	#endif
#endif
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
#ifdef ACCUMULATOR_64s
				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.

				/*
					By doing the add first its possible to reduce the "usual" path through the code to a single comparison.  The JASS v1 "usual" path took three comparisons.
				*/
				*which += score;

				uint64_t key = ((uint64_t)*which << (uint64_t)32) | document_id;

				if (key >= sorted_accumulators[0])			// ==0 is the case where we're the current bottom of heap so might need to be promoted
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
							sorted_accumulators[--needed_for_top_k] = key;
							if (needed_for_top_k == 0)
								top_results.make_beap();
							}
						else
							{
							/*
								here we do a "replace under" - who suports that?  We do a linear search.  In the pointer verison its a free operation!
								If we were to sort first then it'd be N log N to sort and then log N to find, so O(N log N + log N), or O((N + 1) log N), which is O(N log N)
							*/
							uint64_t prior_key = ((uint64_t)(*which - score) << (uint64_t)32) | document_id;
							for (uint64_t *check = sorted_accumulators + needed_for_top_k; check < sorted_accumulators + top_k; check++)
								if (*check == prior_key)
									{
									*check = key;
									break;			// each instance is unique so we only need to do this once.
									}
							}
						}
					else
						{
						uint64_t prior_key = ((uint64_t)(*which - score) << (uint64_t)32) | document_id;

						if (prior_key < sorted_accumulators[0])
							top_results.replace_smallest_with(key);															// we're not in the heap so replace top of heap with this document
						else
							top_results.guaranteed_replace_with_larger(prior_key, key);									// we're already in the heap so promote this document
						}
					}
#else
				accumulator_pointer which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.

				/*
					By doing the add first its possible to reduce the "usual" path through the code to a single comparison.  The JASS v1 "usual" path took three comparisons.
				*/
				*which.pointer() += score;
				if (which >= accumulator_pointers[0])			// ==0 is the case where we're the current bottom of heap so might need to be promoted
					{
					/*
						We end up in the top-k, now to work out why.  As this is a rare occurence, we've got a little bit of time on our hands
					*/
					if (needed_for_top_k > 0)
						{
						/*
							the heap isn't full yet - so change only happens if we're a new addition (i.e. the old value was a 0)
						*/
						if (*which.pointer() == score)
							{
							accumulator_pointers[--needed_for_top_k] = which;
							if (needed_for_top_k == 0)
								{
	#ifdef ACCUMULATOR_POINTER_BEAP
								top_results.make_beap();
	#else
								top_results.make_heap();
	#endif
								}
							}
						}
					else
						{
						*which.pointer() -= score;
/**/					if (which < accumulator_pointers[0])		// HERE: BEAP USES <=, HEAP USES <
							{
							*which.pointer() += score;					// we weren't in there before but we are now so replace element 0
	#ifdef ACCUMULATOR_POINTER_BEAP
							top_results.beap_down(which, 0);
	#else
							top_results.push_back(which);				// we're not in the heap so add this accumulator to the heap
	#endif
							}
						else
							{
							auto at = top_results.find(which);		// we're already in there so find us and reshuffle the beap.
							*which.pointer() += score;
	#ifdef ACCUMULATOR_POINTER_BEAP
							top_results.beap_down(which, at);
	#else
							top_results.promote(which, at);				// we're already in the heap so promote this document
	#endif
							}
						}
					}
#endif
				}

			/*
				QUERY_HEAP::ADD_RSV_D1()
				------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
			*/
			forceinline void add_rsv_d1(DOCID_TYPE document_id)
				{
				document_id += d1_cumulative_sum;
				d1_cumulative_sum = document_id;
				add_rsv(document_id, impact);
				}
				
#ifdef __AVX512F__
			/*
				QUERY_HEAP::ADD_RSV_D1()
				------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
			*/
			forceinline void add_rsv_d1(__m512i document_ids)
				{
				/*
					Compute the cumulative sum using SIMD (and add the previous cumulative sum)
				*/
				__m512i cumsum = _mm512_set1_epi32(d1_cumulative_sum);
				document_ids = simd::cumulative_sum(document_ids);
				document_ids = _mm512_add_epi32(document_ids, cumsum);

#ifdef SIMD_JASS_GROUP_ADD_RSV
				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm_extract_epi32(_mm512_extracti32x4_epi32(document_ids, 3), 3);

				/*
					Add to the accumulators
				*/
				__m512i values = accumulators[document_ids];			// set the dirty flags and gather() the rsv values
				values = _mm512_add_epi32(values, impacts512);			// add the impact scores

				/*
					Compare and turn that into a bit patern.. If a >= b then 0xFFFF else 0x0000
				*/
				__mmask16 cmp = _mm512_cmpge_epi32_mask(values, lowest_in_heap);

				/*
					Check that we got all zeros.
				*/
				if (cmp == 0)
					simd::scatter(&accumulators.accumulator[0], document_ids, values);		// write them back to the accumulators
				else
					{
					/*
						At lest one document has (probably) made the top-k
					*/
					__m128i quad_docs;

					quad_docs = _mm512_extracti32x4_epi32(document_ids, 0);
					if (cmp & 0x000F)
						{
						add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 3), impact);
						}
					else
						simd::scatter(&accumulators.accumulator[0], quad_docs, _mm512_extracti32x4_epi32(values, 0));

					quad_docs = _mm512_extracti32x4_epi32(document_ids, 1);
					if (cmp & 0x00F0)
						{
						add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 3), impact);
						}
					else
						simd::scatter(&accumulators.accumulator[0], quad_docs, _mm512_extracti32x4_epi32(values, 1));

					quad_docs = _mm512_extracti32x4_epi32(document_ids, 2);
					if (cmp & 0x0F00)
						{
						add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 3), impact);
						}
					else
						simd::scatter(&accumulators.accumulator[0], quad_docs, _mm512_extracti32x4_epi32(values, 2));

					quad_docs = _mm512_extracti32x4_epi32(document_ids, 3);
					if (cmp & 0xF000)
						{
						add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
						add_rsv(_mm_extract_epi32(quad_docs, 3), impact);
						}
					else
						simd::scatter(&accumulators.accumulator[0], quad_docs, _mm512_extracti32x4_epi32(values, 3));
					}
				/*
					The lowest value in the heap might have changed so update it.
				*/
				lowest_in_heap = _mm512_set1_epi32(*accumulator_pointers[0]);
#else
				__m128i quad_docs;
				quad_docs = _mm512_extracti32x4_epi32(document_ids, 0);
				add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 3), impact);

				quad_docs = _mm512_extracti32x4_epi32(document_ids, 1);
				add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 3), impact);

				quad_docs = _mm512_extracti32x4_epi32(document_ids, 2);
				add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 3), impact);

				quad_docs = _mm512_extracti32x4_epi32(document_ids, 3);
				add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
				add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
				d1_cumulative_sum = _mm_extract_epi32(quad_docs, 3);
				add_rsv(d1_cumulative_sum, impact);
#endif
				}
#else
			/*
				QUERY_HEAP::ADD_RSV_D1()
				------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
			*/
			forceinline void add_rsv_d1(__m256i document_ids)
				{
				/*
					Compute the cumulative sum using SIMD (and add the previous cumulative sum)
				*/
				document_ids = simd::cumulative_sum(document_ids);
				__m256i cumsum = _mm256_set1_epi32(d1_cumulative_sum);
				document_ids = _mm256_add_epi32(document_ids, cumsum);

#ifdef SIMD_JASS_GROUP_ADD_RSV
				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm256_extract_epi32(document_ids, 7);

				/*
					Add to the accumulators
				*/
				__m256i values = accumulators[document_ids];			// set the dirty flags and gather() the rsv values
				values = _mm256_add_epi32(values, impacts);			// add the impact scores

				/*
					Compare and turn that into a bit patern.. If a >= b then 0xFFFF else 0x0000
				*/
				__m256i cmp_greater = _mm256_cmpgt_epi32(values, lowest_in_heap);
				__m256i cmp_equal = _mm256_cmpeq_epi32(values, lowest_in_heap);
				__m256i cmp = _mm256_or_si256(cmp_greater, cmp_equal);

				/*
					Check that we got all zeros.
				*/
				if (_mm256_testz_si256(cmp, cmp))
					simd::scatter(&accumulators.accumulator[0], document_ids, values);		// write them back to the accumulators
				else
					{
					/*
						At lest one document has (probably) made the top-k
					*/
					__m128i quad_docs;

					quad_docs = _mm256_extracti128_si256(document_ids, 0);
					add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 3), impact);

					quad_docs = _mm256_extracti128_si256(document_ids, 1);
					add_rsv(_mm_extract_epi32(quad_docs, 0), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 1), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 2), impact);
					add_rsv(_mm_extract_epi32(quad_docs, 3), impact);

					/*
						The lowest value in the heap might have changed so update it.
					*/
					lowest_in_heap = _mm256_set1_epi32(*accumulator_pointers[0]);
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
#endif
			/*
				QUERY_HEAP::DECODE_WITH_WRITER()
				--------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			virtual void decode_with_writer(size_t integers, const void *compressed, size_t compressed_size)
				{
				DOCID_TYPE *buffer = reinterpret_cast<DOCID_TYPE *>(decompress_buffer.data());
				decode(buffer, integers, compressed, compressed_size);

#ifdef PRE_SIMD
				DOCID_TYPE id = 0;
				DOCID_TYPE *end = buffer + integers;
				for (auto *current = buffer; current < end; current++)
					{
					id += *current;
					add_rsv(id, impact);
					}
#else
				/*
					D1-decode inplace with SIMD instructions then process one at a time
				*/
				simd::cumulative_sum_256(buffer, integers);

				/*
					Process the d1-decoded postings list.
				*/
				DOCID_TYPE *end;
				DOCID_TYPE *current = buffer;
				end = buffer + (integers & ~0x03);
				while (current < end)
					{
					add_rsv(*(current + 0), impact);
					add_rsv(*(current + 1), impact);
					add_rsv(*(current + 2), impact);
					add_rsv(*(current + 3), impact);
					current += 4;
					}
				end = buffer + integers;
				while (current < end)
					add_rsv(*current++, impact);
#endif
				}

			/*
				QUERY_HEAP::DECODE_WITH_WRITER()
				--------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@details Typically used to export an index, not used to process queries.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			template <typename WRITER>
			void decode_with_writer(WRITER &writer, size_t integers, const void *compressed, size_t compressed_size)
				{
				DOCID_TYPE *buffer = reinterpret_cast<DOCID_TYPE *>(decompress_buffer.data());
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
				JASS_assert(string.str() == "<1,15><3,20>");

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
