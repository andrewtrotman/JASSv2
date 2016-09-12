/*
	INSTREAM.H
	----------
*/
#pragma once

#include <vector>

#include "allocator.h"

namespace JASS
	{
	/*
		CLASS INSTREAM()
		----------------
	*/
	/*!
		@brief
	*/
	class instream
		{
		protected:
			allocator &memory;
			instream *source;

		public:
			instream(allocator &memory, instream *source = NULL) :
				memory(memory),
				source(source)
				{
				}
			
			virtual ~instream()
				{
				delete source;
				}

			virtual void read(std::vector<uint8_t> &buffer)
				{
				/*
					Nothing.
				*/
				}
		} ;
	}

