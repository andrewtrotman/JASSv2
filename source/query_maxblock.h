/*
	QUERY_MAXBLOCK.H
	----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query maxblock to store the accumulators
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "query.h"
#include "heap.h"

namespace JASS
	{
	/*
		CLASS QUERY_MAXBLOCK
		--------------------
	*/
	/*!
		@brief Everything necessary to process a query (using a maxblock) is encapsulated in an object of this type.  Thanks go to Antonio Mallia for inveting this method.
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
		@tparam MAX_DOCUMENTS The maximum number of documents that are ever going to exist in this collection
		@tparam MAX_TOP_K The maximum top-k documents that are going to be asked for
	*/
	class query_maxblock : public query
		{
		public:
			/*
				CLASS QUERY_MAXBLOCK::ITERATOR
				------------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				/*
					CLASS QUERY_MAXBLOCK::ITERATOR::DOCID_RSV_PAIR()
					------------------------------------------------
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
							QUERY_MAXBLOCK::ITERATOR::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
							----------------------------------------------------------
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
					query_maxblock &parent;	///< The query object that this is iterating over
					size_t where;				///< Where in the results list we are

				public:
					/*
						QUERY_MAXBLOCK::ITERATOR::ITERATOR()
						------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query_maxblock &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY_MAXBLOCK::ITERATOR::OPERATOR!=()
						--------------------------------------
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
						QUERY_MAXBLOCK::ITERATOR::OPERATOR++()
						--------------------------------------
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
						QUERY_MAXBLOCK::ITERATOR::OPERATOR*()
						-------------------------------------
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
						size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where]);
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators[id]);
#endif
						}
					};

				/*
					CLASS QUERY_MAXBLOCK::REVERSE_ITERATOR
					--------------------------------------
				*/
				/*!
					@brief Reverse iterate over the top-k
				*/
				class reverse_iterator : public iterator
					{
					public:
						using iterator::iterator;

						/*
							QUERY_MAXBLOCK::REVERSE_ITERATOR::OPERATOR++()
							----------------------------------------------
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

		protected:
#ifdef ACCUMULATOR_64s
			uint64_t sorted_accumulators[MAX_DOCUMENTS];									///< high word is the rsv, the low word is the DocID.
#else
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_DOCUMENTS];					///< Array of pointers to the top k accumulators
#endif

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
			size_t number_of_blocks;															///< The number of blocks
			ACCUMULATOR_TYPE page_maximum[MAX_DOCUMENTS];								///< The current maximum value of the accumulator block
			bool sorted;																			///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())
			size_t non_zero_accumulators;														///< The number of non-zero accumulators (should be top-k or less)

		public:
			/*
				QUERY_MAXBLOCK::QUERY_MAXBLOCK()
				--------------------------------
			*/
			/*!
				@brief Constructor
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			query_maxblock()
				{
				rewind();
				}

			/*
				QUERY_MAXBLOCK::~QUERY_MAXBLOCK()
				---------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query_maxblock()
				{
				}

			/*
				QUERY_MAXBLOCK::INIT()
				----------------------
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
				}

			/*
				QUERY_MAXBLOCK::BEGIN()
				-----------------------
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
				QUERY_MAXBLOCK::END()
				---------------------
			*/
			/*!
				@brief Return an iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto end(void)
				{
				return iterator(*this, non_zero_accumulators);
				}

			/*
				QUERY_MAXBLOCK::RBEGIN()
				------------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto rbegin(void)
				{
				sort();
				return reverse_iterator(*this, non_zero_accumulators - 1);
				}

			/*
				QUERY_MAXBLOCK::REND()
				----------------------
			*/
			/*!
				@brief Return a reverse iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto rend(void)
				{
				return reverse_iterator(*this, maths::maximum((int64_t)-1, (int64_t)(non_zero_accumulators - top_k - 1)));
				}

			/*
				QUERY_MAXBLOCK::REWIND()
				------------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE top_k_lower_bound = 0, ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				sorted = false;
				accumulators.rewind();
				non_zero_accumulators = 0;
#ifdef ACCUMULATOR_STRATEGY_2D
				/* Nothing */
#else
				std::fill(page_maximum, page_maximum + number_of_blocks, 0);
#endif
				query::rewind();
				}

			/*
				QUERY_MAXBLOCK::SORT()
				----------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			virtual void sort(void)
				{
				if (!sorted)
					{
#ifdef ACCUMULATOR_STRATEGY_2D
					/*
						Walk through all the pages looking for the case where an accumulator in the page might appear in the results list
					*/
					non_zero_accumulators = 0;
					for (size_t page = 0; page < number_of_blocks; page++)
						if (accumulators.dirty_flag[page] == 0)
							{
							ACCUMULATOR_TYPE *start = &accumulators.accumulator[page * accumulators.width];
							for (ACCUMULATOR_TYPE *which = start; which < start + accumulators.width; which++)
								{
								if (*which != 0)
									{
	#ifdef ACCUMULATOR_64s
									sorted_accumulators[non_zero_accumulators++] = ((uint64_t)*which << (uint64_t)32) | (which - &accumulators.accumulator[0]);
	#else
									accumulator_pointers[non_zero_accumulators++] = which;
	#endif
									}
								}
							}
#else
					/*
						Walk through all the pages looking for the case where an accumulator in the page might appear in the results list
					*/
					non_zero_accumulators = 0;
					for (size_t page = 0; page < number_of_blocks; page++)
						if (page_maximum[page] != 0)
							{
							for (size_t which = page * block_width; which < page * block_width + block_width; which++)
								{
								if (accumulators.get_value(which) != 0)
									{
	#ifdef ACCUMULATOR_64s
									sorted_accumulators[non_zero_accumulators++] = ((uint64_t)accumulators.get_value(which) << (uint64_t)32) | which;
	#else
									accumulator_pointers[non_zero_accumulators++] = &accumulators[which];
	#endif
									}
								}
							}
#endif

					/*
						We now sort the array over which the heap is built so that we have a sorted list of docids from highest to lowest rsv.
					*/
#ifdef ACCUMULATOR_64s
	#ifdef JASS_TOPK_SORT
					//CHECKED
					top_k_qsort::sort(sorted_accumulators, non_zero_accumulators, top_k);
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);
	#elif defined(CPP_TOPK_SORT)
					//CHECKED
					size_t sort_point = maths::minimum(non_zero_accumulators, top_k);
					std::partial_sort(sorted_accumulators, sorted_accumulators + sort_point, sorted_accumulators + non_zero_accumulators, std::greater<decltype(sorted_accumulators[0])>());
					non_zero_accumulators = sort_point;
	#elif defined(CPP_SORT)
					//CHECKED
					std::sort(sorted_accumulators, sorted_accumulators + non_zero_accumulators, std::greater<decltype(sorted_accumulators[0])>());
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);
	#elif defined(AVX512_SORT)
