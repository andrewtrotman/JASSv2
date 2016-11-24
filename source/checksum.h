/*
	CHECKSUM.H
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Checksum routines
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include <string>

namespace JASS
	{
	/*
		CLASS CHECKSUM
		--------------
	*/
	/*!
		@brief Holder class for checksums, Cyclic Redundency Checks, and so on.
	*/
	class checksum
		{
		public:
			/*
				CHECKSUM::FLETCHER_16()
				-----------------------
			*/
			/*!
				@brief Compute the Fletcher 16-bit checksum of 8-bit data.  Unlike a "regular" 8-bit checsum, this manages position as well as content.
				@details  The Fletcher checksum was designed to identify when two bytes in an 8-bit data-stream are swapped over. which a "regular" checksum cannot detect.
				For details see: Fletcher, J. G. (1982). An Arithmetic Checksum for Serial Transmissions. IEEE Transactions on Communications. COM-30 (1): 247-252. doi:10.1109/tcom.1982.1095369
				The Wikipedia article is: https://en.wikipedia.org/wiki/Fletcher's_checksum
				Dr Dobbs has an article that examines how good the algorithm is: http://www.drdobbs.com/database/fletchers-checksum/184408761
				@param start [in] A pointer to a sequence of bytes to checksum.
				@param end [in] A pointer to the end of the buffer.
				@return The Fletcher 16-bit checksum of the 8-bit sequence.
			*/
			template <typename TYPE>
			static uint16_t fletcher_16(TYPE &start, TYPE &end);


			/*
				CHECKSUM::FLETCHER_16()
				-----------------------
			*/
			/*!
				@brief Compute the Fletcher 16-bit checksum of a string (see the template verison details)..
				@param data [in] A pointer to a sequence of bytes of length length to checksum.
				@param length [in] The number of bytes to checksum.
				@return The Fletcher 16-bit checksum of the 8-bit sequence.
			*/
			static uint16_t fletcher_16(const void *data, size_t length);
	
			/*
				CHECKSUM::FLETCHER_16()
				-----------------------
			*/
			/*!
				@brief Compute the Fletcher 16-bit checksum of a string (see the template verison details)..
				@param string [in] A C++ string to checksum.
				@return The Fletcher 16-bit checksum of the string.
			*/
			static uint16_t fletcher_16(const std::string &string)
				{
				return fletcher_16(string.c_str(), string.length());
				}

			/*
				CHECKSUM::FLETCHER_16()
				-----------------------
			*/
			/*!
				@brief Compute the Fletcher 16-bit checksum of an istream (see the template verison for details)..
				@param stream [in] The stream to checksum
				@return The Fletcher 16-bit checksum of the stream.
			*/
			static uint16_t fletcher_16(std::istream &stream);

			/*
				CHECKSUM::FLETCHER_16_FILE()
				----------------------------
			*/
			/*!
				@brief Compute the Fletcher 16-bit checksum of a disk file.
				@param filename [in] The path to the file to checksum.
				@return The Fletcher 16-bit checksum of the file.
			*/
			static uint16_t fletcher_16_file(const std::string &filename);

			/*
				CHECKSUM::UNITTEST()
				--------------------
			*/
			/*!
				@brief Unit test this class.  Checksums can be verified against:http://www.nitrxgen.net/hashgen/
			*/
			static void unittest(void);
		};

	}