/*
	QUERY_HEAP_CLEAN.H
	------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query using a heap to store the top-k. THIS CODE HAS BEEN STRIPPED OF ALL THEI IFDEFS
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
		CLASS QUERY_HEAP_CLEAN
		----------------------
	*/
	/*!
		@brief Everything necessary to process a query (using a heap) is encapsulated in an object of this type
	*/
	class query_heap_clean : public query
		{
		private:
			typedef pointer_box<ACCUMULATOR_TYPE> accumulator_pointer;

			/*
				CLASS QUERY_HEAP_CLEAN::ITERATOR
				--------------------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				public:
					query_heap_clean &parent;	///< The query object that this is iterating over
					int64_t where;			///< Where in the results list we are

				public:
					/*
						QUERY_HEAP_CLEAN::ITERATOR::ITERATOR()
						--------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query_heap_clean &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY_HEAP_CLEAN::ITERATOR::OPERATOR!=()
						----------------------------------------
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
						QUERY_HEAP_CLEAN::ITERATOR::OPERATOR++()
						----------------------------------------
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
						QUERY_HEAP_CLEAN::ITERATOR::OPERATOR*()
						---------------------------------------
					*/
					/*!
						@brief Return a reference to the <document_id,rsv> pair at the current location.
						@details This method uses ppointer arithmatic to work out the document id from a pointer to the rsv, and
						having done so it constructs an orderer pair <document_id,rsv> to return to the caller.
						@return The current object.
					*/
					docid_rsv_pair operator*()
						{
						size_t id = parent.accumulators.get_index(parent.accumulator_pointers[where].pointer());
						return docid_rsv_pair(id, (*parent.primary_keys)[id], parent.accumulators.get_value(id));
						}
					};

			/*
				CLASS QUERY_HEAP_CLEAN::REVERSE_ITERATOR
				----------------------------------------
			*/
			/*!
				@brief Reverse iterate over the top-k
			*/
			class reverse_iterator : public iterator
				{
				public:
					using iterator::iterator;

					/*
						QUERY_HEAP_CLEAN::REVERSE_ITERATOR::OPERATOR++()
						------------------------------------------------
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
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k
			ACCUMULATOR_TYPE zero;														///< Constant zero used for pointer dereferenced comparisons
			accumulator_pointer accumulator_pointers[MAX_TOP_K];				///< Array of pointers to the top k accumulators
			heap<accumulator_pointer> top_results;									///< Heap containing the top-k results
			bool sorted;																	///< has heap and accumulator_pointers been sorted (false after rewind() true after sort())
			ACCUMULATOR_TYPE top_k_lower_bound;										///< lowest possible score to enter the top k

		public:
			/*
				QUERY_HEAP_CLEAN::QUERY_HEAP_CLEAN()
				------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			query_heap_clean() :
				query(),
				zero(0),
				top_results(accumulator_pointers, top_k)
				{
				rewind();
				}

			/*
				QUERY_HEAP_CLEAN::~QUERY_HEAP_CLEAN()
				-------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~query_heap_clean()
				{
				}

			/*
				QUERY_HEAP_CLEAN::INIT()
				------------------------
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
				top_results.set_top_k(top_k);
				}

			/*
				QUERY_HEAP_CLEAN::BEGIN()
				-------------------------
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
				QUERY_HEAP_CLEAN::END()
				-----------------------
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
				QUERY_HEAP_CLEAN::RBEGIN()
				--------------------------
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
				QUERY_HEAP_CLEAN::REND()
				------------------------
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
				QUERY_HEAP_CLEAN::SIZE()
				------------------------
			*/
			query::DOCID_TYPE size(void)
				{
				return top_k - needed_for_top_k;
				}

			/*
				QUERY_HEAP_CLEAN::REWIND()
				--------------------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			virtual void rewind(ACCUMULATOR_TYPE smallest_possible_rsv = 0, ACCUMULATOR_TYPE top_k_lower_bound = 1, ACCUMULATOR_TYPE largest_possible_rsv = 0)
				{
				sorted = false;
				zero = 0;
				accumulator_pointers[0] = &zero;
				accumulators.rewind();
				needed_for_top_k = this->top_k;
				this->top_k_lower_bound = top_k_lower_bound;
				query::rewind(largest_possible_rsv);
				}

			/*
				QUERY_HEAP_CLEAN::SORT()
				------------------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				if (!sorted)
					{
					std::partial_sort(accumulator_pointers + needed_for_top_k, accumulator_pointers + top_k, accumulator_pointers + top_k);
					sorted = true;
					}
				}

			/*
				QUERY_HEAP_CLEAN::ADD_RSV()
				---------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			#define ADD_RSV(DOCUMENT_ID, SCORE) \
				{ \
				ACCUMULATOR_TYPE _score = SCORE;\
				accumulator_pointer which = &accumulators[(DOCUMENT_ID)];			/* This will create the accumulator if it doesn't already exist. */ \
				/*
					By doing the add first its possible to reduce the "usual" path through the code to a single comparison.  The JASS v1 "usual" path took three comparisons.
				*/ \
				*which.pointer() += _score; \
				if (*which.pointer() >= top_k_lower_bound) \
					{ \
					if (which >= accumulator_pointers[0])			/* == is the case where we're the current bottom of heap so might need to be promoted */ \
						{	\
						/*
							We end up in the top-k, now to work out why.  As this is a rare occurence, we've got a little bit of time on our hands
						*/ \
						if (needed_for_top_k > 0) \
							{ \
							/*
								the heap isn't full yet - so change only happens if we're a new addition (i.e. the old value was a 0)
							*/ \
							if (*which.pointer() - _score < top_k_lower_bound) \
								{ \
								accumulator_pointers[--needed_for_top_k] = which; \
								if (needed_for_top_k == 0) \
									{ \
									top_results.make_heap(); \
									if (top_k_lower_bound != 1) \
										break; /* We must be using the Oracle, and we must have filled the top-k and so we can stop processing this query. */ \
									top_k_lower_bound = *accumulator_pointers[0];		/* set the new bottom of heap value */ \
									} \
								} \
							} \
						else \
							{ \
							*which.pointer() -= _score; \
							if (which < accumulator_pointers[0]) \
								{ \
								*which.pointer() += _score;					/* we weren't in there before but we are now so replace element 0 */ \
								top_results.push_back(which);				/* we're not in the heap so add this accumulator to the heap */ \
								} \
							else \
								{ \
								auto at = top_results.find(which);		/* we're already in there so find us and reshuffle the beap. */ \
								*which.pointer() += _score; \
								top_results.promote(which, at);				/* we're already in the heap so promote this document */ \
								} \
							top_k_lower_bound = *accumulator_pointers[0];			/* set the new bottom of heap value */ \
							} \
						} \
					} \
				}

			/*
				QUERY_HEAP_CLEAN::ADD_RSV()
				---------------------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
			forceinline void add_rsv(DOCID_TYPE document_id, ACCUMULATOR_TYPE score) 
				{ 
				for (int x = 0; x < 1; x++)
					ADD_RSV(document_id, score);
				}

			/*
				QUERY_HEAP_CLEAN::ADD_RSV_D1()
				------------------------------
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

			/*
				QUERY_HEAP_CLEAN::DECODE_WITH_WRITER()
				--------------------------------------
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

				/*
					D1-decode inplace with SIMD instructions then process one at a time
				*/
				simd::cumulative_sum_256(buffer, integers);

				/*
					Process the d1-decoded postings list.  We ask the compiler to unroll the loop as it
					appears to be as fast as manually unrolling it.
				*/
				const DOCID_TYPE *end = buffer + integers;
