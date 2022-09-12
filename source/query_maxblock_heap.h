/*
	QUERY_MAXBLOCK_HEAP.H
	---------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query maxblock to store the accumulators, but using a heap to reduce block checking.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "query.h"
#include "heap.h"

namespace JASS
	{
	/*
		CLASS QUERY_MAXBLOCK_HEAP
		-------------------------
	*/
	/*!
		@brief Everything necessary to process a query (using a maxblock) is encapsulated in an object of this type.  Thanks go to Antonio Mallia for inveting this method.
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
		@tparam MAX_DOCUMENTS The maximum number of documents that are ever going to exist in this collection
		@tparam MAX_TOP_K The maximum top-k documents that are going to be asked for
	*/
	class query_maxblock_heap : public query
		{
		private:
			typedef pointer_box<ACCUMULATOR_TYPE> accumulator_pointer;

		public:
			/*
				CLASS QUERY_MAXBLOCK_HEAP::ITERATOR
				-----------------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				/*
					CLASS QUERY_MAXBLOCK_HEAP::ITERATOR::DOCID_RSV_PAIR()
					-----------------------------------------------------
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
							QUERY_MAXBLOCK_HEAP::ITERATOR::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
							---------------------------------------------------------------
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
					query_maxblock_heap &parent;	///< The query object that this is iterating over
					size_t where;																		///< Where in the results list we are

				public:
					/*
						QUERY_MAXBLOCK_HEAP::ITERATOR::ITERATOR()
						-----------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query_maxblock_heap &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY_MAXBLOCK_HEAP::ITERATOR::OPERATOR!=()
						-------------------------------------------
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
						QUERY_MAXBLOCK_HEAP::ITERATOR::OPERATOR++()
						-------------------------------------------
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
						QUERY_MAXBLOCK_HEAP::ITERATOR::OPERATOR*()
						------------------------------------------
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
					CLASS QUERY_MAXBLOCK_HEAP::REVERSE_ITERATOR
					-------------------------------------------
				*/
				/*!
					@brief Reverse iterate over the top-k
				*/
				class reverse_iterator : public iterator
					{
					public:
						using iterator::iterator;

						/*
							QUERY_MAXBLOCK_HEAP::REVERSE_ITERATOR::OPERATOR++()
							---------------------------------------------------
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
			size_t block_width;																	///< The number of documents per block
			size_t bucket_shift;																	///< The amount to shift to get the right bucket
			size_t number_of_blocks;													///< The number of blocks
			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k
#ifdef ACCUMULATOR_64s
			uint64_t sorted_accumulators[MAX_DOCUMENTS];									///< high word is the rsv, the low word is the DocID.
			heap<uint64_t> top_results;			///< Heap containing the top-k results
#else
			ACCUMULATOR_TYPE zero;															///< Constant zero used for pointer dereferenced comparisons
			accumulator_pointer accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			heap<accumulator_pointer> top_results;										///< Heap containing the top-k results
#endif
			ACCUMULATOR_TYPE page_maximum[accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS>::maximum_number_of_dirty_flags];		///< The current maximum value of the accumulator block
			ACCUMULATOR_TYPE *page_maximum_pointers[accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS>::maximum_number_of_dirty_flags];		///< Poointers to the current maximum value of the accumulator block
			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())

		public:
			/*
				QUERY_MAXBLOCK_HEAP::QUERY_MAXBLOCK_HEAP()
				------------------------------------------
			*/
			/*!
				@brief Constructor
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			query_maxblock_heap() :
				number_of_blocks(0),
#ifdef ACCUMULATOR_64s
				top_results(sorted_accumulators, 0)
#else
				zero(0),
				top_results(accumulator_pointers, 0)
#endif
				{
				rewind();
				}

			/*
				QUERY_MAXBLOCK_HEAP::~QUERY_MAXBLOCK_HEAP()
				-------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query_maxblock_heap()
				{
				}

			/*
				QUERY_MAXBLOCK_HEAP::INIT()
				---------------------------
			*/
			/*!
				@brief Initialise the object. MUST be called before first use.
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
				@param width [in] The width of the 2-d accumulators (if they are being used).
			*/
			virtual void init(const std::vector<std::string> &primary_keys, DOCID_TYPE documents = 1024, size_t top_k = 10, size_t preferred_width = 7)
				{
				query::init(primary_keys, documents, top_k);
				accumulators.init(documents, preferred_width);
				top_results.set_top_k(top_k);
#ifdef ACCUMULATOR_STRATEGY_2D
				number_of_blocks = accumulators.number_of_dirty_flags;
				block_width = accumulators.width;
				bucket_shift = accumulators.shift;
#else
				if (preferred_width >= 1)
					bucket_shift = preferred_width;
				else
					bucket_shift = maths::floor_log2((size_t)sqrt(documents));

				block_width = (size_t)1 << bucket_shift;
				number_of_blocks = (documents + block_width - 1) / block_width;

#endif
				for (size_t which = 0; which < number_of_blocks; which++)
					page_maximum_pointers[which] = &page_maximum[which];
				}

			/*
				QUERY_MAXBLOCK_HEAP::BEGIN()
				----------------------------
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
				QUERY_MAXBLOCK_HEAP::END()
				--------------------------
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
				QUERY_MAXBLOCK_HEAP::RBEGIN()
				-----------------------------
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
				QUERY_MAXBLOCK_HEAP::REND()
				---------------------------
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
				QUERY_MAXBLOCK_HEAP::REWIND()
				-----------------------------
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
				std::fill(page_maximum, page_maximum + number_of_blocks, 0);
				query::rewind();
				}

			/*
				QUERY_MAXBLOCK_HEAP::SORT()
				---------------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				if (!sorted)
					{
					/*
						Sort the page maximum values from highest to lowest.
					*/
					std::sort(page_maximum_pointers, page_maximum_pointers + number_of_blocks,
						[](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool
						{
						return *a > *b ? true : *a < *b ? false : a < b;
						});

//for (size_t page = 0; page < accumulators.number_of_dirty_flags; page++)
//	std::cout << "ID:" << page_maximum_pointers[page] - page_maximum << " Value:" << *(page_maximum_pointers[page]) << " P \n";
//for (size_t page = 0; page < accumulators.number_of_dirty_flags; page++)
//	std::cout << "ID:" << page << " Value:" << page_maximum[page] << " V \n";

					/*
						Walk through the pages looking for the case where an accumulator in the page should appear in the heap
					*/
#ifdef ACCUMULATOR_64s
					for (size_t page = 0; page < number_of_blocks; page++)
						{
//std::cout << "page:" << *page_maximum_pointers[page] << " heap[0]:" << (sorted_accumulators[0] >> 32) << "\n";
						if (*page_maximum_pointers[page] != 0 && *page_maximum_pointers[page] >= (sorted_accumulators[0] >> 32))
							{
							size_t start = (page_maximum_pointers[page] - page_maximum) * block_width;
							for (size_t which = start; which < start + block_width; which++)
								{
								uint64_t key = ((uint64_t)accumulators[which] << (uint64_t)32) | which;
								if (accumulators.get_value(which) > 0 && key > sorted_accumulators[0])			// == 0 is the case where we're the current bottom of heap so might need to be promoted
									{
									if (needed_for_top_k > 0)
										{
										sorted_accumulators[--needed_for_top_k] = key;
										if (needed_for_top_k == 0)
											top_results.make_heap();
										}
									else
										top_results.push_back(key);				// we're not in the heap so add this accumulator to the heap
									}
								}
							}
						else
							break;
						}
#else
					for (size_t page = 0; page < number_of_blocks; page++)
						{
						if (*page_maximum_pointers[page] != 0 && *page_maximum_pointers[page] >= *accumulator_pointers[0])
							{
							size_t start = (page_maximum_pointers[page] - page_maximum) * block_width;
							for (size_t which = start; which < start + block_width; which++)
								{
								if (accumulators.get_value(which) > 0)
									{
									if (accumulator_pointers[0] < &accumulators[which])
										{
										if (needed_for_top_k > 0)
											{
											accumulator_pointers[--needed_for_top_k] = &accumulators[which];
											if (needed_for_top_k == 0)
												top_results.make_heap();
											}
										else
											top_results.push_back(&accumulators[which]);				// we're not in the heap so add this accumulator to the heap
										}
									}
								}
							}
						else
							break;
						}
#endif

					/*
						We now sort the array over which the heap is built so that we have a sorted list of docids from highest to lowest rsv.
					*/
#ifdef ACCUMULATOR_64s
	#ifdef JASS_TOPK_SORT
					// CHECKED
					top_k_qsort::sort(sorted_accumulators + needed_for_top_k, top_k - needed_for_top_k, top_k);
	#elif defined(CPP_TOPK_SORT)
					// CHECKED
					std::partial_sort(sorted_accumulators + needed_for_top_k, sorted_accumulators + top_k, sorted_accumulators + top_k);
	#elif defined(CPP_SORT)
					// CHECKED
					std::sort(sorted_accumulators + needed_for_top_k, sorted_accumulators + top_k);
	#elif defined(AVX512_SORT)
// NOT CHECKED
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
				QUERY_MAXBLOCK_HEAP::ADD_RSV()
				------------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(size_t document_id, ACCUMULATOR_TYPE score)
				{
				size_t page = document_id >> bucket_shift;							// get the page number
				ACCUMULATOR_TYPE *which = &accumulators[document_id];				// This will create the accumulator if it doesn't already exist.

				*which += score;

				page_maximum[page] = maths::maximum(page_maximum[page], *which);
				}

			/*
				QUERY_MAXBLOCK_HEAP::DECODE_WITH_WRITER()
				-----------------------------------------
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
				QUERY_MAXBLOCK_HEAP::DECODE_WITH_WRITER()
				-----------------------------------------
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
				QUERY_MAXBLOCK_HEAP::UNITTEST()
				-------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				query_maxblock_heap *query_object = new query_maxblock_heap;
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

				delete query_object;
				puts("query_maxblock_heap::PASSED");
				}
		};
	}
