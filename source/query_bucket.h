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

#include "query.h"
#include "accumulator_2d.h"
#include "heap.h"

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
						size_t document_id;							///< The document identifier
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
						docid_rsv_pair(size_t document_id, const std::string &key, ACCUMULATOR_TYPE rsv) :
							document_id(document_id),
							primary_key(key),
							rsv(rsv)
							{
							/* Nothing */
							}
					};

				public:
					query_bucket &parent;	///< The query object that this is iterating over
					size_t where;																		///< Where in the results list we are

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
					iterator &operator++(void)
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
						size_t id = parent.accumulator_pointers[where] - parent.shadow_accumulator;
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.shadow_accumulator[id]);
						}
					};

		private:
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())

			static constexpr size_t rounded_top_k = ((size_t)1) << maths::ceiling_log2(MAX_TOP_K);
			static constexpr size_t rounded_top_k_filter = rounded_top_k - 1;
			static constexpr size_t number_of_buckets = std::numeric_limits<ACCUMULATOR_TYPE>::max();
			DOCID_TYPE bucket[number_of_buckets][rounded_top_k];	///< The array of buckets to use.
			ACCUMULATOR_TYPE largest_used_bucket;																///< The largest bucket used (to decrease cost of initialisation and search)
			uint16_t bucket_depth[number_of_buckets];				///< The number of documents in the given bucket
			ACCUMULATOR_TYPE shadow_accumulator[MAX_DOCUMENTS];											///< Used to deduplicate the top-k
			size_t accumulator_pointers_used;																	///< The number of accumulator_pointers used (can be smaller than top_k)

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
				largest_used_bucket(0)
				{
				std::fill(bucket_depth, bucket_depth + number_of_buckets, 0);
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
			*/
			virtual void init(const std::vector<std::string> &primary_keys, DOCID_TYPE documents = 1024, size_t top_k = 10)
				{
				query::init(primary_keys, documents, top_k);
				accumulators.init(documents);
				rewind(number_of_buckets);
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
				return iterator(*this, accumulator_pointers_used);
				}

			/*
				QUERY_BUCKET::REWIND()
				----------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
				@param largest_possible_rsv [in] the largest possible rsv value (or larger that that)
			*/
			virtual void rewind(ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				largest_used_bucket = largest_possible_rsv;
				sorted = false;
				accumulators.rewind();

				std::fill(bucket_depth, bucket_depth + largest_used_bucket, 0);

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
					/*
						Copy to the array of pointers for sorting (this will include duplicates)
					*/
					accumulator_pointers_used = 0;
					for (size_t current_bucket = largest_used_bucket; current_bucket > 0; current_bucket--)
						for (size_t which = 0; which < bucket_depth[current_bucket]; which++)
							{
							size_t doc_id = bucket[current_bucket][which];
							if (accumulators.accumulator[doc_id] != 0)		// only include those not already in the top-k
								{
								shadow_accumulator[doc_id] = accumulators.accumulator[doc_id];
								accumulator_pointers[accumulator_pointers_used] = &shadow_accumulator[doc_id];
								accumulators.accumulator[doc_id] = 0;		// mark it as already in the top-k

								accumulator_pointers_used++;

								if (accumulator_pointers_used >= this->top_k)
									goto got_them_all;
								}
							}
							
				got_them_all:
					/*
						Sort on the top-k
					*/
					top_k_qsort::sort(accumulator_pointers, accumulator_pointers_used, top_k, query::final_sort_cmp);
					sorted = true;
					}
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

				ACCUMULATOR_TYPE new_rsv = *which;

				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = document_id;
				bucket_depth[new_rsv]++;
				}

