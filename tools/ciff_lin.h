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
				CLASS CIFF_LIN::POSTINGS_LIST
				-----------------------------
			*/
			/*!
				@brief A postings list with a term, df, cf, and postings list of <d,tf> pairs.
			*/
			class postings_list
				{
				public:
					/*
						CLASS CIFF_LIN::POSTING
						-----------------------
					*/
					/*!
						@brief A <docid,tf> tuple.
					*/
					class posting
						{
						public:
							uint32_t docid;					///< The Document identifier (probably d-gap encoded).
							uint32_t term_frequency;		///< The number of time the term occurs in document with id docid.
						} ;

				public:
					protobuf::slice term;					///< The term as a <pointer,length> tuple.
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
					static error_code get_next_postings_pair(postings_list::posting &into, const uint8_t *&stream, const uint8_t *stream_end)
						{
						while (stream < stream_end)
							{
							protobuf::wire_type type;
							uint8_t field = protobuf::get_type_and_field(type, stream);

							if (type == protobuf::VARINT)
								{
								uint64_t value = protobuf::get_uint64_t(stream);
								if (field == 1)
									into.docid = value;
								else if (field == 2)
									into.term_frequency = value;
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
								protobuf::slice term = protobuf::get_blob(stream);

								if (field == 1)
									into.term = term;
								else if (field == 4)
									{
									const uint8_t *here = term.start;
									postings_list::posting d_tf_pair;
									if (get_next_postings_pair(d_tf_pair, here, here + term.length) == FAIL)
										return FAIL;
									into.postings.push_back(d_tf_pair);
									}
								}
							else
								return FAIL;
							}
						return OK;
						}
				} ;
				
		private:
			/*
				CLASS CIFF_LIN::ITERATOR
				------------------------
			*/
			/*!
				@brief iterator class for iterating over an index
			*/
			class iterator
				{
				private:
					ciff_lin &source;					///< The ciff_lin being iterated over
					const uint8_t *stream;					///< Where in the source stream we are reading from
					size_t where;						///< Are we the start or end of the data
					postings_list postings;			///< The postings list we just made

				public:
					/*
						CIFF_LIN::ITERATOR::ITERATOR()
						------------------------------
					*/
					/*!
						@brief Constructor
						@param of [in] The ciff_lin object we iterate over
						@where [in] Either 0 for start() or file_size for end()
					*/
					iterator(ciff_lin &of, size_t where) :
						source(of),
						stream(source.source_file),
						where(where)
						{
						/* Nothing */
						}

					/*
						CIFF_LIN::ITERATOR::OPERATOR++()
						--------------------------------
					*/
					/*!
						@brief Move on to the next postings list by constructing and storing the current one.  On error move to the end of the stream and mark it as bad
						@return A reference to this iterator
					*/
					const iterator &operator++()
						{
						int64_t postings_list_length = protobuf::get_uint64_t(stream);

						if (postings_list::get_next_postings(postings, stream, stream + postings_list_length) == FAIL)
							{
							/*
								On error move to the end of the stream and mark the stream as bad
							*/
							stream = source.source_file + source.source_file_length;
							source.status = FAIL;
							}

						return *this;
						}

					/*
						CIFF_LIN::ITERATOR::OPERATOR*()
						-------------------------------
					*/
					/*!
						@brief Return the most recently constructed postings list
						@return A reference to the current postings list
					*/
					const postings_list &operator*() const
						{
						return postings;
						}

					/*
						CIFF_LIN::ITERATOR::OPERATOR!=()
						--------------------------------
					*/
					/*!
						@brief Compare two iterators.
						@return True if the iterators are different
					*/
					bool operator!=(const iterator &with) const
						{
						return stream + where != with.stream + with.where;
						}
				};

		private:
			const uint8_t *source_file;		///< The CIFF file in memory
			size_t source_file_length;			///< The length of the CIFF file in bytes
			
		public:
			error_code status;					///< OK or FAIL (FAIL only on error in input stream)

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
			ciff_lin(const uint8_t *source_file, size_t source_file_length) :
				source_file(source_file),
				source_file_length(source_file_length),
				status(OK)
				{
				/* Nothing */
				}

			/*
				CIFF_LIN::BEGIN()
				-----------------
			*/
			/*!
				@brief Return and iterator to the start of this object
			*/
			const iterator begin()
				{
				return ++iterator(*this, 0);
				}

			/*
				CIFF_LIN::END()
				---------------
			*/
			/*!
				@brief Return and iterator to the end of this object
			*/
			const iterator end()
				{
				return iterator(*this, source_file_length);
				}
		} ;
	}
