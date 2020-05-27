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
					iterator &operator++(void)
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
						size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where]);
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators[id]);
						}
					};

		private:
			ACCUMULATOR_TYPE zero;														///< Constant zero used for pointer dereferenced comparisons
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k
			heap<ACCUMULATOR_TYPE *, typename query::add_rsv_compare> top_results;			///< Heap containing the top-k results
			ACCUMULATOR_TYPE page_maximum[accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS>::maximum_number_of_clean_flags];		///< The current maximum value of the accumulator block
			ACCUMULATOR_TYPE *page_maximum_pointers[accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS>::maximum_number_of_clean_flags];		///< Poointers to the current maximum value of the accumulator block
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
				zero(0),
				top_results(*accumulator_pointers, 0)
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
				QUERY_MAXBLOCK::INIT()
				----------------------
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
				top_results.set_top_k(top_k);
				for (size_t which = 0; which < accumulators.number_of_clean_flags; which++)
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
				return iterator(*this, this->top_k);
				}

			/*
				QUERY_MAXBLOCK_HEAP::REWIND()
				-----------------------------
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
				std::fill(page_maximum, page_maximum + accumulators.number_of_clean_flags, 0);
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
					std::sort(page_maximum_pointers, page_maximum_pointers + accumulators.number_of_clean_flags,
						[](const ACCUMULATOR_TYPE *a, const ACCUMULATOR_TYPE *b) -> bool
						{
						return *a > *b ? true : *a < *b ? false : a < b;
						});

//for (size_t page = 0; page < accumulators.number_of_clean_flags; page++)
//	std::cout << "ID:" << page_maximum_pointers[page] - page_maximum << " Value:" << *(page_maximum_pointers[page]) << " P \n";
//for (size_t page = 0; page < accumulators.number_of_clean_flags; page++)
//	std::cout << "ID:" << page << " Value:" << page_maximum[page] << " V \n";

					/*
						Walk through the pages looking for the case where an accumulator in the page should appear in the heap
					*/
					for (size_t page = 0; page < accumulators.number_of_clean_flags; page++)
						{
						if (*page_maximum_pointers[page] != 0 && *page_maximum_pointers[page] >= *accumulator_pointers[0])
							{
							ACCUMULATOR_TYPE *start = &accumulators.accumulator[(page_maximum_pointers[page] - page_maximum) * accumulators.width];
							for (ACCUMULATOR_TYPE *which = start; which < start + accumulators.width; which++)
								if (*which > 0 && this->cmp(which, accumulator_pointers[0]) > 0)			// == 0 is the case where we're the current bottom of heap so might need to be promoted
									{
									if (needed_for_top_k > 0)
										{
										accumulator_pointers[--needed_for_top_k] = which;
										if (needed_for_top_k == 0)
											top_results.make_heap();
										}
									else
										top_results.push_back(which);				// we're not in the heap so add this accumulator to the heap
									}
								}
							else
								break;
							}

					/*
						We now sort the array over which the heap is built so that we have a sorted list of docids from highest to lowest rsv.
					*/
					top_k_qsort::sort(accumulator_pointers + needed_for_top_k, top_k - needed_for_top_k, top_k, query::final_sort_cmp);

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
				size_t page = accumulators.which_clean_flag(document_id);		// get the page number
				ACCUMULATOR_TYPE *which = &accumulators[document_id];				// This will create the accumulator if it doesn't already exist.

				*which += score;

				page_maximum[page] = maths::maximum(page_maximum[page], *which);
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
				JASS_assert(string.str() == "<3,20><1,15>");

				delete query_object;
				puts("query_maxblock_heap::PASSED");
				}
		};
	}
