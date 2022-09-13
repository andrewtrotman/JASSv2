/*
	DESERIALISED_JASS_V1.H
	----------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Load and deserialise a JASS v1 index
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "string.h"

#include <string>
#include <vector>

#include "file.h"
#include "slice.h"
#include "query_term.h"
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS DESERIALISED_JASS_V1
		--------------------------
	*/
	/*!
		@brief Load and deserialise a JASS v1 index
	*/
	class deserialised_jass_v1
		{
		private:
			static constexpr const char *PRIMARY_KEY_FILENAME = "CIdoclist.bin";
			static constexpr const char *VOCAB_FILENAME = "CIvocab.bin";
			static constexpr const char *TERMS_FILENAME = "CIvocab_terms.bin";
			static constexpr const char *POSTINGS_FILENAME = "CIpostings.bin";

		public:
			/*
				CLASS DESERIALISED_JASS_V1::SEGMENT_HEADER
				------------------------------------------
			*/
			/*!
				@brief Each impact ordered segment contains a header with the impact score and the pointers to documents.
				@details Each JASS v1 postings list consists of a list of pointers to segment headers which, in turn, point to lists of document identifiers.  The
				segment header contains the impact score, a pointer to the (compressed) postings list, and the numner of documents in the list (the segment
				frequency).
			*/
			class segment_header
				{
				public:
					uint32_t impact;					///< The impact score.  Not a query::ACCUMUMLTOR_TYPE as this can overflow
					uint64_t offset;					///< Offset (within the postings file) of the start of the compressed postings list
					uint64_t end;						///< Offset (within the postings file) of the end of the compressed postings list
					query::DOCID_TYPE segment_frequency;			///< The number of document ids in the segment (not end - offset because the postings are compressed)
				};
			/*
				CLASS DESERIALISED_JASS_V1::SEGMENT_HEADER_ON_DISK
				--------------------------------------------------
			*/
			/*!
				@brief Each impact ordered segment contains a header with the impact score and the pointers to documents.
				@details Each JASS v1 postings list consists of a list of pointers to segment headers which, in turn, point to lists of document identifiers.  The
				segment header contains the impact score, a pointer to the (compressed) postings list, and the numner of documents in the list (the segment
				frequency).
			*/
			#pragma pack(push, 1)
			class segment_header_on_disk
				{
				public:
					uint16_t impact;					///< The impact score
					uint64_t offset;					///< Offset (within the postings file) of the start of the compressed postings list
					uint64_t end;						///< Offset (within the postings file) of the end of the compressed postings list
					uint32_t segment_frequency;			///< The number of document ids in the segment (not end - offset because the postings are compressed)
				};
			#pragma pack(pop)

			/*
				CLASS DESERIALISED_JASS_V1::METADATA
				------------------------------------
			*/
			/*!
				@brief metadata for a given term including pointer to postings and number of impacts.
			*/
			class metadata
				{
				public:
					slice term;								///< Pointer to a '\0' terminated string that is this term's name
					uint8_t *offset;						///< Offset to the postings for this term
					uint64_t impacts;						///< The numner of impact segments this term has

				public:
					/*
						DESERIALISED_JASS_V1::METADATA::METADATA()
						------------------------------------------
					*/
					/*!
						@brief Constructor
					*/
					metadata() :
						term(),
						offset(nullptr),
						impacts(0)
						{
						/* Nothing */
						}

					/*
						DESERIALISED_JASS_V1::METADATA::METADATA()
						------------------------------------------
					*/
					/*!
						@brief Constructor
						@param term [in]  The term this object represents
						@param offset [in] The a ppointer to the postings lists for this term
						@param impacts [in] The number of impacts for this term
					*/
					metadata(const slice &term, const void *offset, uint64_t impacts) :
						term(term),
						offset(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(offset))),
						impacts(impacts)
						{
						/* Nothing */
						}

					/*
						DESERIALISED_JASS_V1::METADATA::OPERATOR<()
						-------------------------------------------
					*/
					/*!
						@brief Less than operator used for sorting terms in the vocabulary.
						@param with [in] what to compare to.
						@return true if this is less than with, else false.
					*/
					bool operator<(const slice &with) const
						{
						return slice::strict_weak_order_less_than(term, with);
						}
				};

		protected:
			bool verbose;												///< Should this class produce diagnostics on stdout?

			uint64_t documents;										///< The number of documents in the collection
			file::file_read_only primary_key_memory;			///< Memory used to store the primary key strings
			std::vector<std::string> primary_key_list;		///< The array of primary keys

			uint64_t terms;											///< The number of terms in the collection
			file::file_read_only vocabulary_memory;			///< Memory used to store the vocabulary pointers
			file::file_read_only vocabulary_terms_memory;	///< Memory used to store the vocabulary strings
			std::vector<metadata> vocabulary_list;				///< The (sorted in alphabetical order) array of vocbulary terms

			file::file_read_only postings_memory;				///< Memory used to store the postings

		protected:
			/*
				DESERIALISED_JASS_V1::READ_PRIMARY_KEYS()
				-----------------------------------------
			*/
			/*!
				@brief Read the JASS v1 index primary key file
				@param primary_key_filename [in] the name of the file containing the primary key list ("CIdoclist.bin")
				@return The number of documents in the collection (or 0 on error)
			*/
			virtual size_t read_primary_keys(const std::string &primary_key_filename = PRIMARY_KEY_FILENAME);

			/*
				DESERIALISED_JASS_V1::READ_VOCABULARY()
				---------------------------------------
			*/
			/*!
				@brief Read the JASS v1 index vocabulary files
				@param vocab_filename [in] the name of the file containing the vocabulary pointers ("CIvocab.bin")
				@param terms_filename [in] the name of the file containing the vocabulary strings ("CIvocab_terms.bin")
				@return The number of documents in the collection (or 0 on error)
			*/
			virtual size_t read_vocabulary(const std::string &vocab_filename = VOCAB_FILENAME, const std::string &terms_filename = TERMS_FILENAME);

			/*
				DESERIALISED_JASS_V1::READ_POSTINGS()
				-------------------------------------
			*/
			/*!
				@brief Read the JASS v1 index postings file
				@param postings_filename [in] the name of the file containing the postings ("CIpostings.bin")
				@return size of the posings file or 0 on failure
			*/
			virtual size_t read_postings(const std::string &postings_filename = POSTINGS_FILENAME);

			/*
				DESERIALISED_JASS_V1::READ_INDEX_EXPLICIT()
				-------------------------------------------
			*/
			/*!
				@brief Read a JASS v1 index into memory
				@param primary_key_filename [in] the name of the file containing the primary key list ("CIdoclist.bin")
				@param vocab_filename [in] the name of the file containing the vocabulary pointers ("CIvocab.bin")
				@param terms_filename [in] the name of the file containing the vocabulary strings ("CIvocab_terms.bin")
				@param postings_filename [in] the name of the file containing the postings ("CIpostings.bin")
				@return 0 on failure, non-zero on success
			*/
			size_t read_index_explicit(const std::string &primary_key_filename = PRIMARY_KEY_FILENAME, const std::string &vocab_filename = VOCAB_FILENAME, const std::string &terms_filename = TERMS_FILENAME, const std::string &postings_filename = POSTINGS_FILENAME);

		public:
			/*
				DESERIALISED_JASS_V1::DESERIALISED_JASS_V1()
				--------------------------------------------
			*/
			/*!
				@brief Constructor
				@param verbose [in] Should the index reading methods produce messages on stdout?
			*/
			explicit deserialised_jass_v1(bool verbose = false) :
				verbose(verbose),
				documents(0),
				terms(0)
				{
				/* Nothing */
				}

			/*
				DESERIALISED_JASS_V1::~DESERIALISED_JASS_V1()
				--------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~deserialised_jass_v1()
				{
				/* Nothing */
				}
				
			/*
				DESERIALISED_JASS_V1::READ_INDEX()
				----------------------------------
			*/
			/*!
				@brief Read a JASS v1 index into memory
				@param directory [in] The directory to search for and index
				@return 0 on failure, non-zero on success
			*/
			size_t read_index(const std::string &directory = "");

			/*
				DESERIALISED_JASS_V1::CODEX()
				-----------------------------
			*/
			/*!
				@brief Return a reference to a decompressor that can be used with this index
				@param name [out] The name of the compression codex
				@param d_ness [out] Whether the codex requires D0, D1, etc decoding (-1 if it supports decode_and_process via decode_none)
				@return A reference to a compress_integer that can decode the given codex
			*/
			std::unique_ptr<compress_integer> codex(std::string &name, int32_t &d_ness) const;

			/*
				DESERIALISED_JASS_V1::PRIMARY_KEYS()
				------------------------------------
			*/
			/*!
				@brief Return the list of primary keys as a std::vector<std::string>
				@return A reference to a vector of primary keys
			*/
			const std::vector<std::string> &primary_keys(void) const
				{
				return primary_key_list;
				}

			/*
				DESERIALISED_JASS_V1::POSTINGS()
				--------------------------------
			*/
			/*!
				@brief Return a pointer to the start of the postings "file"
				@return A pointer to the start of the postings "file"
			*/
			const uint8_t *postings(void) const
				{
				const uint8_t *buffer = nullptr;
				postings_memory.read_entire_file(buffer);
				return buffer;
				}

			/*
				DESERIALISED_JASS_V1::DOCUMENT_COUNT()
				--------------------------------------
			*/
			/*!
				@brief Return the number of documents in the collection
				@return the number of documents in the collection
			*/
			size_t document_count(void) const
				{
				return documents;
				}

			/*
				DESERIALISED_JASS_V1::POSTINGS_DETAILS()
				----------------------------------------
			*/
			/*!
				@brief Return the meta-data about the postings list
				@param metadata [out] If the term is found then this is is changed to contain the metadata about the term
				@param term [in] Find the metadata for this term
				@return true on success, false on fail (e.g. term not in dictionary)
			*/
			bool postings_details(metadata &metadata, const query_term &term) const
				{
				auto found = std::lower_bound(vocabulary_list.begin(), vocabulary_list.end(), term.token());

				/*
					Term not in the dictionary and we've walked off the end.
				*/
				if (found == vocabulary_list.end())
					return false;

				/*
					Term might be in the dictionary, better check
				*/
				if (term.token() == found->term)
					{
					metadata = *found;
					return true;
					}

				/*
					We don't have a match
				*/
				return false;
				}

			/*
				DESERIALISED_JASS_V1::GET_SEGMENT_LIST()
				----------------------------------------
			*/
			/*!
				@brief Extract the segment headers and return them in the parameter called segments
				@param segments [out] The list of segments for the given search term (caller must ensure this ponts to a large enough array)
				@param metadata [in] The metadata for the given search term
				@param smallest [out] The largest impact score for this term
				@param largest [out] The smallest impact score for this term
				@return The number of segments extracted and added to the list
			*/
			virtual size_t get_segment_list(segment_header *segments, metadata &metadata, size_t query_term_frequency, uint32_t &smallest, uint32_t &largest, query::DOCID_TYPE &document_frequency) const
				{
				document_frequency = 0;
				segment_header *current_segment = segments;
				for (uint64_t segment = 0; segment < metadata.impacts; segment++)
					{
					uint64_t *postings_list = (uint64_t *)metadata.offset;
					segment_header_on_disk *next_segment_in_postings_list = (segment_header_on_disk *)(postings() + postings_list[segment]);

					current_segment->impact = next_segment_in_postings_list->impact * query_term_frequency;
					current_segment->offset = next_segment_in_postings_list->offset;
					current_segment->end = next_segment_in_postings_list->end;
					current_segment->segment_frequency = next_segment_in_postings_list->segment_frequency;
					document_frequency += next_segment_in_postings_list->segment_frequency;

					current_segment++;
					}

				/*
					Compute the smallest and largest impact scores and return them in the right order
				*/
				smallest = segments->impact;
				largest = (current_segment - 1)->impact;
				if (smallest > largest)
					std::swap(smallest, largest);

				return metadata.impacts;
				}


			/*
				DESERIALISED_JASS_V1::BEGIN()
				-----------------------------
			*/
			/*!
				@brief return an iterator over the vocabulary.
				@return an iterator over the vocabulary.
			*/
			auto begin(void)
				{
				return vocabulary_list.begin();
				}

			/*
				DESERIALISED_JASS_V1::END()
				---------------------------
			*/
			/*!
				@brief return an iterator to the end of the vocabulary.
				@return an iterator to the end of the vocabulary.
			*/
			auto end(void)
				{
				return vocabulary_list.end();
				}
		};
	}
