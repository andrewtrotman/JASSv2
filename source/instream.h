/*
	INSTREAM.H
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Base class for reading data from some input source.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <vector>

#include "document.h"
#include "allocator.h"

namespace JASS
	{
	/*
		CLASS INSTREAM()
		----------------
	*/
	/*!
		@brief Read data from an input stream.
		@details This is the abstract base class for reading data from an input source.  If the indexer, for example, needs to read from a file
		then an instance of a subclass of this class can be used and once created the user need not know where the data is coming from.  Its an
		abstraction over input streams of a generic interface to get data. 
		
		There are two "kinds" of these objects.  Ones that read from a stream such as a file, and ones that generate documents ready for indexing.
		They share the same interface so that its possible to chain them together to form pipelines such as read_file | de-zip | de-tar | index.
		
		The constructor of a complex pipeline does not want to keep track of each and every pointer to parts of the stream - and to free them on
		competion so this object deletes the predecessor in the pipeline if deleted.  This propegates down the pipeline which is eventially cleaned
		up bottom up.
	*/
	class instream
		{
		protected:
			allocator *memory;				///< Any and all memory allocation must happen using this object.
			instream *source;					///< If this object is reading from another instream then this is that instream.

		public:
			/*
				INSTREAM::INSTREAM()
				--------------------
			*/
			/*!
				@brief Constructor.
				@param memory [in] If this object needs to allocate memory (for example, a buffer) then it should be allocated from this pool.
				@param source [in] This object reads data from source before processing and passingin via read().
			*/
			instream(allocator *memory = nullptr, instream *source = nullptr) :
				memory(memory),				// store the memory pointer (which this object does not free on deletion)
				source(source)					// store the instream (which this object does free on deletion)
				{
				}
			/*
				INSTREAM::~INSTREAM()
				---------------------
			*/
			/*!
				@brief Destructor.
				@details This destructor not only cleans up this object but also any object that is earlier in the pipeline - so a deletion of the root
				of the pipeline will delete the entire pipeline.
			*/
			virtual ~instream()
				{
				delete source;
				}
	
			/*
				INSTREAM::READ()
				----------------
			*/
			/*!
				@brief Read buffer.size() bytes of data into buffer, resizing buffer on eof.
				@param buffer [out] buffer.size() bytes of data are read from source into buffer which is resized to the number of bytes read on eof.
			*/
			virtual void read(document &buffer) = 0;
		} ;
	}

