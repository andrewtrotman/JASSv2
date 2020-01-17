/*
	CHANNEL_TREC.H
	--------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Input (not output) channel for turning TREC fopic files into JASS queries.
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/

#pragma once

#include <sstream>

#include "channel.h"

namespace JASS
	{
	/*
		CLASS CHANNEL_TREC
		------------------
		This channel reads from another channel assuming that that channel
		is a TREC topic file.  This allows the search engine to directly
		read TREC topic files without pre-processing.
	*/
	class channel_trec : public channel
		{
		private:
			std::unique_ptr<channel> in_channel;	///< The channel this channel reads from
			std::string buffer;							///< Storage of date read from the in_channel
			bool read;										///< Do we need to read from the input channel
			bool at_eof;									///< Are we at end of input?
			int64_t number;								///< The topic number of the topic being constructed
			std::string tag;								///< The tag set to use

		private:
			/*
				CHANNEL_TREC::CONSTRUCT()
				-------------------------
			*/
			/*!
				@brief Piece to geether the parts of the query to make the whole.
				@param query[out] The query once constructed.
				@param number [in] The query number (is put at the head of the query)
				@param text [in] The text of the query.
			*/
			void construct(std::string &query, size_t number, const std::string &&text);

		protected:
			/*
				CHANNEL_TREC::BLOCK_WRITE()
				---------------------------
			*/
			/*!
				@brief Not implemented (will cause program termination)
			*/
			virtual size_t block_write(const void *buffer, size_t length)
				{
				exit(printf("channel_trec::block_write not implemented (class only supports gets())"));
				}
				
			/*
				CHANNEL_TREC::BLOCK_READ()
				--------------------------
			*/
			/*!
				@brief Not implemented (will cause program termination)
			*/
			virtual size_t block_read(void *into, size_t length)
				{
				exit(printf("channel_trec::block_read not implemented (class only supports gets())"));
				}

		public:
			/*
				CHANNEL_TREC::CHANNEL_TREC()
				----------------------------
			*/
			/*!
				@brief Constructor
				@param in [in] the channel that this channel reads from (normally a file)
				@param tagset [in] a set combination of 't','d','n' (for title, desc, narr).
			*/
			channel_trec(std::unique_ptr<channel> &in, std::string tagset) :
				in_channel(std::move(in)),
				tag(tagset)
				{
				read = true;
				number = -1;
				at_eof = false;
				}

			/*
				CHANNEL_TREC::~CHANNEL_TREC()
				----------------------------
			*/
			virtual ~channel_trec()
				{
				/* Nothing */
				}

			/*
				CHANNEL_TREC::GETS()
				--------------------
			*/
			/*!
				@brief Read a '\n' terminated string from the channel into the parameter.
				@param into [out] Read into this string.
			*/
			virtual void gets(std::string &into);

			/*
				CHANNEL_TREC::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
	}