// NOT CHECKED
					Sort512_uint64_t::Sort(sorted_accumulators, non_zero_accumulators);
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);
	#endif
#else
	#ifdef JASS_TOPK_SORT
					//CHECKED
					top_k_qsort::sort(accumulator_pointers, non_zero_accumulators, top_k);
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);
	#elif defined(CPP_TOPK_SORT)
					//CHECKED
					size_t sort_point = maths::minimum(non_zero_accumulators, top_k);
					std::partial_sort(accumulator_pointers, accumulator_pointers + sort_point, accumulator_pointers + non_zero_accumulators,  [](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool { return *a > *b ? true : *a < *b ? false : a > b; });
					non_zero_accumulators = sort_point;
	#elif defined(CPP_SORT)
					//CHECKED
					std::sort(accumulator_pointers, accumulator_pointers + non_zero_accumulators,  [](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool { return *a > *b ? true : *a < *b ? false : a > b; });
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);
	#elif defined(AVX512_SORT)
					//CHECKED
					assert(false);
	#endif
#endif
					sorted = true;
					}
				}

			/*
				QUERY_MAXBLOCK::ADD_RSV()
				-------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(size_t document_id, ACCUMULATOR_TYPE score)
				{
#ifdef ACCUMULATOR_STRATEGY_2D
				size_t page = accumulators.which_dirty_flag(document_id);		// get the page number
				if (accumulators.dirty_flag[page] == 0)
					page_maximum[page] = 0;
#else
				size_t page = document_id >> bucket_shift;							// get the page number
#endif
				ACCUMULATOR_TYPE *which = &accumulators[document_id];				// This will create the accumulator if it doesn't already exist.

				*which += score;

				page_maximum[page] = maths::maximum(page_maximum[page], *which);
				}

			/*
				QUERY_MAXBLOCK::DECODE_WITH_WRITER()
				------------------------------------
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
				QUERY_MAXBLOCK::DECODE_WITH_WRITER()
				------------------------------------
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
				QUERY_MAXBLOCK::UNITTEST_THIS()
				-------------------------------
			*/
			/*!
				@brief Unit test an instance of this class
			*/
			static void unittest_this(query_maxblock *query_object)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				query_object->init(keys, 1024, 2);
				query_object->rewind();
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
				JASS_assert(string.str() == "<1,15><2,12>");
				}

			/*
				QUERY_MAXBLOCK::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				auto object = new query_maxblock;
				unittest_this(object);
				delete object;

				puts("query_maxblock::PASSED");
				}
		};
	}