#if defined(__clang__)
				#pragma unroll 8
#elif defined(__GNUC__) || defined(__GNUG__)
				#pragma GCC unroll 8
#endif
				for (DOCID_TYPE *current = buffer; current < end; current++)
					ADD_RSV(*current, impact);
				}

			/*
				QUERY_HEAP_CLEAN::DECODE_WITH_WRITER()
				--------------------------------------
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
				QUERY_HEAP_CLEAN::TOP_UP()
				--------------------------
			*/
			/*!
				@brief Walk through the remnatns of the search and top-up the top-k
				@details If the Oracle prediction is to large then the top-k will not be full, but the accuulators will be correct.  This
				method will walk through the accumulators and add to the top-k
			*/
			void  top_up(void)
				{
				/*
					Set the actual lower bound on the top-k to the current lowest score in the heap
					knowing that it is probably a 0, and therefore any score greater than or equal to 1 is
					good enough to put into the heap.
				*/
				ACCUMULATOR_TYPE heap_entry_point = *accumulator_pointers[0] == 0 ? 1 : *accumulator_pointers[0];

				/*
					Any score greater than or equal to top_k_lower_bound is already in the heap
				*/
				for (size_t page = 0; page < accumulators.number_of_dirty_flags; page++)
					{
					/*
						If the row was used in this query then we want to add any documents that have
						an score higher then the bottom of the heap
					*/
					if (!accumulators.dirty_flag[page])
						{
						size_t from = page << accumulators.shift;
						size_t to = (page + 1) << accumulators.shift;
						for (; from < to; from++)
							{
							auto which = &accumulators.accumulator[from];
							if (*which >= heap_entry_point && *which < top_k_lower_bound)
								{
								/*
									Insert into the heap
								*/
								if (needed_for_top_k > 0)
									{
									/*
										The heap isn't full yet - so put this accumulator into the heap
									*/
									accumulator_pointers[--needed_for_top_k] = which;
									if (needed_for_top_k == 0)
										{
										top_results.make_heap();							// We've filled the heap so build it
										heap_entry_point = *accumulator_pointers[0];		// set the new bottom of heap value
										}
									}
								else
									{
									top_results.push_back(which);							// we're not in the heap so add this accumulator to the heap
									heap_entry_point = *accumulator_pointers[0];			// set the new bottom of heap value
									}
								}
							}
						}
					}
				}

			/*
				QUERY_HEAP_CLEAN::UNITTEST()
				----------------------------
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
