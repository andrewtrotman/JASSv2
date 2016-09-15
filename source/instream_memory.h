/*
	INSTREAM_MEMORY.H
	-----------------
*/
#pragma once

#include <stdio.h>

#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_MEMORY
		---------------------
	*/
	class instream_memory : public instream
		{
		private:
			size_t bytes_read;
			const uint8_t *file;
			size_t file_length;

		public:
			instream_memory(const uint8_t *memory, size_t length):
				bytes_read(0),
				file(memory),
				file_length(length)
				{
				/*
					Nothing
				*/
				}

			virtual void read(document &buffer);
		};
	}