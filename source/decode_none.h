/*
	DECODE_NONE
	-----------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The D<n> decoder to use when the compressor / decompressor do the decoding
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
	class decoder_none
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
				DECODER_NONE::DECODE_AND_PROCESS()
				----------------------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param impact [in] The impact score to add for each document id in the list.
				@param accumulators [in] The accumulators to add to
				@param decoder [in] The codex to use to decompress and preocess.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			template <typename QUERY_T, typename DECOMPRESSOR_T>
			void decode_and_process(uint16_t impact, QUERY_T &accumulators, DECOMPRESSOR_T &decoder, size_t integers, const void *compressed, size_t compressed_size)
				{
				accumulators.set_score(impact);
//				decoder.decode(accumulators, integers, compressed, compressed_size);
				}
		};
	}
