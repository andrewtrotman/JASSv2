/*
	DESERIALISED_JASS_V2.H
	----------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Load and deserialise a JASS v2 index
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman
*/
#pragma once

#include "deserialised_jass_v1.h"
#include "compress_integer_variable_byte.h"

namespace JASS
	{
	/*
		CLASS DESERIALISED_JASS_V2
		--------------------------
	*/
	/*!
		@brief Load and deserialise a JASS v2 index
	*/
	class deserialised_jass_v2 : public deserialised_jass_v1
		{
		protected:
			/*
				DESERIALISED_JASS_V2::READ_VOCABULARY()
				---------------------------------------
			*/
			/*!
				@brief Read the JASS v2 index vocabulary files
				@param vocab_filename [in] the name of the file containing the vocabulary pointers ("CIvocab.bin")
				@param terms_filename [in] the name of the file containing the vocabulary strings ("CIvocab_terms.bin")
				@return The number of documents in the collection (or 0 on error)
			*/
			virtual size_t read_vocabulary(const std::string &vocab_filename = "CIvocab.bin", const std::string &terms_filename = "CIvocab_terms.bin");

			/*
				DESERIALISED_JASS_V2::READ_PRIMARY_KEYS()
				-----------------------------------------
			*/
			/*!
				@brief Read the JASS v1 index primary key file
				@param primary_key_filename [in] the name of the file containing the primary key list ("CIdoclist.bin")
				@return The number of documents in the collection (or 0 on error)
			*/
			virtual size_t read_primary_keys(const std::string &primary_key_filename = "CIdoclist.bin");

		public:
			/*
				DESERIALISED_JASS_V2::DESERIALISED_JASS_V2()
				--------------------------------------------
			*/
			/*!
				@brief Constructor
				@param verbose [in] Should the index reading methods produce messages on stdout?
			*/
			explicit deserialised_jass_v2(bool verbose = false) :
				deserialised_jass_v1(verbose)
				{
				/* Nothing */
				}

			/*
				DESERIALISED_JASS_V2::~DESERIALISED_JASS_V2()
				--------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~deserialised_jass_v2()
				{
				/* Nothing */
				}

			/*
				DESERIALISED_JASS_V2::POSTINGS_DETAILS()
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
				DESERIALISED_JASS_V2::GET_SEGMENT_LIST()
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
				/*
					Extract all the segments
				*/
				segment_header *current_segment = segments;
				uint8_t *segment_header_pointer = metadata.offset;

				for (uint64_t segment = 0; segment < metadata.impacts; segment++)
					{
					compress_integer_variable_byte::decompress_into(&current_segment->impact, segment_header_pointer);
					compress_integer_variable_byte::decompress_into(&current_segment->offset, segment_header_pointer);
					compress_integer_variable_byte::decompress_into(&current_segment->end , segment_header_pointer);
					compress_integer_variable_byte::decompress_into(&current_segment->segment_frequency, segment_header_pointer);
					current_segment->offset += segment_header_pointer - postings();		//v2 index is relative to the segment header
					current_segment->impact *= query_term_frequency;
					current_segment->end += current_segment->offset;					// V2 indexes store length rather than an end pointer
					document_frequency += current_segment->segment_frequency;
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
		};
	}
