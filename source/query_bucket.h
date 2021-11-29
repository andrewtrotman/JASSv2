/*
	QUERY_BUCKET.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query using a bucket sort to store the top-k
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "heap.h"
#include "query.h"
#include "accumulator_2d.h"
#include "sort512_uint64_t.h"
#include "accumulator_counter.h"
#include "accumulator_counter_interleaved.h"

namespace JASS
	{
	/*
		CLASS QUERY_BUCKET
		------------------
	*/
	/*!
		@brief Everything necessary to process a query (using a bucket sort) is encapsulated in an object of this type
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
		@tparam MAX_DOCUMENTS The maximum number of documents that are ever going to exist in this collection
		@tparam MAX_TOP_K The maximum top-k documents that are going to be asked for
	*/
	class query_bucket : public query
		{
		public:
			/*
				CLASS QUERY_BUCKET::ITERATOR
				----------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				/*
					CLASS QUERY_BUCKET::ITERATOR::DOCID_RSV_PAIR()
					----------------------------------------------
				*/
				/*!
					@brief Literally a <document_id, rsv> ordered pair.
				*/
				class docid_rsv_pair
					{
					public:
						DOCID_TYPE document_id;							///< The document identifier
						const std::string &primary_key;			///< The external identifier of the document (the primary key)
						ACCUMULATOR_TYPE rsv;						///< The rsv (Retrieval Status Value) relevance score

					public:
						/*
							QUERY_BUCKET::ITERATOR::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
							--------------------------------------------------------
						*/
						/*!
							@brief Constructor.
							@param document_id [in] The document Identifier.
							@param key [in] The external identifier of the document (the primary key).
							@param rsv [in] The rsv (Retrieval Status Value) relevance score.
						*/
						docid_rsv_pair(DOCID_TYPE document_id, const std::string &key, ACCUMULATOR_TYPE rsv) :
							document_id(document_id),
							primary_key(key),
							rsv(rsv)
							{
							/* Nothing */
							}
					};

				public:
					query_bucket &parent;	///< The query object that this is iterating over
					size_t where;				///< Where in the results list we are

				public:
					/*
						QUERY_BUCKET::ITERATOR::ITERATOR()
						----------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query_bucket &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY_BUCKET::ITERATOR::OPERATOR!=()
						------------------------------------
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
						QUERY_BUCKET::ITERATOR::OPERATOR++()
						------------------------------------
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
						QUERY_BUCKET::ITERATOR::OPERATOR*()
						-----------------------------------
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
						size_t id = parent.accumulator_pointers[where] - parent.shadow_accumulator;
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.shadow_accumulator[id]);
#endif
						}
					};

			/*
				CLASS QUERY_BUCKET::REVERSE_ITERATOR
				------------------------------------
			*/
			/*!
				@brief Reverse iterate over the top-k
			*/
			class reverse_iterator : public iterator
				{
				public:
					using iterator::iterator;

					/*
						QUERY_BUCKET::REVERSE_ITERATOR::OPERATOR++()
						--------------------------------------------
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
#ifdef ACCUMULATOR_64s
			uint64_t sorted_accumulators[MAX_TOP_K];		///< high word is the rsv, the low word is the DocID.
#else
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			ACCUMULATOR_TYPE shadow_accumulator[MAX_DOCUMENTS];				///< Used to deduplicate the top-k
#endif
			uint64_t accumulators_used;												///< The number of accumulator_pointers used (can be smaller than top_k)

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

			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())

//			static constexpr size_t rounded_top_k = ((size_t)1) << maths::ceiling_log2(MAX_TOP_K); ///< Sets the bucket depth based on top-k
			static constexpr size_t rounded_top_k = 256;															///< Sets the bucket depth to 256

			static constexpr size_t rounded_top_k_filter = rounded_top_k - 1;

			static constexpr size_t number_of_buckets = (std::numeric_limits<ACCUMULATOR_TYPE>::max)() > 0xFFFF ? 0xFFFF : (std::numeric_limits<ACCUMULATOR_TYPE>::max)();
			DOCID_TYPE bucket[number_of_buckets][rounded_top_k];						///< The array of buckets to use.
			ACCUMULATOR_TYPE largest_used_bucket;											///< The largest bucket used (to decrease cost of initialisation and search)
			ACCUMULATOR_TYPE smallest_used_bucket;											///< The smallest bucket used (to decrease cost of initialisation and search)
			uint8_t bucket_depth[number_of_buckets];										///< The number of documents in the given bucket

		public:
			/*
				QUERY_BUCKET::QUERY_BUCKET()
				----------------------------
			*/
			/*!
				@brief Constructor
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			query_bucket() :
				query(),
				largest_used_bucket(0),
				smallest_used_bucket((std::numeric_limits<ACCUMULATOR_TYPE>::max)())
				{
				std::fill(bucket_depth, bucket_depth + number_of_buckets, char());
//				memset(bucket_depth, 0, number_of_buckets * sizeof(bucket_depth[0]));

				rewind();
				}

			/*
				QUERY_BUCKET::~QUERY_BUCKET()
				-----------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query_bucket()
				{
				}

			/*
				QUERY_BUCKET::INIT()
				--------------------
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
				rewind(0, number_of_buckets);
				}

			/*
				QUERY_BUCKET::BEGIN()
				---------------------
			*/
			/*!
				@brief Return an iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto begin(void)
				{
				sort();
				return iterator(*this, 0);
				}

			/*
				QUERY_BUCKET::END()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto end(void)
				{
				return iterator(*this, accumulators_used);
				}

			/*
				QUERY_BUCKET::RBEGIN()
				----------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto rbegin(void)
				{
				sort();
				return reverse_iterator(*this, accumulators_used - 1);
				}

			/*
				QUERY_BUCKET::REND()
				--------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto rend(void)
				{
				return reverse_iterator(*this, -1);
				}

			/*
				QUERY_BUCKET::REWIND()
				----------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
				@param largest_possible_rsv [in] the largest possible rsv value (or larger that that)
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE top_k_lower_bound = 0,ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				smallest_used_bucket = smallest_possible_rsv;
				largest_used_bucket = largest_possible_rsv;
				sorted = false;
				accumulators.rewind();

				std::fill(bucket_depth + smallest_used_bucket, bucket_depth + largest_used_bucket + 1, char());
//				memset(bucket_depth + smallest_used_bucket, 0, (largest_used_bucket - smallest_used_bucket + 1) * sizeof(bucket_depth[0]));

				query::rewind();
				}

			/*
				QUERY_BUCKET::SORT()
				--------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				if (!sorted)
					{
#ifdef ACCUMULATOR_64s
					/*
						Turn into keys for sorting
					*/
					accumulators_used = 0;
					for (size_t current_bucket = largest_used_bucket; current_bucket >= smallest_used_bucket; current_bucket--)
						{
						size_t end_looking_at = maths::minimum((size_t)bucket_depth[current_bucket], (size_t)rounded_top_k);
						for (size_t which = 0; which < end_looking_at; which++)
							{
							uint64_t doc_id = bucket[current_bucket][which];
							uint64_t rsv = accumulators.get_value(doc_id);
							if (rsv != 0)		// only include those not already in the top-k
								{
								sorted_accumulators[accumulators_used] = (rsv << ((uint64_t)32)) | doc_id;
								accumulators[doc_id] = 0;		// mark it as already in the top-k

								accumulators_used++;

								if (accumulators_used >= top_k)
									goto got_them_all;
								}
							}
						}

				got_them_all:
					/*
						Sort on the top-k
					*/
	#ifdef JASS_TOPK_SORT
					// CHECKED
					top_k_qsort::sort(sorted_accumulators, accumulators_used, top_k);
	#elif defined(CPP_TOPK_SORT)
					// CHECKED
					std::partial_sort(sorted_accumulators, sorted_accumulators + (top_k > accumulators_used ? accumulators_used : top_k), sorted_accumulators + accumulators_used);
	#elif defined(CPP_SORT)
					// CHECKED
					std::sort(sorted_accumulators, sorted_accumulators + accumulators_used);
	#elif defined(AVX512_SORT)
// NOT CHECKED
					Sort512_uint64_t::Sort(sorted_accumulators, accumulators_used);
	#endif
#else
					/*
						Copy to the array of pointers for sorting
					*/
					accumulators_used = 0;
					for (size_t current_bucket = largest_used_bucket; current_bucket > smallest_used_bucket; current_bucket--)
						{
						size_t end_looking_at = maths::minimum((size_t)bucket_depth[current_bucket], (size_t)rounded_top_k);
						for (size_t which = 0; which < end_looking_at; which++)
							{
							size_t doc_id = bucket[current_bucket][which];
							auto rsv = accumulators.get_value(doc_id);
							if (rsv != 0)		// only include those not already in the top-k
								{
								shadow_accumulator[doc_id] = rsv;
								accumulator_pointers[accumulators_used] = &shadow_accumulator[doc_id];
								accumulators[doc_id] = 0;		// mark it as already in the top-k (as it can appear in multiple buckets)

								accumulators_used++;

								if (accumulators_used >= top_k)
									goto got_them_all;
								}
							}
						}

				got_them_all:
					/*
						Sort on the top-k
					*/

	#ifdef JASS_TOPK_SORT
					// CHECKED
					top_k_qsort::sort(accumulator_pointers, accumulators_used, top_k);
	#elif defined(CPP_TOPK_SORT)
					// CHECKED
					std::partial_sort(accumulator_pointers, accumulator_pointers + (top_k < accumulators_used ? top_k : accumulators_used), accumulator_pointers + accumulators_used, [](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool { return *a > *b ? true : *a < *b ? false : a > b; });
	#elif defined(CPP_SORT)
					// CHECKED
					std::sort(accumulator_pointers, accumulator_pointers + accumulators_used, [](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool { return *a > *b ? true : *a < *b ? false : a > b; });
	#elif defined(AVX512_SORT)
					// CHECKED
					assert(false);
	#endif
#endif
					sorted = true;
					}
				}

			/*
				QUERY_BUCKET::SET_BUCKET()
				--------------------------
			*/
			/*!
				@brief update the bucket for the given document
				@param document_id [in] which document to set
				@param score [in] the value to set
			*/
			forceinline void set_bucket(DOCID_TYPE document_id, ACCUMULATOR_TYPE score)
				{
				bucket[score][bucket_depth[score] & rounded_top_k_filter] = document_id;
				bucket_depth[score]++;
				}

			/*
				QUERY_BUCKET::SET_BUCKET()
				--------------------------
			*/
			/*!
				@brief update the bucket for the given document
				@param document_ids [in] which document to set
				@param values [in] which value to set
			*/
			forceinline void set_bucket(__m128i document_ids, __m128i values)
				{
				set_bucket(_mm_extract_epi32(document_ids, 0), _mm_extract_epi32(values, 0));
				set_bucket(_mm_extract_epi32(document_ids, 1), _mm_extract_epi32(values, 1));
				set_bucket(_mm_extract_epi32(document_ids, 2), _mm_extract_epi32(values, 2));
				set_bucket(_mm_extract_epi32(document_ids, 3), _mm_extract_epi32(values, 3));
				}

			/*
				QUERY_BUCKET::ADD_RSV()
				-----------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(DOCID_TYPE document_id, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.
				*which += score;
				set_bucket(document_id, *which);
				}

#ifdef ACCUMULATOR_STRATEGY_2D
			/*
				QUERY_BUCKET::ADD_RSV()
				-----------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
			*/
			forceinline void add_rsv(__m256i document_ids)
				{
				/*
					Add to the accumulators
				*/
				__m256i values = accumulators[document_ids];			// set the dirty flags and gather() the rsv values
				values = _mm256_add_epi32(values, impacts256);			// add the impact scores

				/*
					Write back the accumulators
				*/
				simd::scatter(&accumulators.accumulator[0], document_ids, values);

				/*
					Update the buckets
				*/
				set_bucket(_mm256_extracti128_si256(document_ids, 0), _mm256_extracti128_si256(values, 0));
				set_bucket(_mm256_extracti128_si256(document_ids, 1), _mm256_extracti128_si256(values, 1));
				}

#ifdef __AVX512F__
			/*
				QUERY_BUCKET::ADD_RSV()
				-----------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_ids [in] which document to increment
			*/
			forceinline void add_rsv(__m512i document_ids)
				{
				/*
					Add to the accumulators
				*/
				__m512i values = accumulators[document_ids];										// set the dirty flags and gather() the rsv values
				values = _mm512_add_epi32(values, impacts512);										// add the impact scores

				/*
					Write back the accumulators
				*/
				simd::scatter(&accumulators.accumulator[0], document_ids, values);

#ifdef SIMD_JASS_GROUP_ADD_RSV
				/*
					Retrieve the current bottom of bucket indexes
				*/
				__m512i depths = simd::gather(bucket_depth, values);

				/*
					Compute the new indexes for each document ID
				*/
				__m512i conflict = _mm512_conflict_epi32(depths);
				__m512i new_index = simd::popcount(conflict);
				depths = _mm512_add_epi32(depths, new_index);

				/*
					Place the DocIDs there.  We can scatter because no two writes are to the same location because the popcnt
					above increments the index into the buckets array by 1 for each conflicting address!
				*/
				__m512i columns = _mm512_and_epi32(depths, _mm512_set1_epi32(rounded_top_k_filter));
				__m512i rows = _mm512_mullo_epi32(values, _mm512_set1_epi32(rounded_top_k));
				__m512i buckets = _mm512_add_epi32(rows, columns);
				simd::scatter(&bucket[0][0], buckets, document_ids);

				/*
					We've added at least one to each bucket_depth so we account for that here.  Note that the writes must happen from
					LSB to MSB (according to the Intel spec) and so if we increment by more than 1 then the conflict() popcnt() above
					will have accumulated increments towards the MSB end and so the LSB writes can be ignored.
				*/
				__m512i new_depths = _mm512_add_epi32(depths, _mm512_set1_epi32(1));
				simd::scatter(bucket_depth, values, new_depths);
#else
				/*
					Update the buckets
				*/
				set_bucket(_mm512_extracti32x4_epi32(document_ids, 0), _mm512_extracti32x4_epi32(values, 0));
				set_bucket(_mm512_extracti32x4_epi32(document_ids, 1), _mm512_extracti32x4_epi32(values, 1));
				set_bucket(_mm512_extracti32x4_epi32(document_ids, 2), _mm512_extracti32x4_epi32(values, 2));
				set_bucket(_mm512_extracti32x4_epi32(document_ids, 3), _mm512_extracti32x4_epi32(values, 3));
#endif
				}
#endif
#endif

			/*
				QUERY_BUCKET::ADD_RSV_D1()
				--------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
			*/
			forceinline void add_rsv_d1(DOCID_TYPE document_id)
				{
				document_id += d1_cumulative_sum;
				d1_cumulative_sum = document_id;

				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.
				*which += impact;
				set_bucket(document_id, *which);
				}

#ifdef ACCUMULATOR_STRATEGY_2D

			/*
				QUERY_BUCKET::ADD_RSV_D1()
				--------------------------
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

				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm256_extract_epi32(document_ids, 7);

				/*
					Add to the RSVs and keep track of the top-k
				*/
				add_rsv(document_ids);
				}

#ifdef __AVX512F__
			/*
				QUERY_BUCKET::ADD_RSV_D1()
				--------------------------
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
				document_ids = simd::cumulative_sum(document_ids);
				__m512i cumsum = _mm512_set1_epi32(d1_cumulative_sum);
				document_ids = _mm512_add_epi32(document_ids, cumsum);

				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm_extract_epi32(_mm512_extracti32x4_epi32(document_ids, 3), 3);

				/*
					Add to the RSVs and keep track of the top-k
				*/
				add_rsv(document_ids);
				}
