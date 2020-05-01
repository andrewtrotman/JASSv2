/*
	DECODE_NONE.H
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Experimental D0 decode used to push add_rsv() into the decompressor
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <vector>

#include "query.h"
#include "compress_integer_none.h"

namespace JASS
	{
	/*
		CLASS DECODER_NONE
		------------------
	*/
	/*!
		@brief The D<n> decoder to use when the compressor / decompressor do the decoding
		@details See decode_d0 and decode_d1.  When the decompressor does the decoding there is no need
		to further process the integer sequence beyond processing it.  So this class implements the
		"none" decoder that does no work.  It differs from D0 in so far as the class implements the
		SIMD scatter / gather behaviour.
	*/
	template <typename DECOMPRESSOR>
	class decoder_none : public DECOMPRESSOR
		{
		public:
			/*
				DECODER_NONE::DECODER_NONE()
				----------------------------
			*/
			/*!
				@brief Constructor
			*/
			decoder_none() = delete;

			/*
				DECODER_NONE::DECODER_NONE()
				----------------------------
			*/
			/*!
				@brief Constructor
				@param max_integers [in] The maximum number of integers that will ever need to be decoded using this object (i.e. the number of documents in the collection + overflow).
			*/
			explicit decoder_none(size_t max_integers)
				{
				/* Nothing */
				}

			/*
				DECODER_NONE::DECODE_AND_ADD_RSV()
				----------------------------------
			*/
			/*!
				@brief Decode the comprerssed postings and call add_rsv() on each one
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			void decode_and_add_rsv(size_t integers, const void *compressed, size_t compressed_size)
				{
				DECOMPRESSOR::decode(integers, compressed, compressed_size);
				}
		};
	}
