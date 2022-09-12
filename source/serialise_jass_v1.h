/*
	SERIALISE_JASS_V1.H
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Serialise an index in the experimental JASS-CI format used (by JASS version 1) in the RIGOR workshop.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "file.h"
#include "slice.h"
#include "allocator_cpp.h"
#include "index_postings.h"
#include "index_manager.h"
#include "compress_integer_qmx_jass_v1.h"
#include "compress_integer_elias_gamma_simd.h"

namespace JASS
	{
	/*
		CLASS SERIALISE_JASS_V1
		-----------------------
	*/
	/*!
		@brief Serialise an index in the experimental JASS-CI format used (by JASS version 1) in the RIGOR workshop.
		@details The original version of JASS was an experimental hack in reducing the complexity of the ATIRE search 
		engine, that resulted in an index that was large, but easy to process. The intent was to go back and "fix" the 
		index to be smaller and faster. That never happened. Instead it was used as the basis of other work. In an 
		effort to bring up this re-write of ATIRE and JASS, compatibility with the hack (known as JASS version 1) is 
		maintained so that the indexer can be checked without writing the search engine itself (i.e. this JASS is being
		 bootstrapped from JASS version 1)

		The paper comparing JASS version 1 to other search engines (including ATIRE) is here: J. Lin, M. Crane, A. Trotman,
		J. Callan, I. Chattopadhyaya, J. Foley, G. Ingersoll, C. Macdonald, S. Vigna (2016), Toward Reproducible Baselines:
		The Open-Source IR Reproducibility Challenge, Proceedings of the European Conference on Information Retrieval
		(ECIR 2016), pp. 408-420.

		The JASS version 1 index in made up of 4 files: CIvocab_terms.bin, CIvocab.bin, CIpostings.bin, and CIdoclist.bin

		CIdoclist.bin: The list of document identifiers (each '\0' terminated). Then an index to each of the doclents 
		(stored as a table of uint64_t). The final 8 bytes of the file is an uin64_t storing the total numbner of unique 
		documents in the collection.

		CIvocab_terms.bin: This is a list of all the unique terms in the collection (the closure of the vocabulary). It is 
		stored as a sequence of '\0' terminated UTF-8 strings. So, if the vocabularty contains three terms, "a", "bb" and "cc",
		then the contents of CIvocab_terms.bin will be "a\0bb\0cc\0". This file does not need to be sorted in alphabetical (or 
		similar) order.

		CIvocab.bin: This is a list of triples (term, offset, impacts). Term is a pointer to the string in the CIvocab_terms.bin 
		file (i.e. a byte offset within the file). Offset is the offset (in CIpostings.bin) of the start of the postings list. 
		Impacts is the number of impacts in the impact ordered postings list. JASS v1 assumes this file is sorted in alphabetical 
		order by the term string (i.e. where term points to) when using strcmp().

		CIpostings.bin: This file contains all the postings lists compressed using the same codex. This is different from 
		ATIRE which allows each postings list to be encoded using a different codex. The first byte of this file specifies 
		the codex where s=uncompressed, c=VarByte, 8=Simple8, q=QMX, Q=QMX4D, R=QMX0D. This is followed by the postings lists.
		A postings list is: a list of 64-bit pointer to headers. Each header is (uint16_t impact_score, uint64_t start,
		uint64_t end, uint32_t impact_frequency) where impact_score is the impact value, start and end are pointers to the
		compressed docids, and impact_frequency is the number of dociment_ids in the list. The header is terminated with a 
		row of all 0s (i.e. 22 consequitive 0-bytes). This is followed by the list of docid's for each segment - each compressed 
		seperately. These lists do not have the impact score stored at the start and do not have 0 terminators on them. This 
		means score-at-a-time processing is the only paradigm, even if term-at-a-time processing is done score-at-a-time for 
		each term. ATIRE could do either (but it was a compile time flag).
	*/
	class serialise_jass_v1 : public index_manager::delegate
		{
		protected:
			/*
				CLASS SERIALISE_JASS_V1::VOCAB_TRIPPLE
				--------------------------------------
			*/
			/*!
				@brief The tripple used in CIvocab.bin
			*/
			class vocab_tripple
				{
				public:
					slice	token;				///< The term as a string (needed for sorting the std::vector vocab_tripple array later)
					uint64_t term;				///< The pointer to the \0 terminated string in the CI_vovab_terms.bin file.
					uint64_t offset;			///< The pointer to the postings stored in the CIpostings.bin file.
					uint64_t impacts;			///< The number of impacts that exist for this term.

				public:
					/*
						 SERIALISE_JASS_V1::VOCAB_TRIPPLE::VOCAB_TRIPPLE()
						--------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param string [in] The term that that this object represents.
						@param term [in] The location of this term in CIvocab_terms.bin.
						@param offset [in] The offset of the postings list in CIpostings.bin.
						@param impacts [in] The number of impacts in the postings list.
					*/
					vocab_tripple(const slice &string, uint64_t term, uint64_t offset, uint64_t impacts) :
						token(string),
						term(term),
						offset(offset),
						impacts(impacts)
						{
						/* Nothing */
						}
					
					/*
						 SERIALISE_JASS_V1::VOCAB_TRIPPLE::OPERATOR<()
						----------------------------------------------
					*/
					/*!
						@brief Compare (using strcmp() colaiting sequence) this object with another for less than.
						@param other [in] The object to compare to
						@return true if this < other, else false
					*/
					bool operator<(const vocab_tripple &other) const
						{
						return slice::strict_weak_order_less_than(token, other.token);
						}
				};

		public:
			/*
				ENUM JASS_V1_CODEX
				------------------
			*/
			/*!
				@brief The compression scheme that is active
			*/
			enum  jass_v1_codex
				{
				uncompressed = 's',				///< Postings are not compressed.
				variable_byte = 'c',				///< Postings are compressed using ATIRE's variable byte encoding.
				simple_8b = '8',					///< Postings are compressed using ATIRE's simple-8b encoding.
				qmx = 'q',							///< Postings are compressed using JASS v1's variant of QMX (with difference (D1) encoding).
				qmx_d4 = 'Q',						///< Postings are compressed using QMX with Lemire's D4 delta encoding.
				qmx_d0 = 'R',						///< Postings are compressed using QMX without delta encoding.
				elias_gamma_simd = 'G',			///< Postings are compressed using Elias gamma SIMD encoding.
				elias_gamma_simd_vb = 'g',		///< Postings are compressed using Elias gamma SIMD encoding with variable byte endings.
				elias_delta_simd = 'D'			///< Postings are compressed using Elias delta SIMD encoding.
				};

		protected:
			file vocabulary_strings;							///< The concatination of UTS-8 encoded unique tokens in the collection.
			file vocabulary;										///< Details about the term (including a pointer to the term, a pointer to the postings, and the quantum count.
			file postings;											///< The postings lists.
			file primary_keys;									///< The list of external identifiers (document primary keys).
			std::vector<vocab_tripple> index_key;			///< The entry point into the JASS v1 index is CIvocab.bin, the index key.
			std::vector<uint64_t> primary_key_offsets;	///< A list of locations (on disk) of each primary key.
			allocator_pool memory;								///< Memory used to store the impact-ordered postings list.
			index_postings_impact impact_ordered;			///< The re-used impact ordered postings list.
			std::string compressor_name;						///< The name of the compresson algorithm
			int compressor_d_ness;								///< The d-ness of the compression algorithm
			std::unique_ptr<compress_integer> encoder;							///< The integer encoder used to compress postings lists.
			allocator_cpp<uint8_t> allocator;				///< C++ allocator between memory object and std::vector object
			std::vector<uint8_t, allocator_cpp<uint8_t>> compressed_buffer;		///< The buffer used to compress postings into.
			std::vector<slice, allocator_cpp<slice>> compressed_segments;			///< vector of pointers (and lengths) to the compressed postings.
			uint8_t alignment;									///< Postings lists are padded to this alignment (used for codexes that require word alignment).

		protected:
			/*
				SERIALISE_JASS_V1::WRITE_POSTINGS()
				-----------------------------------
			*/
			/*!
				@brief Convert the postings list to the JASS v1 format and serialise it to disk.
				@param postings [in] The postings list to serialise.
				@param number_of_impacts [out] The number of distinct impact scores seen in the postings list.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
				@return The location (in CIpostings.bin) of the start of the serialised postings list.
			*/
			virtual size_t write_postings(const index_postings &postings, size_t &number_of_impacts, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies);

		public:
			/*
				SERIALISE_JASS_V1::SERIALISE_JASS_V1()
				--------------------------------------
			*/
			/*!
				@brief Constructor
				@param documents [in] The number of documents in the collection (used to allocate re-usable buffers).
				@param encoder [in] An shared pointer to a codex responsible for performing the compression of postings lists (default = compress_integer_QMX_jass_v1()).
				@param alignment [in] The start address of a postings list is padded to start on these boundaries (needed for compress_integer_QMX_jass_v1 (use 16), and others).  Default = 0.
			*/
			serialise_jass_v1(size_t documents, jass_v1_codex codex = jass_v1_codex::elias_gamma_simd, int8_t alignment = 1) :
				index_manager::delegate(documents),
				vocabulary_strings("CIvocab_terms.bin", "w+b"),
				vocabulary("CIvocab.bin", "w+b"),
				postings("CIpostings.bin", "w+b"),
				primary_keys("CIdoclist.bin", "w+b"),
				memory(1024 * 1024),								///< The allocation block size is currently 1MB, big enough for most postings lists (but it'll grow for larger ones).
				impact_ordered(documents, memory),
				encoder(get_compressor(codex, compressor_name, compressor_d_ness)),
				allocator(memory),
				compressed_buffer(allocator),
				compressed_segments(allocator),
				alignment(alignment)
				{
				/*
					Allocate space for storing the compressed postings.  But, allocate too much space as some
					encoders can't write a sequence smaller than a minimum size.

					How large does this need to be?
					We store the:
						postings, each docid is 8 bytes and there are |documents| of those
						The impact header consisting of an impact (2 bytes) + start_pointer (8 bytes) + length (8 bytes) + frequency (4 bytes)
						There are index_postings_impact::largest_impact of those.
					To make things worse, each of these are stored compressed, and so might be bigger than the
					raw size by 8/7 (assuming variable byte), so the raw storage is:
						 8/7 *(documents * 8 + 22 * index_postings_impact::largest_impact)
					but, each of these two things is stored in a vector as a slice and each slice takes
					8 bytes for the address and 8 bytes for the size giving an additional 2 * 16 * index_postings_impact::largest_impact
					giving a total of
						8/7 *(documents * 8 + (22 + 2 * 16) * index_postings_impact::largest_impact)
					and now lets add a bit for reasons we can't predict (the std::vector has house-keeping)
						1 MB
					and make sure integer rounding doesn't get this wrong:
						8 * (documents * 8 + (22 + 2 * 16) * index_postings_impact::largest_impact) / 7 + 1024 * 1024
				*/
				compressed_buffer.resize(8 * (documents * 8 + (22 + 2 * 16) * index_postings_impact::largest_impact) / 7 + 1024 * 1024);
				compressed_segments.reserve(index_postings_impact::largest_impact);

// std::cout << compressor_name << "-D" << compressor_d_ness << "\n";

				postings.write(&codex, 1);
				}

			/*
				SERIALISE_JASS_V1::~SERIALISE_JASS_V1()
				--------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~serialise_jass_v1()
				{
				/* Nothing */
				}

			/*
				SERIALISE_JASS_V1::FINISH()
				---------------------------
			*/
			/*!
				@brief Finish up any serialising that needs to be done.
			*/
			virtual void finish(void);

			/*
				 SERIALISE_JASS_V1::SERIALISE_VOCABULARY_POINTERS()
				--------------------------------------------------
			*/
			/*!
				@brief Serialise the pointers that point between the vocab and the postings (the CIvocab.bin file).
			*/
			virtual void serialise_vocabulary_pointers(void);

			/*
				 SERIALISE_JASS_V1::SERIALISE_PRIMARY_KEYS()
				--------------------------------------------
			*/
			/*!
				@brief Serialise the primary keys (or any extra stuff at the end of the primary key file).
			*/
			virtual void serialise_primary_keys(void);

			/*
				SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
				-----------------------------------------
			*/
			/*!
				@brief The callback function to serialise the postings (given the term) is operator().
				@param term [in] The term name.
				@param postings [in] The postings lists.
				@param document_frequency [in] The document frequency of the term
				@param document_ids [in] An array (of length document_frequency) of document ids.
				@param term_frequencies [in] An array (of length document_frequency) of term frequencies (corresponding to document_ids).
			*/
			virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies);

			/*
				SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
				-----------------------------------------
			*/
			/*!
				@brief The callback function to serialise the primary keys (external document ids) is operator().
				@param document_id [in] The internal document identfier.
				@param primary_key [in] This document's primary key (external document identifier).
			*/
			virtual void operator()(size_t document_id, const slice &primary_key);


			/*
				SERIALISE_JASS_V1::GET_COMPRESSOR()
				-----------------------------------
			*/
			/*!
				@brief Return a reference to a compressor/decompressor that can be used with this index
				@param codex [in] The codex to use
				@param name [out] The name of the compression codex
				@param d_ness [out] Whether the codex requires D0, D1, etc decoding (-1 if it supports decode_and_process via decode_none)
				@return A reference to a compress_integer that can decode the given codex
			*/
			static std::unique_ptr<compress_integer> get_compressor(jass_v1_codex codex, std::string &name, int32_t &d_ness);

			/*
				SERIALISE_JASS_V1::UNITTEST()
				-----------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
