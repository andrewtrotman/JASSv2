/*
	PROTOBUF.H
	----------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Hand crafted methods to read protocol buffer (protobuf) encoded files.
	@details For details of the encoding see: https://developers.google.com/protocol-buffers/docs/encoding
*/
#pragma once

#include <stdint.h>
#include "slice.h"

namespace JASS
	{
	/*
		CLASS PROTOBUF
		--------------
	*/
	/*!
		@brief Functions to read a protobuf buffer
		@details For details of the encoding see: https://developers.google.com/protocol-buffers/docs/encoding
		This class is written to be standalone from JASSv2 so that others can use it without including all of JASSv2
	*/
	class protobuf
		{
		public:
			/*
				ENUM PROTOBUF::WIRE_TYPE
				------------------------
			*/
			/*!
				@enum wire_type
				@brief The known protobuf types for protobuf 3.
			*/
			enum wire_type
				{
				VARINT = 0,					///< Variable byte encoded integer of up-to 64 bits, signed or unsigned
				SIXTY_FOUR_BIT = 1,		///< Little endian 64-bit number, integer or floating point types
				BLOB = 2,					///< Blob or string.  length VARINT then data
				GROUP_START = 3,			///< Depricated in protobuf 3
				GROUP_END = 4,				///< Depricated in protobuf 3
				THIRTY_TWO_BIT = 5		///< Little endian 32-bit number, integer or floating point types
				};

		private:
			/*
				PROTOBUF::UN_ZIGZAG()
				---------------------
			*/
			/*!
				@brief Turn unsigned ZigZaged integer into a signed integer.
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"ZigZag encoding maps signed integers to unsigned integers so that numbers with a small absolute
				value (for instance, -1) have a small varint encoded value too. It does this in a way that "zig-zags"
				back and forth through the positive and negative integers, so that -1 is encoded as 1, 1 is encodedas
				2, -2 is encoded as 3, and so on". This method undoes the encoding.
				@param bits [in] The ZigZag encoding to convert
				@return The signed integer
			*/
			inline static int64_t un_zigzag(uint64_t bits)
				{
				return (int64_t)(bits >> 1) ^ -((int64_t)(bits & 0x1));
				}

		public:
			/*
				PROTOBUF::GET_UINT64_T()
				------------------------
			*/
			/*
				@brief Read and decode an unsigned 64-bit VARINT integer.
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"Each byte in a varint, except the last byte, has the most significant bit (msb) set - this indicates
				that there are further bytes to come. The lower 7 bits of each byte are used to store the two's complement
				representation of the number in groups of 7 bits, least significant group first."
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			uint64_t static get_uint64_t(const uint8_t *&stream)
				{
				uint64_t got;
				uint64_t next;
				uint8_t shift = 0;

				if (((got = *stream++) & 0x80) == 0)
					return got;

				got &= 0x7F;

				while (1)
					{
					shift += 7;
					if (((next = *stream++) & 0x80) == 0)
						return got | next << shift;
					got |= (next & 0x7F) << shift;
					}

				return got;
				}

			/*
				PROTOBUF::GET_INT64_T()
				-----------------------
			*/
			/*
				@brief Read and decode a signed 64-bit VARINT integer.
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static uint64_t get_int64_t(const uint8_t *&stream)
				{
				return un_zigzag(get_uint64_t(stream));
				}

			/*
				PROTOBUF::GET_UINT32_T()
				------------------------
			*/
			/*
				@brief Read and decode an unsigned 32-bit VARINT integer.
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static uint32_t get_uint32_t(const uint8_t *&stream)
				{
				return static_cast<uint32_t>(un_zigzag(get_uint64_t(stream)));
				}

			/*
				PROTOBUF::GET_INT32_T()
				-----------------------
			*/
			/*
				@brief Read and decode a signed 32-bit VARINT integer.
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static int32_t get_int32_t(const uint8_t *&stream)
				{
				return static_cast<int32_t>(un_zigzag(get_uint64_t(stream)));
				}

			/*
				PROTOBUF::GET_BLOB()
				--------------------
			*/
			/*
				@brief Read and decode a blob or string.
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"The value is a varint encoded length followed by the specified number of bytes of data."
				@param stream [in,out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return A slice of stream containing the blob
			*/
			inline static slice get_blob(const uint8_t *&stream)
				{
				size_t length = get_uint64_t(stream);
				const uint8_t *at = &stream[0];
				stream += length;

				return slice(const_cast<uint8_t *>(at), length);
				}

			/*
				PROTOBUF::GET_64_T()
				--------------------
			*/
			/*
				@brief Read and decode a 64-bit number.
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"Non-varint numeric types are simple – double and fixed64 have wire type 1, which tells the parser to expect a
				fixed 64-bit lump of data" and "The values are stored in little-endian byte order."
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static uint64_t get_64_t(const uint8_t *&stream)
			{
			uint64_t answer;

			answer = static_cast<uint64_t>(*stream++) << 0;
			answer |= static_cast<uint64_t>(*stream++) << 8;
			answer |= static_cast<uint64_t>(*stream++) << 16;
			answer |= static_cast<uint64_t>(*stream++) << 24;
			answer |= static_cast<uint64_t>(*stream++) << 32;
			answer |= static_cast<uint64_t>(*stream++) << 40;
			answer |= static_cast<uint64_t>(*stream++) << 48;
			answer |= static_cast<uint64_t>(*stream++) << 56;

			return answer;
			}

			/*
				PROTOBUF::GET_32_T()
				--------------------
			*/
			/*
				@brief Read and decode a 64-bit number.
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"Non-varint numeric types are simple" ... "float and fixed32 have wire type 5, which tells it to expect 32 bits"
				and "The values are stored in little-endian byte order."
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static uint32_t get_32_t(const uint8_t *&stream)
			{
			uint32_t answer;

			answer = static_cast<uint32_t>(*stream++) << 0;
			answer |= static_cast<uint32_t>(*stream++) << 8;
			answer |= static_cast<uint32_t>(*stream++) << 16;
			answer |= static_cast<uint32_t>(*stream++) << 24;

			return answer;
			}

			/*
				PROTOBUF::GET_DOUBLE()
				----------------------
			*/
			/*
				@brief Read and decode a double .
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"Non-varint numeric types are simple – double and fixed64 have wire type 1, which tells the parser to expect a fixed 64-bit lump of data"
				and "the values are stored in little-endian byte order."
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The decoded integer
			*/
			inline static double get_double(const uint8_t *&stream)
			{
			union
				{
				uint64_t byte_sequence;
				double number;
				} answer;

			answer.byte_sequence = get_64_t(stream);
			return answer.number;
			}

			/*
				PROTOBUF::GET_TYPE_AND_FIELD()
				------------------------------
			*/
			/*!
				@brief Extract the field number and its type from the stream
				@details The spec (https://developers.google.com/protocol-buffers/docs/encoding) states:
				"Each key in the streamed message is a varint with the value (field_number << 3) | wire_type" ...
				"in other words, the last three bits of the number store the wire type.".
				@param type [out] The type of the field
				@param stream [in, out] A reference to a pointer to the start of the value, points to the next byte on return.
				@return The number of the field
			*/
			static uint8_t get_type_and_field(wire_type &type, const uint8_t *&stream)
				{
				uint8_t encoding = *stream++;

				type = static_cast<wire_type>(encoding & 0x07);

				return encoding >> 3;
				}
		} ;
	}
