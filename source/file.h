/*
	FILE.H
	------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
/*!
	@file
	@brief Partial file and whole file based I/O methods.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#ifdef _MSC_VER
	#include <windows.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace JASS
	{
	/*
		CLASS FILE
		----------
	*/
	/*!
		@brief File based I/O methods including whole file and partial files.
		@details This class exists in order to abstract file I/O which has, in the past, been different on different platforms.  For example,
		64-bit file I/O under Windows (using Win32) is awkward because the Win32 functions do not take 64-bit parameters.
		
		This file class is based on Resource Allocation Is Initialisation (RAII).  That is, the file is opened when the object is constructed
		and closed when the object is destroyed.
	*/
	class file
		{
		public:
			/*
				CLASS FILE::FILE_READ_ONLY
				--------------------------
			*/
			/*!
				@brief A read_only file object, the memory was probably allocated with mmap() and needs deallocating accordingly
			*/
			class file_read_only
				{
				friend class file;
				private:
#ifdef _MSC_VER
					HANDLE hFile;							///< The file being mapped
					HANDLE hMapFile;						///< The mapping of that file
#endif
					const void *file_contents;								///< The contents of the file.
					size_t size;											///< The size of the file.

				public:
					/*
						FILE::FILE_READ_ONLY::FILE_READ_ONLY()
						--------------------------------------
					*/
					/*!
						@brief Constructor
					*/
					file_read_only():
						file_contents(nullptr),
						size(0)
						{
						/* Nothing */
						}

					/*
						FILE::FILE_READ_ONLY::OPEN()
						----------------------------
					*/
					/*!
						@brief Open and read the file into memory
						@param filename [in] The name of the file to read
						@return The size of the file
					*/
					size_t open(const std::string &filename);

					/*
						FILE::FILE_READ_ONLY::~FILE_READ_ONLY()
						---------------------------------------
					*/
					/*!
						@brief Destrucgtor
					*/
					~file_read_only();

					/*
						FILE::FILE_READ_ONLY::READ_ENTIRE_FILE()
						----------------------------------------
					*/
					/*!
						@brief Return the contents and length of the file.
						@param into [out] The pointer to write into.
						@return The size of the file in bytes
					*/
					size_t read_entire_file(const uint8_t *&into) const
						{
						into = reinterpret_cast<const uint8_t *>(file_contents);
						return size;
						}
				};

		protected:
			FILE *fp; 								///< The underlying representation is a FILE *  from C (as they appear to be fast).
			size_t file_position;				///< The ftell() position in the file.
			size_t buffer_size;					///< Size of the internal file buffering.
			size_t buffer_used;					///< How much of the internal file buffer is being used.
			std::unique_ptr<uint8_t []> buffer;	///< Internal file buffer
			size_t bytes_written;				///< Number of bytes written to this file.
			size_t bytes_read;					///< Number of bytes read from this file.

		public:
			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor
			*/
			file() = delete;

			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor with a C FILE * object
				@param fp [in] The FILE * object this object should use.  This class takes ownership and closes the file on destruction.
			*/
			file(FILE *fp) :
				fp(fp),
				file_position(0),
				buffer_size(10 * 1024 * 1024),					// start with a buffer of this size
				buffer_used(0),
				buffer(std::make_unique<uint8_t []>(buffer_size)),
				bytes_written(0),
				bytes_read(0)
				{
				/* Nothing */
				}

			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor used for opening files.
				@param filename [in] the name of the file.
				@param mode [in] The file open mode.  See C's fopen() for details on possible modes.
			*/
			file(const char *filename, const char *mode) :
				file(nullptr)
				{
				/*
					Open the given file in the given mode
				*/
				#if defined(__STDC_LIB_EXT1__)
					fopen_s(&fp, filename, mode);
				#else
					fp = fopen(filename, mode);
				#endif
				}

			/*
				FILE::FILE()
				------------
			*/
			/*!
				@brief Constructor used for opening files.
				@param filename [in] the name of the file.
				@param mode [in] The file open mode.  See C's fopen() for details on possible modes.
			*/
			file(const std::string &filename, const std::string &mode) :
				file(filename.c_str(), mode.c_str())
				{
				/* Nothing */
				}

			/*
				FILE::~FILE()
				-------------
			*/
			/*!
				@brief Destructor
			*/
			~file()
				{
				flush();
				if (fp != nullptr && fp != stdin && fp != stdout && fp != stderr)
					fclose(fp);
				}

			/*
				FILE::SETVBUF()
				---------------
			*/
			/*!
				@brief change the size of the internal buffer (does not flush() first)
				@details The C standard states "This function should be called once the stream
				has been associated with an open file, but before any input or output operation
				is performed with it", and this is true of this function too.
				@param size [in] The new size of the buffer.
				@return 1 on success, 0 on failure.

			*/
			size_t setvbuf(size_t size)
				{
				buffer_size = size;
				buffer = std::make_unique<uint8_t []>(buffer_size);
				return buffer == NULL ? 0 : 1;
				}

			/*
				FILE::FLUSH()
				-------------
			*/
			/*!
				@brief Flush the internal buffers to disk (called automatically on close).
			*/
			void flush(void)
				{
				if (buffer_used > 0)
					{
					/*
						All physical output to the disk happens with this one line of code.  If this is
						done with blocking I/O then this causes a bottleneck as we wait for the OS
						to write to disk.
					*/
					::fwrite(buffer.get(), 1, buffer_used, fp);
					buffer_used = 0;
					}
				}

			/*
				FILE::READ()
				------------
			*/
			/*!
				@brief Read bytes number of bytes from the give file into the buffer.
				@param data [out] Buffer large enough to hold bytes number of bytes of data which are written into the memory pointed to by buffer.
				@param size [in] The number of bytes of data to read.
				@return The number of bytes of data that were read and written into buffer.
			*/
			size_t read(void *data, size_t size)
				{
				/*
					Keep track of the number of bytes we've been asked to write
				*/
				bytes_read += size;

				/*
					Flush the write cache
				*/
				flush();

				/*
					Take note of the new file position (at the end of the read)
				*/
				file_position += size;		// this is where we'll be at the end of the read

				/*
					And now perform the read
				*/
				return ::fread(data, 1, size, fp);
				}

			/*
				FILE::READ()
				------------
			*/
			/*!
				@brief Read buffer.size() bytes from the give file into the buffer.  If at end of file then this method will resize buffer to the number of bytes read from the file.
				@param buffer [in, out] Read buffer.size() bytes into buffer, calling buffer.resize() on failure.
			*/
			void read(std::vector<uint8_t> &buffer)
				{
				/*
					Read from the file
				*/
				size_t bytes_read = read(&buffer[0], buffer.size());

				/*
					If we got a short read then resize the buffer to signal back to the caller that we failed to read (probably EOF).
				*/
				if (bytes_read == 0)
					buffer.resize(0);
				else if (bytes_read != buffer.size())
					buffer.resize(bytes_read);
				}

			/*
				FILE::WRITE()
				-------------
			*/
			/*!
				@brief Write bytes number of bytes to the give file at the current cursor position.
				@param data [in] the byte sequence to write.
				@param size [in] The number of bytes of data to write.
				@return The number of bytes of data that were written to the file.
			*/
			size_t write(const void *data, size_t size)
				{
				uint8_t *from;
				size_t block_size;

				/*
					Keep track of the total number of bytes we've been asked to write to the file
				*/
				bytes_written += size;

				/*
					Update the file pointer
				*/
				file_position += size;

				if (buffer_used + size < buffer_size)
					{
					/*
						The data fits in the internal buffers
					*/
					memcpy(buffer.get() + buffer_used, data, (size_t)size);
					buffer_used += size;
					}
				else
					{
					/*
						The data does not fit in the internal buffers so it is
						necessary to flush the buffers and then do the write
					*/
					from = (uint8_t *)data;
					do
						{
						flush();
						block_size = size <= buffer_size ? size : buffer_size;
						memcpy(buffer.get(), from, (size_t)block_size);
						buffer_used += block_size;
						from += block_size;
						size -= block_size;
						}
					while (size > 0);
					}
				return 1;
				}

			/*
				FILE::WRITE()
				-------------
			*/
			/*!
				@brief Write bytes number of bytes to the give file at the current cursor position.
				@param buffer [in] the byte sequence to write.
				@return The number of bytes of data that were written to the file.
			*/
			size_t write(const std::string &buffer)
				{
				return write(buffer.c_str(), buffer.size());
				}

			/*
				FILE::SIZE()
				------------
			*/
			/*!
				@brief Return the length of the file as it currently stands
				@return File size in bytes.  0 is returned either on error or non-existant file (or 0-length file).
			*/
			size_t size(void) const;

			/*
				FILE::TELL()
				------------
			*/
			/*!
				@brief Return the byte offset of the file pointer in the current file.
				@return byte offset.  0 is returned either on error or non-existant file (or actually at 0).
			*/
			size_t tell(void)
				{
				return file_position;
				}

			/*
				FILE::SEEK()
				------------
			*/
			/*!
				@brief Seek to the given offset in the file.
				@details Throws std::out_of_range in the unlikely event of an error.
				@param offset [in] The location to seek to.
			*/
			void seek(size_t offset)
				{
				/*
					Empty the write buffer
				*/
				flush();

				/*
					Seek
				*/
				#ifdef _MSC_VER
					auto error = _fseeki64(fp, offset, SEEK_SET);
				#else
					auto error = fseeko(fp, offset, SEEK_SET);
				#endif

				/*
					Store the file file position
				*/
				file_position = offset;

				/*
					if error is non-zero then seek failed - which if a fatal error.
				*/
				if (error != 0)
					throw std::out_of_range("file::seek() failure");		// LCOV_EXCL_LINE	// This is highly unlikely to happen - its not clear why seek() can fail.
				}

			/*
				FILE::READ_ENTIRE_FILE()
				------------------------
			*/
			/*!
				@brief Read the contents of file filename into the std::string into.
				@details Because into is a string it is naturally '\0' terminated by the C++ std::string class.
				@param filename [in] The path of the file to read.
				@param into [out] The std::string to write into.  This string will be re-sized to the size of the file.
				@return The size of the file in bytes
			*/
			static size_t read_entire_file(const std::string &filename, std::string &into);

			/*
				FILE::READ_ENTIRE_FILE()
				------------------------
			*/
			/*!
				@brief Read the contents of file filename into the std::string into.
				@details Because into is a string it is naturally '\0' terminated by the C++ std::string class.
				@param filename [in] The path of the file to read.
				@param into [out] The std::string to write into.  This string will be re-sized to the size of the file.
				@return The size of the file in bytes
			*/
			static size_t read_entire_file(const std::string &filename, file_read_only &into)
				{
				return into.open(filename);
				}

			/*
				FILE::WRITE_ENTIRE_FILE()
				-------------------------
			*/
			/*!
				@brief Write the contents of buffer to the file specified in filenane.
				@details If the file does not exist it is created.  If it does already exist it is overwritten.
				@param filename [in] The path of the file to write to.
				@param buffer [in] The data to write to the file.
				@return True if successful, false if unsuccessful
			*/
			static bool write_entire_file(const std::string &filename, const std::string &buffer);
			
			/*
				FILE::BUFFER_TO_LIST()
				----------------------
			*/
			/*!
				@brief Turn a single std::string into a vector of uint8_t * (i.e. "C" strings).
				@details Note that these pointers are in-place.  That is,
				they point into buffer so any change to the uint8_t or to buffer effect each other.  This method removes blank lines from buffer and
				changes buffer by inserting '\0' characters at the end of each line.
				@param line_list [out] The vector to write into
				@param buffer [in, out] the string to decompose
			*/
			static void buffer_to_list(std::vector<uint8_t *> &line_list, std::string &buffer);
		
			/*
				FILE::IS_DIRECTORY()
				--------------------
			*/
			/*!
				@brief Determines whether the given file system object is a directoy or not.
				@param filename [in] The path to the file system object to check.
				@return True if the given path is a directory, false if it is not (or does not exist).
			*/
			static bool is_directory(const std::string &filename);

			/*
				FILE::MKSTEMP()
				---------------
			*/
			/*!
				@brief Generate a temporary filename containing the given prefix
				@details This method is a wrapper for mkstemp on Linux / MacOS and _mktemp on windows.  These methods are renounds for
				having the problem that the filename may not be unique once it has been generated and therefore opening a file with this
				name may fail.  However, there are times when this doesn't matter (such as unit tests).
				@param prefix [in] The prefix to the unique filenane,
				@return A unique filename at the time the method is called.
			*/
			static std::string mkstemp(std::string prefix);

			/*
				FILE::UNITTEST()
				----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
	}