#endif
#endif

			/*
				QUERY_BUCKET::DECODE_WITH_WRITER()
				----------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			virtual void decode_with_writer(size_t integers, const void *compressed, size_t compressed_size)
				{
				DOCID_TYPE *buffer = reinterpret_cast<DOCID_TYPE *>(decompress_buffer.data());
				decode(buffer, integers, compressed, compressed_size);

				/*
					D1-decode inplace with SIMD instructions then process one at a time
				*/
				simd::cumulative_sum_256(buffer, integers);

				/*
					Process the d1-decoded postings list.
				*/
				DOCID_TYPE *end;
#ifdef SIMD_ADD_RSV_AFTER_CUMSUM
	#ifdef __AVX512F__
				end = buffer + (integers & ~0x0F);
				__m512i *chunk = (__m512i *)buffer;
				if (chunk < (__m512i *)end)
					{
					impacts512 = _mm512_set1_epi32(impact);
					do
						add_rsv(_mm512_loadu_si512(chunk));
					while (++chunk < (__m512i *)end);
					}
	#elif defined(__AVX2__)
				end = buffer + (integers & ~0x07);
				__m256i *chunk = (__m256i *)buffer;
				if (chunk < (__m256i *)end)
					{
					impacts256 = _mm256_set1_epi32(impact);
					do
						add_rsv(_mm256_loadu_si256(chunk));
					while (++chunk < (__m256i *)end);
					}
	#else
				DOCID_TYPE *chunk = buffer;
	#endif
#else
				DOCID_TYPE *chunk = buffer;
#endif
				DOCID_TYPE *current = (DOCID_TYPE *)chunk;
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
				}

			/*
				QUERY_BUCKET::DECODE_WITH_WRITER()
				----------------------------------
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
				QUERY_BUCKET::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				query_bucket *query_object = new query_bucket;
				query_object->init(keys, 1024, 2);
				query_object->rewind(0, 100, 100);
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

				delete query_object;
				puts("query_bucket::PASSED");
				}
		};
	}
