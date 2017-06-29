/*
	CHANNEL_FILE.H
	--------------
*/
#pragma once

#include <stdio.h>

#include "channel.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS CHANNEL_FILE
		------------------
	*/
	class channel_file : public channel
	{
	private:
		std::string filename;
		FILE *outfile;
		instream *infile;
		bool eof;

	protected:
		virtual size_t block_write(const std::string &block);
		virtual void block_read(std::string &into);
		virtual bool getsz(const std::string into, char terminator = '\0');

	public:
		channel_file();
		channel_file(const std::string &filename);
		virtual ~channel_file();
	} ;
}
