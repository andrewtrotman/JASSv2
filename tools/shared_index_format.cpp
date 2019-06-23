/*
	SHARED_INDEX_FORMAT.CPP
	-----------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@brief Convert Jimmy Lin's shared index format into a JASS index.
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

	This program, reads the index directly rather than using protobuf.

	For details of the encoding see: https://developers.google.com/protocol-buffers/docs/encoding
*/
#include <stdint.h>

#include <iostream>

#include "file.h"
#include "forceinline.h"


/*
	ENUM PROTOPUB_TYPE
	------------------
*/
enum protopub_type
	{
	VARINT = 0,
	SIXTY_FOUR_BIT = 1,
	BLOB = 2,
	GROUP_START = 3,
	GROUP_END = 4,
	THIRTY_TWO_BIT = 5
	};

/*
	PROTOPUB_TYPE_NAME()
	--------------------
*/
const char *protopub_type_name(protopub_type type)
	{
	switch (type)
		{
		case VARINT:
			return "int";
		case SIXTY_FOUR_BIT:
			return "64-bit";
		case BLOB:
			return "blob";
		case GROUP_START:
			return "DEPRECATED";
		case GROUP_END:
			return "DEPRECATED";
		case THIRTY_TWO_BIT:
			return "32-bit";
		default:
			return "ERROR";
		}
	}

/*
	UN_ZIGZAG()
	------------
*/
int64_t un_zigzag(uint64_t bits)
	{
	return (int64_t)((bits >> 1) ^ -(bits & 0x1));
	}

/*
	GET_UINT64_T()
	--------------
*/
uint64_t get_uint64_t(uint8_t *&stream)
	{
	uint64_t got;
	uint64_t next;
	uint8_t shift = 0;

	if (((got = *stream++) & 0x80) == 0)
		{
printf("[%x]", (int)got);
		return got;
		}

printf("[%x]", (int)got);

	got &= 0x7F;

	while (1)
		{
		shift += 7;
		if (((next = *stream++) & 0x80) == 0)
			{
printf("[%x]", (int)next);
			return got | next << shift;
			}
printf("[%x]", (int)next);
		got |= (next & 0x7F) << shift;
		}

	return 0;
	}

/*
	GET_INT64_T()
	-------------
*/
forceinline uint64_t get_int64_t(uint8_t *&stream)
	{
	return un_zigzag(get_uint64_t(stream));
	}

/*
	GET_UINT32_T()
	--------------
*/
forceinline uint32_t get_uint32_t(uint8_t *&stream)
	{
	return (uint32_t)un_zigzag(get_uint64_t(stream));
	}

/*
	GET_INT32_T()
	-------------
*/
forceinline int32_t get_int32_t(uint8_t *&stream)
	{
	return (int32_t)un_zigzag(get_uint64_t(stream));
	}

/*
	GET_TYPE_AND_FIELD()
	--------------------
*/
uint8_t get_type_and_field(protopub_type &type, uint8_t *&stream)
	{
	uint8_t encoding = *stream++;

printf("%x->", encoding);

	type = (protopub_type)(encoding & 0x07);

	return encoding >> 3;
	}

/*
	GET_BLOB()
	----------
*/
std::string decode_blob(size_t &length, uint8_t *&stream)
	{
	length = get_uint64_t(stream);
	auto at = stream;
	stream += length;

	return std::string((char *)at, (size_t)length);
	}

/*
	GET_NEXT()
	----------
*/
void get_next(uint8_t *&stream)
	{
	protopub_type type;
	uint8_t field = get_type_and_field(type, stream);
	printf("%s field_%d: ", protopub_type_name(type), field);

	switch (type)
		{
		case VARINT:
			{
			uint64_t value = get_uint64_t(stream);
			std::cout << "(int) " << value << "\n";
			break;
			}
		case BLOB:
			{
			size_t length;
			std::string term = decode_blob(length, stream);

			if (field == 1)
				std::cout << "(Blob:" << length << ")\"" << term << "\"\n";
			else
				{
				std::cout << "(Blob:" << length << ")\n";

				uint8_t *here = (uint8_t *)&term[0];
				uint8_t *end = here + length;
				while (here < end)
					get_next(here);
				}
			break;
			}
		case THIRTY_TWO_BIT:
		case SIXTY_FOUR_BIT:
		case GROUP_START:
		case GROUP_END:
		default:
			std::cout << "ERROR\n";
			break;
		}
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::string file;
	size_t file_size = JASS::file::read_entire_file(argv[1], file);

	uint8_t *stream = (uint8_t *)&file[0];

	int64_t first = get_uint64_t(stream);
	printf("%d-byte chunk\n", (int)first);
	uint8_t *end = stream + first;

	while (stream < end)
		get_next(stream);

	first = get_uint64_t(stream);
	printf("%d-byte chunk\n", (int)first);
	get_next(stream);
	get_next(stream);
	get_next(stream);
	get_next(stream);

	return 0;
	}
