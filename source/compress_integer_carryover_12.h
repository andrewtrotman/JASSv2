/*
	COMPRESS_INTEGER_CARRYOVER_12.H
	-------------------------------
	Functions for Vo Ngoc Anh and Alistair Moffat's Carryover-12 compression scheme
	This is a port of Anh and Moffat's code to ATIRE then ported to JASS.

	Originally (http://www.cs.mu.oz.au/~alistair/carry/)
		Copyright (C) 2003  Authors: Vo Ngoc Anh & Alistair Moffat

		This program is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

	These Changes
		Copyright (C) 2006, 2009 Authors: Andrew Trotman
		22 January 2010, Vo Ngoc Anh and Alistair Moffat gave permission to BSD this code and derivitaves of it:

		> From: "Vo Ngoc ANH" <vo@csse.unimelb.edu.au>
		> To: "Andrew Trotman" <andrew@cs.otago.ac.nz>
		> Cc: "Alistair Moffat" <alistair@csse.unimelb.edu.au>; "andrew Trotman" <andrew.trotman@otago.ac.nz>
		> Sent: Friday, January 22, 2010 1:11 PM
		> Subject: Re: Carryover 12
		>
		>
		>
		> Hi Andrew,
		> Thank you for your interest. To tell the truth, I don't have enough
		> knowledge on the license matter. From my side, the answer is Yes. Please
		> let me know if I need to do anything for that.
		>
		> Cheers,
		> Anh.
		>
		> On Fri, 22 Jan 2010, Andrew Trotman wrote:
		>
		>> Hi,
		>>
		>> At ADCS I spoke biefly to Alistair about licenses for your source code to
		>> the Carryover 12 compression scheme.  I have a copy I downloaded shortly
		>> after you released it, and I've hacked it quite a bit.  The problem I
		>> have is that I want to release my hacked version using a BSD license and
		>> your code is GPL. The two licenses are not compatible.  I want to use the
		>> BSD license because my code includes other stuff that is already BSD
		>> (such as a hashing algorithm). As far as I know the only GPL code I'm
		>> using is yours.
		>>
		>> Could I please have a "special" license to release my derivitave of your
		>> code (and derivitaves of my derivitaves) under the BSD license.
		>>
		>> Thanks
		>> Andrew.

*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_CARRYOVER_12
		-----------------------------------
	*/
	/*!
		@brief Carryover-12 integer compression
		@details Similar to Simple-9, except that it uses the wasted bits between words.

		See:
			V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166
	*/
	class compress_integer_carryover_12 : public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_CARRYOVER_12::COMPRESS_INTEGER_CARRYOVER_12
				------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_carryover_12()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRYOVER_12::~COMPRESS_INTEGER_CARRYOVER_12
				------------------------------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~compress_integer_carryover_12()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRYOVER_12::ENCODE()
				---------------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);
			
			/*
				COMPRESS_INTEGER_CARRYOVER_12::DECODE()
				---------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);
			
			/*
				COMPRESS_INTEGER_CARRYOVER_12::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
