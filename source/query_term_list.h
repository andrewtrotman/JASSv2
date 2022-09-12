/*
	QUERY_TERM_LIST.H
	-----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A list of query terms (i.e. a query)
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <array>
#include <algorithm>

#include "query_term.h"
#include "dynamic_array.h"

namespace JASS
	{
	/*
		CLASS QUERY_TERM_LIST
		---------------------
	*/
	/*!
		@typedef query_term_list
		@brief A list of query tokens (i.e. a query).
	*/
	class query_term_list
		{
		friend std::ostream &operator<<(std::ostream &stream, const query_term_list &object);

		private:
			static const size_t max_query_terms = 0xFFF;			///< We allow up-to this numnber of (nont necessarily unique) terms in a query.

		private:
			size_t terms_in_query;								///< The numner of terms in this query.
			query_term terms[max_query_terms];				///< The quey terms themselves.

		public:
			/*
				QUERY_TERM_LIST::QUERY_TERM_LIST()
				----------------------------------
			*/
			/*!
				@brief Constructort
			*/
			query_term_list() :
				terms_in_query(0)
				{
				/* Nothing */
				}

			/*
				QUERY_TERM_LIST::BEGIN()
				------------------------
			*/
			/*!
				@brief Return the start of the iterator point
			*/
			auto begin(void) const
				{
				return &terms[0];
				}

			/*
				QUERY_TERM_LIST::BEGIN()
				------------------------
			*/
			/*!
				@brief Return the start of the iterator point
			*/
			auto begin(void)
				{
				return &terms[0];
				}

			/*
				QUERY_TERM_LIST::END()
				----------------------
			*/
			/*!
				@brief return the end of iterator point
			*/
			auto end(void) const
				{
				return &terms[terms_in_query];
				}

			/*
				QUERY_TERM_LIST::END()
				----------------------
			*/
			/*!
				@brief return the end of iterator point
			*/
			auto end(void)
				{
				return &terms[terms_in_query];
				}

			/*
				QUERY_TERM_LIST::PUSH_BACK()
				----------------------------
			*/
			/*!
				@brief Add a query term to the list.  This does not increase the term count if the term
				has been seen before - call sort_unique() to do that.
			*/
			void push_back(const slice &term)
				{
				if (terms_in_query < max_query_terms)
					{
					terms[terms_in_query].term = term;
					terms[terms_in_query].query_frequency = 1;
					terms_in_query++;
					}
				}

			/*
				QUERY_TERM_LIST::SORT_UNIQUE()
				------------------------------
			*/
			/*!
				@brief Sort the query terms then unique the list, incrementing the term count if duplicates are seen.  At the end
				the list has each term represented once and all the term counts represent the numbner of times the term has been
				seen in the original query.
			*/
			void sort_unique(void)
				{
				if (terms_in_query == 0)
					return;

				std::sort(begin(), end());

				size_t from = 1;
				size_t to = 0;

				while (from < terms_in_query)
					{
					if (terms[from].term == terms[to].term)
						terms[to].query_frequency++;
					else
						{
						to++;
						terms[to] = terms[from];
						}

					from++;
					}

				terms_in_query = to + 1;
				}

			/*
				QUERY_TERM_LIST::UNITTEST()
				---------------------------
			*/
			/*!
				@brief Test this class.
			*/
			static void unittest(void)
				{
				/*
					Empty list
				*/
				{
				query_term_list terms;

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "");
				}

				/*
					Single member list
				*/
				{
				query_term_list terms;

				terms.push_back("a");

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "(a,1)");
				}

				/*
					Single unique value list
				*/
				{
				query_term_list terms;

				terms.push_back("a");
				terms.push_back("a");
				terms.push_back("a");

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "(a,3)");
				}

				/*
					Multiple value list ending with a single element
				*/
				{
				query_term_list terms;

				terms.push_back("b");
				terms.push_back("b");
				terms.push_back("a");

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "(a,1)(b,2)");
				}

				/*
					Multiple value list ending with a duplicate element
				*/
				{
				query_term_list terms;

				terms.push_back("b");
				terms.push_back("b");
				terms.push_back("a");
				terms.push_back("a");

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "(a,2)(b,2)");
				}

				/*
					Multiple value list shuffled
				*/
				{
				query_term_list terms;

				terms.push_back("a");
				terms.push_back("b");
				terms.push_back("a");
				terms.push_back("b");

				terms.sort_unique();
				std::ostringstream into;
				into << terms;
				JASS_assert(into.str() == "(a,2)(b,2)");
				}
				
				puts("query_term_list::PASSED");
				}
		};

	/*
		OPERATOR<<()
		------------
	*/
	inline std::ostream &operator<<(std::ostream &stream, const query_term_list &object)
		{
		for (const auto &single : object)
			stream << single;

		return stream;
		}
	}
