/*
	JASS_ANYTIME_INDEX.H
	--------------------
*/
#include "string.h"

#include <string>
#include <vector>

#include "slice.h"
#include "query_term.h"
#include "compress_integer.h"

/*
*/
class anytime_index
	{
	public:
		/*
		*/
		class metadata
			{
			public:
				JASS::slice term;						///< Pointer to a '\0' terminated string that is this terms
				uint8_t *offset;						///< Offset to the postings for this term
				uint64_t impacts;						///< The numner of impact segments this term has

			public:
				metadata() :
					term(),
					offset(nullptr),
					impacts(0)
					{
					/* Nothing */
					}

				metadata(const JASS::slice &term, const char *offset, uint64_t impacts) :
					term(term),
					offset(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(offset))),
					impacts(impacts)
					{
					/* Nothing */
					}

				bool operator<(const JASS::slice &with) const
					{
					return JASS::slice::strict_weak_order_less_than(term, with);
					}
			};

	private:
		bool verbose;											///< Should this class produce diagnostics on stdout?

		uint64_t documents;									///< The number of documents in the collection
		std::string primary_key_memory;					///< Memory used to store the primary key strings
		std::vector<std::string> primary_key_list;			///< The array of primary keys

		uint64_t terms;										///< The numner of terms in the collection
		std::string vocabulary_memory;					///< Memory used to store the vocabulary pointers
		std::string vocabulary_terms_memory;			///< Memory used to store the vocabulary strings
		std::vector<metadata> vocabulary_list;			///< The (sorted in alphabetical order) array of vocbulary terms

		std::string postings_memory;						///< Memory used to store the postings

	protected:
		/*
			ANYTIME_INDEX::READ_PRIMARY_KEYS()
			----------------------------------
		*/
		/*!
			@brief Read the JASS v1 index primary key file
			@param primary_key_filename [in] the name of the file containing the primary key list ("CIdoclist.bin")
			@return The number of documents in the collection (or 0 on error)
		*/
		size_t read_primary_keys(const std::string &primary_key_filename = "CIdoclist.bin");

		/*
			ANYTIME_INDEX::READ_VOCABULARY()
			--------------------------------
		*/
		/*!
			@brief Read the JASS v1 index vocabulary files
			@param vocab_filename [in] the name of the file containing the vocabulary pointers ("CIvocab.bin")
			@param terms_filename [in] the name of the file containing the vocabulary strings ("CIvocab_terms.bin")
			@return The number of documents in the collection (or 0 on error)
		*/
		size_t read_vocabulary(const std::string &vocab_filename = "CIvocab.bin", const std::string &terms_filename = "CIvocab_terms.bin");

		/*
			ANYTIME_INDEX::READ_POSTINGS()
			------------------------------
		*/
		/*!
			@brief Read the JASS v1 index postings file
			@param postings_filename [in] the name of the file containing the postings ("CIpostings.bin")
			@return size of the posings file or 0 on failure
		*/
		size_t read_postings(const std::string &postings_filename = "CIpostings.bin");

	public:
		/*
			ANYTIME_INDEX::ANYTIME_INDEX()
			------------------------------
		*/
		/*!
			@brief Constructor
			@param verbose [in] Should the index reading methods produce messages on stdout?
		*/
		anytime_index(bool verbose) :
			verbose(verbose),
			documents(0),
			terms(0)
			{
			/* Nothing */
			}

		/*
			ANYTIME_INDEX::READ_INDEX()
			---------------------------
		*/
		/*!
			@brief Read a JASS v1 index into memory
			@param primary_key_filename [in] the name of the file containing the primary key list ("CIdoclist.bin")
			@param vocab_filename [in] the name of the file containing the vocabulary pointers ("CIvocab.bin")
			@param terms_filename [in] the name of the file containing the vocabulary strings ("CIvocab_terms.bin")
			@param postings_filename [in] the name of the file containing the postings ("CIpostings.bin")
			@return 0 on failure, non-zero on success
		*/
		size_t read_index(const std::string &primary_key_filename = "CIdoclist.bin", const std::string &vocab_filename = "CIvocab.bin", const std::string &terms_filename = "CIvocab_terms.bin", const std::string &postings_filename = "CIpostings.bin");

		/*
			ANYTIME_INDEX::CODEX()
			----------------------
		*/
		/*!
			@brief Return a reference to a decompressor that can be used with this index
			@return A reference to a compress_integer that can decode the given codex
		*/
		JASS::compress_integer &codex(void);

		/*
			ANYTIME_INDEX::PRIMARY_KEYS()
			-----------------------------
		*/
		/*!
			@brief Return the list of primary keys as a std::vector<std::string>
			@return A reference to a vector of primary keys
		*/
		const std::vector<std::string> &primary_keys(void)
			{
			return primary_key_list;
			}

		/*
			ANYTIME_INDEX::POSTINGS()
			-------------------------
		*/
		/*!
			@brief Return a pointer to the start of the postings "file"
			@return A pointer to the start of the postings "file"
		*/
		const uint8_t *postings(void)
			{
			return reinterpret_cast<const uint8_t *>(&postings_memory[0]);
			}

		/*
			ANYTIME_INDEX::DOCUMENT_COUNT()
			-------------------------------
		*/
		/*!
			@brief Return the number of documents in the collection
			@return the number of documents in the collection
		*/
		size_t document_count(void)
			{
			return documents;
			}

		/*
			ANYTIME_INDEX::POSTINGS_DETAILS()
			---------------------------------
		*/
		/*!
			@brief Return the meta-data about the postings list
			@param metadata [out] If the term is found then this is is changed to contain the metadata about the term
			@param term [in] Find the metadata for this term
			@return true on success, false on fail (e.g. term not in dictionary)
		*/
	 	bool postings_details(metadata &metadata, const JASS::query_term &term)
			{
			auto found = std::lower_bound(vocabulary_list.begin(), vocabulary_list.end(), term.token());
			if (term.token() == found->term)
				{
				metadata = *found;
				return true;
				}

			return false;
			}
	};
