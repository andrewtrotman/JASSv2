/*
	JASS_VOCABULARY.H
	-----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
 */
/*!
	@file
	@brief Describes the Compiled Indexes vocabulary structure
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
 */

/*
	CLASS JASS_CI_VOCAB
	-------------------
*/
/*!
	@brief The structure of the Compiled Indexes vocabulary.
*/
class JASS_ci_vocab
	{
	public:
		const char *term;							///< The search engine vocabulary term
		void (*method)(void);					///< The method to call when that term is seen in the query

	public:
		/*
			JASS_CI_VOCAB::OPERATOR<()
			--------------------------
		*/
		operator<(const JASS_ci_vocab &other)
			{
			return strcmp(term, other.term) < 0 ? true : false;
			}
	};

/*
	EXTERNS
	-------
*/
extern JASS_ci_vocab dictionary[];		///< The Compiled indexes vocabulary (which is, on startup, not assumed to be sorted, but is thereafter)
extern uint64_t dictionary_length;		///< The numnber of terms in the vocabulary
