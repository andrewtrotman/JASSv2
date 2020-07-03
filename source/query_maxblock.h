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
					iterator &operator++(void)
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
						size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where]);
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators[id]);
						}
					};

		private:
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_DOCUMENTS];					///< Array of pointers to the top k accumulators
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;			///< The accumulators, one per document in the collection
			ACCUMULATOR_TYPE page_maximum[accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS>::maximum_number_of_dirty_flags];		///< The current maximum value of the accumulator block
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
			virtual void init(const std::vector<std::string> &primary_keys, DOCID_TYPE documents = 1024, size_t top_k = 10, size_t width = 7)
				{
				query::init(primary_keys, documents, top_k);
				accumulators.init(documents, width);
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
				QUERY_MAXBLOCK::REWIND()
				------------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				sorted = false;
				accumulators.rewind();
				non_zero_accumulators = 0;
//				std::fill(page_maximum, page_maximum + accumulators.number_of_dirty_flags, 0);
				query::rewind();
				}

			/*
				QUERY_MAXBLOCK::SORT()
				----------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				if (!sorted)
					{
					/*
						Walk through the pages looking for the case where an accumulator in the page might appear in the results list
					*/
					non_zero_accumulators = 0;
					for (size_t page = 0; page < accumulators.number_of_dirty_flags; page++)
						if (accumulators.dirty_flag[page] != 0xFF)
							{
							ACCUMULATOR_TYPE *start = &accumulators.accumulator[page * accumulators.width];
							for (ACCUMULATOR_TYPE *which = start; which < start + accumulators.width; which++)
								if (*which != 0)
									accumulator_pointers[non_zero_accumulators++] = which;
							}

					/*
						We now sort the array over which the heap is built so that we have a sorted list of docids from highest to lowest rsv.
					*/
					top_k_qsort::sort(accumulator_pointers, non_zero_accumulators, top_k, query::final_sort_cmp);
					non_zero_accumulators = maths::minimum(non_zero_accumulators, top_k);

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
				size_t page = accumulators.which_dirty_flag(document_id);		// get the page number
				if (accumulators.dirty_flag[page])
					page_maximum[page] = 0;
				ACCUMULATOR_TYPE *which = &accumulators[document_id];				// This will create the accumulator if it doesn't already exist.

				*which += score;

				page_maximum[page] = maths::maximum(page_maximum[page], *which);
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
				query_maxblock *query_object = new query_maxblock;
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
				JASS_assert(string.str() == "<3,20><1,15>");

				delete query_object;
				puts("query_maxblock::PASSED");
				}
		};
	}