#ifdef SIMD_JASS
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

				ACCUMULATOR_TYPE new_rsv = *which;

				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = document_id;
				bucket_depth[new_rsv]++;
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
				document_ids = simd::cumulative_sum(document_ids);
				__m512i cumsum = _mm512_set1_epi32(d1_cumulative_sum);
				document_ids = _mm512_add_epi32(document_ids, cumsum);

				/*
					Save the cumulative sum
				*/
				d1_cumulative_sum = _mm_extract_epi32(_mm512_extracti32x4_epi32(document_ids, 3), 3);

				/*
					Add to the accumulators
				*/
				__m512i values = accumulators[document_ids];			// set the dirty flags and gather() the rsv values
				values = _mm512_add_epi32(values, impacts);			// add the impact scores

				/*
					Write back the accumulators
				*/
				simd::scatter(&accumulators.accumulator[0], document_ids, values);

				//
				// bucket[new_rsv * rounded_top_k + (bucket_depth[new_rsv] & rounded_top_k_filter)] = new_doc;
				// bucket_depth[new_rsv]++;
				//
				//
				// bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = document_id;
				// bucket_depth[new_rsv]++;
				//

				/*
					Compute the bucket indexes
				*/
				__m512i depths = simd::gather(bucket_depth, values);
				__m512i ander = _mm512_set1_epi32(rounded_top_k_filter);
				__m512i one = _mm512_set1_epi32(1);
				depths = _mm512_and_epi32(depths, ander);

				__m512i rows = _mm512_mullo_epi32(values, rounded_top_k);
				__m512i columns = _mm512_and_epi32(depths, ander);
				__m512i buckets = _mm512_add_epi32(row, columns);

				/*
					Check for conflicts
				*/
				__mmask16 unwritten = 0xFFFF;

				do
					{
					/*
						See if we have any conflicts, and turn into a mask for first occurences
					*/
					__m512i conflict = _mm512_maskz_conflict_epi32(unwritten, buckets);
					conflict = _mm512_and_epi32(_mm512_set1_epi32(unwritten), conflict);
					__mmask16 mask = _mm512_testn_epi32_mask(conflict, _mm512_set1_epi32(0xFFFF'FFFF));
					mask &= unwritten;

					/*
						read, merge, write the first occurences. First the DocIDs then the bucket positions
					*/
					_mm512_mask_i32scatter_epi32(bucket, mask, buckets, document_ids, 4);

					/*
						increment the column numbers
					*/
					__m512i columns = _mm512_and_epi32(_mm512_mask_add_epi32(depths, mask, depths, one), ander);
					__m512i buckets = _mm512_add_epi32(row, columns);

					/*
						ready for the next set
					*/
					unwritten ^= mask;
					}
				while (unwritten != 0);

				/*
					save the column numbers
				*/
				simd::scatter(bucket_depth, values, depths);


#ifdef NEVER
				/*
					Update the buckets
				*/
				__m128i quad_docs = _mm512_extracti32x4_epi32(document_ids, 0);
				__m128i quad_rsv = _mm512_extracti32x4_epi32(values, 0);

				uint32_t new_rsv = _mm_extract_epi32(quad_rsv, 0);
				uint32_t new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;



				quad_docs = _mm512_extracti32x4_epi32(document_ids, 1);
				quad_rsv = _mm512_extracti32x4_epi32(values, 1);

				new_rsv = _mm_extract_epi32(quad_rsv, 0);
				new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;


				quad_docs = _mm512_extracti32x4_epi32(document_ids, 2);
				quad_rsv = _mm512_extracti32x4_epi32(values, 2);

				new_rsv = _mm_extract_epi32(quad_rsv, 0);
				new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;



				quad_docs = _mm512_extracti32x4_epi32(document_ids, 3);
				quad_rsv = _mm512_extracti32x4_epi32(values, 3);

				new_rsv = _mm_extract_epi32(quad_rsv, 0);
				new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;
				}
#endif
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
					Write back the accumulators
				*/
				simd::scatter(&accumulators.accumulator[0], document_ids, values);

				/*
					Update the buckets
				*/
				__m128i quad_docs = _mm256_extracti128_si256(document_ids, 0);
				__m128i quad_rsv = _mm256_extracti128_si256(values, 0);

				uint32_t new_rsv = _mm_extract_epi32(quad_rsv, 0);
				uint32_t new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;



				quad_docs = _mm256_extracti128_si256(document_ids, 1);
				quad_rsv = _mm256_extracti128_si256(values, 1);

				new_rsv = _mm_extract_epi32(quad_rsv, 0);
				new_doc = _mm_extract_epi32(quad_docs, 0);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 1);
				new_doc = _mm_extract_epi32(quad_docs, 1);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 2);
				new_doc = _mm_extract_epi32(quad_docs, 2);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;

				new_rsv = _mm_extract_epi32(quad_rsv, 3);
				new_doc = _mm_extract_epi32(quad_docs, 3);
				bucket[new_rsv][bucket_depth[new_rsv] & rounded_top_k_filter] = new_doc;
				bucket_depth[new_rsv]++;
				}
#endif
#endif
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
					D1-decode inplace with SIMD instructions then process one at a time
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

				delete query_object;
				puts("query_bucket::PASSED");
				}
		};
	}
