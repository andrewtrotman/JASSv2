/*
	FILE.CPP
	--------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
	#include <io.h>
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif
#include <limits>

#include "file.h"
#include "asserts.h"

namespace JASS
	{

	/*
		FILE::FILE_READ_ONLY::OPEN()
		----------------------------
	*/
	size_t file::file_read_only::open(const std::string &filename)
		{
		#ifdef _MSC_VER
			hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				return 0;

			hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
			if (hMapFile == NULL)
				{
				CloseHandle(hFile);
				return 0;
				}

			void *lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
			if (lpMapAddress == NULL)
				{
				CloseHandle(hFile);
				CloseHandle(hMapFile);
				return 0;
				}

			file_contents = (uint8_t *)lpMapAddress;

			DWORD high;
			DWORD low = GetFileSize(hFile, &high);

			size = ((uint64_t)high << (uint64_t)32) + (uint64_t)low;

			return size;
		#else
			/*
				Open the file
			*/
			int reader;

			if ((reader = ::open(filename.c_str(), O_RDONLY)) < 0)
				return 0;

			/*
				Find out how large it is
			*/
			struct stat statistics;
			if (fstat(reader, &statistics) != 0)
				{
				close(reader);
				return 0;
				}

			/*
				Allocate space for it and load it
			*/
			#ifdef __APPLE__
				file_contents = (uint8_t *)mmap(nullptr, statistics.st_size, PROT_READ, MAP_PRIVATE, reader, 0);
			#else
				file_contents = (uint8_t *)mmap(nullptr, statistics.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, reader, 0);
			#endif

			/*
				Close the file
			*/
			close(reader);

			if (file_contents == nullptr)
				return 0;

			/*
				Remember the file size
			*/
			size = statistics.st_size;

			return size;
		#endif
		}

	/*
		FILE::FILE_READ_ONLY::~FILE_READ_ONLY()
		---------------------------------------
	*/
	file::file_read_only::~file_read_only()
		{
		#ifdef _MSC_VER
			UnmapViewOfFile((void *)file_contents);
			CloseHandle(hMapFile); // close the file mapping object
			CloseHandle(hFile);   // close the file itself
		#else
			munmap((void *)file_contents, size);
		#endif
		}


	/*
		FILE::READ_ENTIRE_FILE()
		------------------------
		This uses a combination of "C" FILE I/O and C++ strings in order to copy the contents of a file into an internal buffer.
		There are many different ways to do this, but this is the fastest according to this link: http://insanecoding.blogspot.co.nz/2011/11/how-to-read-in-file-in-c.html
		Note that there does not appear to be a way in C++ to avoid the initialisation of the string buffer.
		
		Returns the length of the file in bytes - which is also the size of the string buffer once read.
	*/
		size_t file::read_entire_file(const std::string &filename, std::string &into)
		{
		FILE *fp;		
		// "C" pointer to the file
#ifdef _MSC_VER
		struct __stat64 details;				// file system's details of the file
#else
		struct stat details;				// file system's details of the file
#endif
		size_t file_length = 0;			// length of the file in bytes

		/*
			Fopen() the file then fstat() it.  The alternative is to stat() then fopen() - but that is wrong because the file might change between the two calls.
		*/
		if ((fp = fopen(filename.c_str(), "rb")) != nullptr)
			{
#ifdef _MSC_VER
			if (_fstat64(fileno(fp), &details) == 0)
#else
			if (fstat(fileno(fp), &details) == 0)
#endif
				if ((file_length = details.st_size) != 0)
					{
					into.resize(file_length);
					if (fread(&into[0], details.st_size, 1, fp) != 1)
						into.resize(0);				// LCOV_EXCL_LINE	// happens when reading the file_size buyes failes (i.e. disk or file failure).
					}
			fclose(fp);
			}

		return file_length;
		}

	/*
		FILE::WRITE_ENTIRE_FILE()
		-------------------------
		Uses "C" file I/O to write the contents of buffer to the given names file.
		
		Returns true on success, else false.
	*/
	bool file::write_entire_file(const std::string &filename, const std::string &buffer)
		{
		FILE *fp;						// "C" file to write to

		if ((fp = fopen(filename.c_str(), "wb")) == nullptr)
			return false;

		size_t success = fwrite(&buffer[0], buffer.size(), 1, fp);

		fclose(fp);

		return success == 1 ? true : false;
		}

	/*
		FILE::BUFFER_TO_LIST()
		----------------------
		Turn a single std::string into a vector of uint8_t * (i.e. "C" Strings). Note that these pointers are in-place.  That is,
		they point into buffer so any change to the uint8_t or to buffer effect each other.
		
		Note: This method removes blank lines from the input file.
	*/
	void file::buffer_to_list(std::vector<uint8_t *> &line_list, std::string &buffer)
		{
		uint8_t *pos;
		size_t line_count = 0;

		/*
			Walk the buffer counting how many lines we think are in there.
		*/
		pos = (uint8_t *)&buffer[0];
		while (*pos != '\0')
			{
			if (*pos == '\n' || *pos == '\r')
				{
				/*
					a seperate line is a consequative set of '\n' or '\r' lines.  That is, it removes blank lines from the input file.
				*/
				while (*pos == '\n' || *pos == '\r')
					pos++;
				line_count++;
				}
			else
				pos++;
			}

		/*
			resize the vector to the right size, but first clear it.
		*/
		line_list.clear();
		line_list.reserve(line_count);

		/*
			Now rewalk the buffer turning it into a vector of lines
		*/
		pos = (uint8_t *)&buffer[0];
		if (*pos != '\n' && *pos != '\r' && *pos != '\0')
			line_list.push_back(pos);
		while (*pos != '\0')
			{
			if (*pos == '\n' || *pos == '\r')
				{
				*pos++ = '\0';
				/*
					a seperate line is a consequative set of '\n' or '\r' lines.  That is, it removes blank lines from the input file.
				*/
				while (*pos == '\n' || *pos == '\r')
					pos++;
				if (*pos != '\0')
					line_list.push_back(pos);
				}
			else
				pos++;
			}
		}

	/*
		FILE::IS_DIRECTORY()
		--------------------
		Determines whether the given file system object is a directoy or not.
	
		Returns true if filename is a directory, else returns false.
	*/
	bool file::is_directory(const std::string &filename)
		{
		#ifdef WIN32
			struct __stat64 st;				// file system details

			if (_stat64(filename.c_str(), &st) == 0)
				return (st.st_mode & _S_IFDIR) == 0 ? false : true;		// check the _S_IFDIR flag as there is no S_ISDIR() on Windows
			return false;
		#else
			struct stat st;				// file system details

			if (stat(filename.c_str(), &st) == 0)
					return S_ISDIR(st.st_mode);		// simply check the S_ISDIR() flag
			return false;
		#endif
		}

	/*
		FILE::SIZE()
		------------
	*/
	size_t file::size(void) const
		{
		/*
			If we're standard in (stdin) then the file is of infinite length
		*/
		if (fp == stdin)
			return (std::numeric_limits<size_t>::max)();

		/*
			If we don't exist then we must be 0 in size
		*/
		if (fp == nullptr)
			return 0;
		/*
			Since we already have a handle to the file, we just remember where we are,
			seek to the end and check where that is, and seek back.  This will probably
			be very fast as it doesn't (normally) need to do and I/O to compute the answer
		*/
		#ifdef WIN32
			int64_t current_position = _ftelli64(fp);
			if (current_position < 0)
				return 0;							// this only happens on _ftelli64() failing
			if (_fseeki64(fp, 0, SEEK_END) < 0)
				return 0;
			int64_t file_size = _ftelli64(fp);
			if (_fseeki64(fp, current_position, SEEK_SET) < 0)
				return 0;
		#else
			off_t current_position = ftello(fp);
			if (current_position < 0)
				return 0;							// LCOV_EXCL_LINE // this only happens on ftello() failing
			if (fseeko(fp, 0, SEEK_END) < 0)
				return 0;							// LCOV_EXCL_LINE	// when seek fails
			off_t file_size = ftello(fp);
			if (fseeko(fp, current_position, SEEK_SET) < 0)
				return 0;							// LCOV_EXCL_LINE	// seek has failed.
		#endif
		
		/*
			This will fail in the case where off_t is larger than a size_t.  This is unlikely.
			On the machines this is being developed on both size_t and off_t are 8-byte integers.
		*/
		return file_size < 0 ? 0 : file_size;
		}
	
	/*
		FILE::MKSTEMP()
		---------------
	*/
	std::string file::mkstemp(std::string prefix)
		{
		prefix = prefix + "XXXXXX";
		#ifdef WIN32
		auto filename = const_cast<char *>(prefix.c_str());
			::_mktemp(filename);
		#else
			::umask(::umask(0));				// This sets the umask to its current value, and prevents Coverity from producing a warning
			int file_descriptor = ::mkstemp(const_cast<char *>(prefix.c_str()));
			if (file_descriptor >= 0)
				close(file_descriptor);
		#endif
		
		return std::string(prefix.c_str());
		}


	/*
		FILE::UNITTEST()
		----------------
	*/
	void file::unittest(void)
		{
		std::vector<uint8_t *> lines;
		std::string example_file;
		std::string reread;

		/*
			CHECK IS_DIRECTORY()
		*/
		/*
			Dot must be a directory (on Linux and Windows and OS X)
		*/
		JASS_assert(is_directory("."));
		JASS_assert(!is_directory(".JASS."));		// should fail on a file that doesn't exist (but this might, no easy way to check).
		
		/*
			something we know is not a directory.  In this case we'll use this very file.  Yes, this assumes
			the unit tests are not run when the source code is not available - but I think that's reasonable.
		*/
		JASS_assert(!is_directory(__FILE__));

		/*
			CHECK WRITE_ENTIRE_FILE() then READ_ENTIRE_FILE()
		*/
		example_file = "text for example file";			// sample to be written and read back
		
		/*
			create a temporary filename.  There doesn't appear to be a clean way of doing this.
		*/
		auto filename = file::mkstemp("jass");

		/*
			write, read back, and check we didn't lose anything along the way.
		*/
		std::string bad_filename = "";
		write_entire_file(bad_filename, example_file);
		write_entire_file(filename, example_file);
		read_entire_file(filename, reread);
		JASS_assert(example_file == reread);
		
		/*
			Check that read works
		*/
		file *disk_object = new file(filename, "rb");
		std::vector<uint8_t> disk_object_contents;
		disk_object_contents.resize(example_file.size() + 1024);
		disk_object->read(disk_object_contents);
		std::string disk_object_as_string(disk_object_contents.begin(), disk_object_contents.end());
		JASS_assert(example_file == disk_object_as_string);
		
		disk_object->read(disk_object_contents);			// read past end of file
		JASS_assert(disk_object_contents.size() == 0);

		/*
			Check seek and tell()
		*/
		disk_object->seek(5);
		uint8_t byte;
		auto check = disk_object->read(&byte, 1);
		JASS_assert(check == 1);
		JASS_assert(byte == example_file[5]);
		JASS_assert(disk_object->tell() == 6);

		/*
			Clean up
		*/
		delete disk_object;
		(void)remove(filename.c_str());								// delete the file once we're done with it (cast to void to remove Coverity warning)
	
		/*
			CHECK BUFFER_TO_LIST()
		*/
		/*
			Empty file is of length 0
		*/
		example_file = "";
		buffer_to_list(lines, example_file);
		JASS_assert(lines.size() == 0);

		/*
			File with only blank lines is of length 0
		*/
		example_file = "\r\n";
		buffer_to_list(lines, example_file);
		JASS_assert(lines.size() == 0);

		/*
			File without any new lines is of length 1
		*/
		example_file = "one";
		buffer_to_list(lines, example_file);
		JASS_assert(lines.size() == 1);
		JASS_assert(std::string((char *)lines[0]) == example_file);
		
		/*
			File with a single new line in the middle (none on the end) is of length 2
		*/
		example_file = "one\ntwo";
		buffer_to_list(lines, example_file);
		JASS_assert(lines.size() == 2);
		JASS_assert(std::string((char *)lines[0]) == "one");
		JASS_assert(std::string((char *)lines[1]) == "two");

		/*
			File with tons of blank lines, this one is of length 2
		*/
		example_file = "\n\n\none\r\n\n\rtwo\n\r\n\r\r\r\n\n\n";
		buffer_to_list(lines, example_file);
		JASS_assert(lines.size() == 2);
		JASS_assert(std::string((char *)lines[0]) == "one");
		JASS_assert(std::string((char *)lines[1]) == "two");

		/*
			Try stdin
		*/
		file stdio(stdin);
		JASS_assert(stdio.size() == (std::numeric_limits<size_t>::max)());

		/*
			Try with a FILE *
		*/
		file star(nullptr);
		JASS_assert(stdio.size() == (std::numeric_limits<size_t>::max)());

		/*
			CHECK SETVBUF
		*/
		{
		auto filename = file::mkstemp("jass");
		{
		file tester(filename, "w+b");
		tester.setvbuf(3);
		tester.write(example_file);
		}
		std::string got;
		read_entire_file(filename, got);
		JASS_assert(got == example_file);
		}

		/*
			Yay, we passed
		*/
		puts("file::PASSED");
		}
	}
