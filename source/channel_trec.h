/*
	CHANNEL_TREC.H
	--------------
*/
#pragma once

#include <sstream>

#include "channel.h"
//#include "stop_word.h"

namespace JASS
	{
	/*
		CLASS CHANNEL_TREC
		------------------
		This channel reads from another channel assuming that that channel
		is a TREC topic file.  This allows the search engine to directly
		read TREC topic files without pre-processing.

		The out channel is what-ever is passed to the constructor of this object
	*/
	class channel_trec : public channel
		{
		private:
			channel &in_channel;				///< The channel this channel reads from
			std::string buffer;				///< Storage of date read from the in_channel
			long read;
			long at_eof;
			long number;
			std::string tag;					///< The tag set to use

		private:
			/*
				CHANNEL_TREC::CLEAN()
				---------------------
			*/
			/*!
				@brief Parse the query using the document parser and turn into searchable tokens.
				@param clean_query[out] The query once cleaned.
				@param number [in] The query number (is put at the head of the clean query)
				@param raw_query [in] The query that should be cleaned.
			*/
			void clean(std::string &clean_query, size_t number, const std::string &&raw_query);

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
				exit(printf("channel_trec::block_read not implemented (class only supports gets())"));
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
				exit(printf("channel_trec::block_write not implemented (class only supports gets())"));
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
			channel_trec(channel &in, std::string tagset) :
				in_channel(in),
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
