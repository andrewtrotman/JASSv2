/*
	CHANNEL.H
	---------
*/
#pragma once

#include <stdio.h>
#include <string.h>


namespace JASS
	{
	/*
		CLASS CHANNEL
		-------------
	*/
	class channel
	{
	protected:
		virtual long long block_write(char *source, long long length) = 0;		// returns bytes written
		virtual long long block_write(const char *source, long long length) { return block_write((char *)source, length); }
		virtual char *block_read(char *into, long long length) = 0;
		virtual char *getsz(char terminator = '\0') = 0;

	public:
		static const char endl = '\n';
		static const char ends = '\0';

	public:
		ANT_channel() {}
		virtual ~ANT_channel() {}

		long long write(char *source, long long length) 			{ return block_write(source, length); }
		long long write(unsigned char *source, long long length) { return write((char *)source, length); }

		long long write(char *source) 							{ return block_write(source, strlen(source)); }
		long long write(unsigned char *source) 					{ return write((char*)source); }

		long long write(char source) 							{ return block_write(&source, 1); }
		long long write(short source) 							{ return write((long long)source); }
		long long write(int source) 								{ return write((long long)source); }
		long long write(long source) 							{ return write((long long)source); }
		long long write(long long source) 						{ char buffer[32]; sprintf(buffer, "%lld", source); return write(buffer); }

		long long write(unsigned char source) 					{ return write((char)source); }
		long long write(unsigned short source) 					{ return write((unsigned long long)source); }
		long long write(unsigned int source) 					{ return write((unsigned long long)source); }
		long long write(unsigned long source) 					{ return write((unsigned long long)source); }
		long long write(unsigned long long source) 				{ char buffer[32]; sprintf(buffer, "%llu", source); return write(buffer); }

		char *read(char *destination, long long length) 	{ return block_read(destination, length); }
		char *gets(void) 									{ return getsz('\n'); }
		long long puts(char *string) 							{ return block_write(string, strlen(string)) < 0 ? 0 : block_write("\n", 1); }

		ANT_channel &operator<<(char source)				{ write(source); return *this; }
		ANT_channel &operator<<(short source)				{ write(source); return *this; }
		ANT_channel &operator<<(int source)					{ write(source); return *this; }
		ANT_channel &operator<<(long source)				{ write(source); return *this; }
		ANT_channel &operator<<(long long source)			{ write(source); return *this; }

		ANT_channel &operator<<(unsigned short source)		{ write(source); return *this; }
		ANT_channel &operator<<(unsigned int source)		{ write(source); return *this; }
		ANT_channel &operator<<(unsigned long source)		{ write(source); return *this; }
		ANT_channel &operator<<(unsigned long long source)	{ write(source); return *this; }

		ANT_channel &operator<<(char *source) 				{ write(source); return *this; }
		ANT_channel &operator<<(unsigned char *source) 		{ write(source); return *this; }
	} ;
}
