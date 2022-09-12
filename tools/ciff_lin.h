/*
	CIFF_LIN.H
	----------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Reader for Jimmy Lin's shared index format.
	@details Jimmy uses Anserini to index and then exports using Google protocol buffers.  The
	protocol buffer format is specified by:

	@code{.unparsed}
	syntax = "proto3";
	package io.anserini.cidxf;

	message Posting {
	  int32 docid = 1;
	  int32 tf = 2;
	}

	message PostingsList {
	  string term = 1;
	  int64 df = 2;
	  int64 cf = 3;
	  repeated Posting posting = 4;
	}
	@endcode

	Where each PostingsList is written using writeDelimitedTo() and so each postings list is prefixed by a length integer.

	This code provides an iterator over a file of this format (once read into memory)

	For details of the encoding see: https://developers.google.com/protocol-buffers/docs/encoding
*/
#pragma once

#include <stdint.h>

#include <vector>
#include <limits>

#include "posting.h"
#include "protobuf.h"

namespace JASS
	{
	/*
		CLASS CIFF_LIN
		--------------
	*/
	/*!
		@brief Reader for Jimmy Lin's shared index format.
		@details Jimmy uses Anserini to index and then exports using Google protocol buffers.  The
		protocol buffer format is specified by:

		@code{.unparsed}

		syntax = "proto3";

		package io.osirrc.ciff;

		// An index stored in CIFF is a single file comprised of exactly the following:
		//  - A Header protobuf message,
		//  - Exactly the number of PostingsList messages specified in the num_postings_lists field of the Header
		//  - Exactly the number of DocRecord messages specified in the num_doc_records field of the Header
		// The protobuf messages are defined below.

		// This is the CIFF header. It always comes first.
		message Header {
		  int32 version = 1;              // Version.

		  int32 num_postings_lists = 2;   // Exactly the number of PostingsList messages that follow the Header.
		  int32 num_docs = 3;             // Exactly the number of DocRecord messages that follow the PostingsList messages.

		  // The total number of postings lists in the collection; the vocabulary size. This might differ from
		  // num_postings_lists, for example, because we only export the postings lists of query terms.
		  int32 total_postings_lists = 4;

		  // The total number of documents in the collection; might differ from num_doc_records for a similar reason as above.
		  int32 total_docs = 5;

		  // The total number of terms in the entire collection. This is the sum of all document lengths of all documents in
		  // the collection.
		  int64 total_terms_in_collection = 6;

		  // The average document length. We store this value explicitly in case the exporting application wants a particular
		  // level of precision.
		  double average_doclength = 7;

		  // Description of this index, meant for human consumption. Describing, for example, the exporting application,
		  // document processing and tokenization pipeline, etc.
		  string description = 8;
		}

		// An individual posting.
		message Posting {
		  int32 docid = 1;
		  int32 tf = 2;
		}

		// A postings list, comprised of one ore more postings.
		message PostingsList {
		  string term = 1;   // The term.
		  int64 df = 2;      // The document frequency.
		  int64 cf = 3;      // The collection frequency.
		  repeated Posting postings = 4;
		}

		// A record containing metadata about an individual document.
		message DocRecord {
		  int32 docid = 1;               // Refers to the docid in the postings lists.
		  string collection_docid = 2;   // Refers to a docid in the external collection.
		  int32 doclength = 3;           // Length of this document.
		}
		@endcode

		Where each PostingsList is written using writeDelimitedTo() and so each postings list is prefixed by a length integer.

		This code provides an iterator over a file of this format (once read into memory)

		For details of the encoding see: https://developers.google.com/protocol-buffers/docs/encoding
	*/
	class ciff_lin
		{
		public:
			/*!
				@enum error_code
				@brief success or failure.
			*/
			enum error_code
				{
				OK = 0,			///< Method completed successfully
				FAIL = 1			///< Method did not completed successfully
				} ;

			/*
				CLASS CIFF_LIN::HEADER
				----------------------
			*/
			/*!
				@brief The header of the CIFF file, it happens first in the file and describes how many postings and document details are included.
			*/
			class header
				{
				public:
					int32_t version;              			///< Version number of the CIFF standard.
					int32_t num_postings_lists;  		 	///< Exactly the number of PostingsList messages that follow the Header.
					int32_t num_docs;            		 	///< Exactly the number of DocRecord messages that follow the PostingsList messages.
					int32_t total_postings_lists;			///< The total number of postings lists in the collection; the vocabulary size. This might differ from num_postings_lists, for example, because we only export the postings lists of query terms.
					int32_t total_docs;						///< The total number of documents in the collection; might differ from num_doc_records for a similar reason as above.
					int64_t total_terms_in_collection;	///< The total number of terms in the entire collection. This is the sum of all document lengths of all documents in the collection.
					double average_doclength;				///< The average document length. We store this value explicitly in case the exporting application wants a particular level of precision.
					std::string description;						///< Description of this index, meant for human consumption. Describing, for example, the exporting application, document processing and tokenization pipeline, etc.

				public:
					/*
						CIFF_LIN::HEADER::HEADER()
						--------------------------
					*/
					/*!
						@brief Constructor
					*/
					header():
						version(0),
						num_postings_lists(0),
						num_docs(0),
						total_postings_lists(0),
						total_docs(0),
						total_terms_in_collection(0),
						average_doclength(0),
						description("")
						{
						/* Nothing */
						}

					/*
						CIFF_LIN::HEADER::TEXT_RENDER()
						-------------------------------
					*/
					/*!
						@brief Dump the value of the header to stdout
					*/
					void text_render(void)
						{
						std::cout << "version:" << version << "\n";
						std::cout << "num_postings_lists:" << num_postings_lists << "\n";
						std::cout << "num_docs:" << num_docs << "\n";
						std::cout << "total_postings_lists:" << total_postings_lists << "\n";
						std::cout << "total_docs:" << total_docs << "\n";
						std::cout << "total_terms_in_collection:" << total_terms_in_collection << "\n";
						std::cout << "average_doclength:" << average_doclength << "\n";
						std::cout << "description:" << description << "\n";
						}
				};

			/*
				CLASS CIFF_LIN::POSTINGS_LIST
				-----------------------------
			*/
			/*!
				@brief A postings list with a term, df, cf, and postings list of <d,tf> pairs.
			*/
			class postings_list
				{
				public:
					slice term;									///< The term as a <pointer,length> tuple.
					uint64_t document_frequency;			///< The number of documents containing the term.
					uint64_t collection_frequency;		///< The number of times the term occurs in the collection.
					std::vector<posting> postings;		///< The postings list, a vector of <d,tf> tuples

				private:
					/*
						CIFF_LIN::POSTINGS_LIST::GET_NEXT_POSTINGS_PAIR()
						-------------------------------------------------
					*/
					/*!
						@brief read a <d,tf> pair from a protobuf encoded stream
						@param into [out] The posting pair being read
						@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
						@param stream_end [in] A pointer to the byte after the last byte in the stream.
						@return OK on success, FAIL on error (unknown field number)
					*/
					static error_code get_next_postings_pair(posting &into, const uint8_t *&stream, const uint8_t *stream_end)
						{
						while (stream < stream_end)
							{
							protobuf::wire_type type;
							uint8_t field = protobuf::get_type_and_field(type, stream);

							if (type == protobuf::VARINT)
								{
								uint64_t value = protobuf::get_uint64_t(stream);
								if (field == 1)
									into.docid = static_cast<uint32_t>(value);
								else if (field == 2)
									into.term_frequency = static_cast<uint32_t>(value);
								else
									return FAIL;
								}
							else
								return FAIL;
							}
						return OK;
						}

				public:
					/*
						CIFF_LIN::POSTINGS_LIST::GET_NEXT_POSTINGS()
						--------------------------------------------
					*/
					/*!
						@brief read a full postings list (inclding term, df, cf, etc.) from a protobuf encoded stream
						@param into [out] The postings list being read
						@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
						@param stream_end [in] A pointer to the byte after the last byte in the stream.
						@return OK on success, FAIL on error (unknown field number)
					*/
					static error_code get_next_postings(postings_list &into, const uint8_t *&stream, const uint8_t *stream_end)
						{
						while (stream < stream_end)
							{
							protobuf::wire_type type;
							uint8_t field = protobuf::get_type_and_field(type, stream);

							if (type == protobuf::VARINT)
								{
								uint64_t value = protobuf::get_uint64_t(stream);

								if (field == 2)
									into.document_frequency = value;
								else if (field == 3)
									into.collection_frequency = value;
								else
									return FAIL;
								}
							else if (type == protobuf::BLOB)
								{
								slice term = protobuf::get_blob(stream);

								if (field == 1)
									into.term = term;
								else if (field == 4)
									{
									const uint8_t *here = reinterpret_cast<const uint8_t *> (term.address());
									posting d_tf_pair;
									if (get_next_postings_pair(d_tf_pair, here, here + term.size()) == FAIL)
										return FAIL;
									into.postings.push_back(d_tf_pair);
									}
								}
							else
								return FAIL;
							}
						return OK;
						}

				/*
					CIFF_LIN::POSTINGS_LIST::CLEAR()
					--------------------------------
				*/
				/*!
					@brief removes all content from the postings list
				*/
				void clear(void)
					{
					term.clear();
					document_frequency = 0;
					collection_frequency = 0;
					postings.clear();
					}
				};

			/*
				CLASS CIFF_LIN::DOC_RECORD
				--------------------------
			*/
			/*!
				@brief a document record object containing document lengths and primary keys
			*/
			class doc_record
				{
				public:
					int32_t docid;					///< The docid in the postings lists.
					slice collection_docid;		///< The primary key as a <pointer,length> tuple.
					int32_t doclength;			///< The length of the document.

				public:
					/*
						CIFF_LIN::DOC_RECORD::DOC_RECORD()
						----------------------------------
					*/
					/*!
						@brief Constructor
					*/
					doc_record() :
						docid(0),
						doclength(0)
						{
						/* Nothing */
						}

					/*
						CIFF_LIN::DOC_RECORD::CLEAR()
						-----------------------------
					*/
					/*!
						@brief Clear the contents of this doc record.
					*/
					void clear(void)
						{
						docid = 0;
						collection_docid.clear();
						doclength = 0;
						}

					/*
						CIFF_LIN::DOC_RECORD::GET_NEXT()
						--------------------------------
					*/
					/*!
						@brief Get the next doc record from the stream
					*/
					static error_code get_next(doc_record &into, const uint8_t *&stream)
						{
						size_t length = protobuf::get_uint64_t(stream);
						const uint8_t *stream_end = stream + length;

						while (stream < stream_end)
							{
							protobuf::wire_type type;
							uint8_t field = protobuf::get_type_and_field(type, stream);

							if (type == protobuf::VARINT)
								{
								uint64_t value = protobuf::get_uint64_t(stream);

								if (field == 1)
									into.docid = value;
								else if (field == 3)
									into.doclength = value;
								else
									return FAIL;
								}
							else if (type == protobuf::BLOB)
								{
								slice term = protobuf::get_blob(stream);
								if (field == 2)
									into.collection_docid = term;
								else
									return FAIL;
								}
							else
								return FAIL;
							}
						return OK;
						}
				};
				
		private:
			/*
				CLASS CIFF_LIN::POSTINGS_LIST_ITERATOR
				--------------------------------------
			*/
			/*!
				@brief iterator class for iterating over an index
			*/
			class postings_list_iterator
				{
				private:
					ciff_lin &source;					///< The ciff_lin being iterated over
					const uint8_t *stream;			///< Where in the source stream we are reading from
					size_t which;						///< Which postings list we are current on (counting from 0)
					postings_list postings;			///< The postings list we just made

				public:
					/*
						CIFF_LIN::POSTINGS_LIST_ITERATOR::POSTINGS_LIST_ITERATOR()
						----------------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param of [in] The ciff_lin object we iterate over
						@param where [in] Either 0 for start of n for the number of postings lists in the file
					*/
					postings_list_iterator(ciff_lin &of, size_t which) :
						source(of),
						stream(source.stream),
						which(which)
						{
						/* Nothing */
						}

					/*
						CIFF_LIN::POSTINGS_LIST_ITERATOR::OPERATOR++()
						----------------------------------------------
					*/
					/*!
						@brief Move on to the next postings list by constructing and storing the current one.  On error move to the end of the stream and mark it as bad
						@return A reference to this iterator
					*/
					postings_list_iterator &operator++()
						{
						which++;
						return *this;
						}

					/*
						CIFF_LIN::POSTINGS_LIST_ITERATOR::OPERATOR*()
						---------------------------------------------
					*/
					/*!
						@brief Return the most recently constructed postings list
						@return A reference to the current postings list
					*/
					postings_list &operator*()
						{
						int64_t postings_list_length = protobuf::get_uint64_t(stream);

						postings.clear();
						if (postings_list::get_next_postings(postings, stream, stream + postings_list_length) == FAIL)
							{
							/*
								On error move to the end of the stream and mark the stream as bad
							*/
							which = (std::numeric_limits<decltype(which)>::max)();
							source.status = FAIL;
							}

						return postings;
						}

					/*
						CIFF_LIN::POSTINGS_LIST_ITERATOR::OPERATOR!=()
						----------------------------------------------
					*/
					/*!
						@brief Compare two iterators.
						@return True if the iterators are different
					*/
					bool operator!=(postings_list_iterator &with)
						{
						bool answer = which <= with.which;
						source.stream = stream;
						return answer;
						}
				};

			/*
				CLASS CIFF_LIN::POSTINGS_FOREACH
				--------------------------------
			*/
			/*!
				@brief An object used to allow iteration over postings lists
			*/
			class postings_foreach
				{
				private:
					ciff_lin &parent;

				public:
					postings_foreach(ciff_lin &object) :
						parent(object)
						{
						/* Nothing */
						}

				/*
					CIFF_LIN::POSTINGS_FOREACH::BEGIN()
					-----------------------------------
				*/
				/*!
					@brief Return and iterator to the start of this object
				*/
				ciff_lin::postings_list_iterator begin()
					{
					return ++ciff_lin::postings_list_iterator(parent, 0);
					}

				/*
					CIFF_LIN::POSTINGS_FOREACH::END()
					---------------------------------
				*/
				/*!
					@brief Return and iterator to the end of this object
				*/
				ciff_lin::postings_list_iterator end()
					{
					return ciff_lin::postings_list_iterator(parent, parent.ciff_header.num_postings_lists);
					}
				};

			/*
				CLASS CIFF_LIN::DOCRECORDS_ITERATOR
				-----------------------------------
			*/
			/*!
				@brief iterator class for iterating over an index
			*/
			class docrecords_iterator
				{
				private:
					ciff_lin &source;					///< The ciff_lin being iterated over
					const uint8_t *stream;			///< Where in the source stream we are reading from
					size_t which;						///< Which doc record we are current on (counting from 0)
					doc_record current;				///< The doc record list we just made

				public:

					/*
						CIFF_LIN::DOCRECORDS_ITERATOR::DOCRECORDS_ITERATOR()
						----------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param of [in] The ciff_lin object we iterate over
						@where [in] Either 0 for start() or file_size for end()
					*/
					docrecords_iterator(ciff_lin &of, size_t which) :
						source(of),
						stream(source.stream),
						which(which)
						{
						/* Nothing */
						}

					/*
						CIFF_LIN::DOCRECORDS_ITERATOR::OPERATOR++()
						-------------------------------------------
					*/
					/*!
						@brief Move on to the next postings list by constructing and storing the current one.  On error move to the end of the stream and mark it as bad
						@return A reference to this iterator
					*/
					docrecords_iterator &operator++()
						{
						current.clear();

						if (doc_record::get_next(current, stream) == FAIL)
							{
							/*
								On error move to the end of the stream and mark the stream as bad
							*/
							which = (std::numeric_limits<decltype(which)>::max)();
							source.status = FAIL;
							}

						which++;
						return *this;
						}

					/*
						CIFF_LIN::DOCRECORDS_ITERATOR::OPERATOR*()
						------------------------------------------
					*/
					/*!
						@brief Return the most recently constructed postings list
						@return A reference to the current postings list
					*/
					doc_record &operator*()
						{
						return current;
						}

					/*
						CIFF_LIN::DOCRECORDS_ITERATOR::OPERATOR!=()
						-------------------------------------------
					*/
					/*!
						@brief Compare two iterators.
						@return True if the iterators are different
					*/
					bool operator!=(docrecords_iterator &with)
						{
						return which <= with.which;
						}
				};

			/*
				CLASS CIFF_LIN::DOCRECORDS_FOREACH
				----------------------------------
			*/
			/*!
				@brief An object used to allow iteration over document records
			*/
			class docrecords_foreach
				{
				private:
					ciff_lin &parent;

				public:
					docrecords_foreach(ciff_lin &object) :
						parent(object)
						{
						/* Nothing */
						}

				/*
					CIFF_LIN::DOCRECORDS_FOREACH::BEGIN()
					-------------------------------------
				*/
				/*!
					@brief Return and iterator to the start of this object
				*/
				ciff_lin::docrecords_iterator begin()
					{
					return ++ciff_lin::docrecords_iterator(parent, 0);
					}

				/*
					CIFF_LIN::DOCRECORDS_FOREACH::END()
					-----------------------------------
				*/
				/*!
					@brief Return and iterator to the end of this object
				*/
				ciff_lin::docrecords_iterator end()
					{
					return ciff_lin::docrecords_iterator(parent, parent.ciff_header.num_docs);
					}
				};

		private:
			const uint8_t *source_file;				///< The CIFF file in memory
			const uint8_t *stream;						///< Where in the CIFF we currently are
			header ciff_header;							///< The header from the CIFF file
			
		public:
			error_code status;							///< OK or FAIL (FAIL only on error in input stream)

		protected:
			/*
				READ_HEADER()
				-------------
			*/
			/*!
				@brief Read the CIFF header containing details about how many postings lists, etc.
				@param header [out] The header once read.
				@return OK on success, FAIL on failure.
			*/
			error_code read_header(header &header)
				{
				stream = source_file;
				size_t length = protobuf::get_uint64_t(stream);
				const uint8_t *stream_end = stream + length;

				while (stream < stream_end)
					{
					protobuf::wire_type type;
					uint8_t field = protobuf::get_type_and_field(type, stream);

					if (type == protobuf::VARINT)
						{
						uint64_t value = protobuf::get_uint64_t(stream);

						if (field == 1)
							header.version = value;
						else if (field == 2)
							header.num_postings_lists = value;
						else if (field == 3)
							header.num_docs = value;
						else if (field == 4)
							header.total_postings_lists = value;
						else if (field == 5)
							header.total_docs = value;
						else if (field == 6)
							header.total_terms_in_collection = value;
						else
							return FAIL;
						}
					else if (type == protobuf::SIXTY_FOUR_BIT)
						{
						double value = protobuf::get_double(stream);
						if (field == 7)
							header.average_doclength = value;
						else
							return FAIL;
						}
					else if (type == protobuf::BLOB)
						{
						slice term = protobuf::get_blob(stream);

						if (field == 8)
							header.description = std::string(reinterpret_cast<char *>(term.address()), term.size());
						else
							return FAIL;
						}
					else
						return FAIL;
					}
				return OK;
				}

		public:
			/*
				CIFF_LIN::CIFF_LIN()
				--------------------
			*/
			/*!
				@brief Constructor
				@param source_file [in] a Pointer to the protobuf file once already read into memory.
				@param source_file_length [in] The length (in bytes) of the source file once in memory.
			*/
			ciff_lin(const uint8_t *source_file) :
				source_file(source_file),
				stream(source_file),
				status(OK)
				{
				read_header(ciff_header);
				ciff_header.text_render();
				}

			/*
				CIFF_LIN::POSTINGS()
				--------------------
			*/
			/*!
				@brief Return an object capable of being an iterator for postings lists.  Assumes the "file pointer" is in the right place.
				@return an object for use in a for statement thus: for (const auto &postings_list : ciff.postings())
			*/
			postings_foreach postings(void)
				{
				return postings_foreach(*this);
				}

			/*
				CIFF_LIN::DOCRECORDS()
				----------------------
			*/
			/*!
				@brief Return an object capable of being an iterator for document details.  Assumes the "file pointer" is in the right place.
				@return an object for use in a for statement thus: for (const auto &postings_list : ciff.docrecords())
			*/
			docrecords_foreach docrecords(void)
				{
				return docrecords_foreach(*this);
				}

			/*
				CIFF_LIN::GET_HEADER()
				----------------------
			*/
			/*!
				@brief Return the header object
				@return a ciff_lin::header object from the start of the file
			*/
			header &get_header(void)
				{
				return ciff_header;
				}
		} ;
	}
