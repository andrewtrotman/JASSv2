/*
	MEMORY.C
	--------
*/
#ifdef _MSC_VER
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

#if (_WIN32_WINNT < 0x0600) || (_MSC_VER < 1500) 			// prior to Vista or prior to Visual Studio 2008
	/*
		If you compile under XP then GetLargePageMinimum() doesn't exist; it is first seen in
		Server 2003 and Vista.  So, if you have a version of the compiler from the XP era then
		we need to fake this method because you might compile under XP and run under Vista (or
		later) which would cause the run-time check to succeed so a function stub is needed.
		The solution is to require Microsoft C/C++ version 15.00 or later, but some existing
		users don't have that.

		The hack is to set the large page size to the small page size (4096 bytes) in the case 
		of compiled under XP but run under Vista.
	*/
	#define GetLargePageMinimum() (4096)
#endif

#ifndef LARGE_MEMORY_PAGES
	#undef GetLargePageMinimum
	#define GetLargePageMinimum() (4096)
#endif

/*
	ANT_MEMORY::ANT_MEMORY()
	------------------------
*/
ANT_memory::ANT_memory(long long block_size_for_allocation, long long memory_ceiling)
{
#ifdef _MSC_VER
	OSVERSIONINFO os_info;
	SYSTEM_INFO hardware_info = {0};

	os_info.dwOSVersionInfoSize = sizeof(os_info);
	GetVersionEx(&os_info);
	has_large_pages = os_info.dwMajorVersion >= 6;		// version 6 is Vista & Server2008 - prior to this there was no large page support
	large_page_size = has_large_pages ? GetLargePageMinimum() : 0;
	GetSystemInfo(&hardware_info);
	short_page_size = hardware_info.dwPageSize;
#else
	if ((short_page_size = large_page_size = sysconf(_SC_PAGESIZE)) <= 0)
		short_page_size = large_page_size = 4096;		// use 4K blocks by default (as this is the Pentium (and ARM) small page size)
#endif
//printf("Large Page Size: %lld Small Page Size:%lld\n", (long long)large_page_size, (long long)short_page_size);

this->block_size = block_size_for_allocation;
this->memory_ceiling = memory_ceiling == 0 ? LLONG_MAX : memory_ceiling;
chunk = NULL;

rewind();
}

/*
	ANT_MEMORY::~ANT_MEMORY()
	-------------------------
*/
ANT_memory::~ANT_memory()
{
rewind();		// free the all in-use memory (if any)
}

#ifdef _MSC_VER
	/*
		ANT_MEMORY::SET_PRIVILEGE()
		---------------------------
	*/
	long ANT_memory::set_privilege(char *priv_name, long enable)
	{
	HANDLE token;
	TOKEN_PRIVILEGES tp;
	long success = FALSE;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
	  
	if (LookupPrivilegeValue(NULL, priv_name, &tp.Privileges[0].Luid) == 0)
		CloseHandle(token);
	else
		if (AdjustTokenPrivileges( token, FALSE, &tp, 0, NULL, 0) == ERROR_NOT_ALL_ASSIGNED)
			CloseHandle(token);
		else
			success = TRUE;

	return success;
	}
#endif

/*
	ANT_MEMORY::ALLOC()
	-------------------
	Allocate large pages from the OS (and if this fails then use small pages).  Under Windows large pages can fail if:
	1. we are out of memory
	2. the memory is fragmented
	3. we don't have permissions to allocate large pages of memory
	4. we're not on Vista (or later).

	to check whether or not you have permission to allocate large pages see:
	Control Panel->Administrative Tools->Local Security Settings->Local Policies->User Rights Assignment->Lock pages in memory
*/
void *ANT_memory::alloc(long long *size)
{
/*
	allocate memory from the operating system
*/
#ifdef _MSC_VER
	#ifdef PURIFY
		/*
			call malloc on every memory allocation
		*/
		if (allocated + *size > memory_ceiling)
			return NULL;			// we've overflowed the soft limit
		else
			return ::malloc((size_t)*size);
	#else
		/*
			get a large chunk of memory from Windows
		*/
		void *answer = NULL;
		long long bytes = 0;

		/*
			First try using large page memory blocks
		*/
		#ifdef LARGE_MEMORY_PAGES
			if (has_large_pages)
				if (set_privilege(SE_LOCK_MEMORY_NAME, TRUE))		// try and get permission from the OS to allocate large pages
					{
					bytes = large_page_size * ((*size + large_page_size - 1) / large_page_size);
					if (allocated + bytes <= memory_ceiling)		// check for overflow of the soft memory limt
						answer = VirtualAlloc(NULL, (size_t)bytes, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
					set_privilege(SE_LOCK_MEMORY_NAME, FALSE);		// drop back to the initial security level
					}
		#endif
		/*
			If we don't have large pages or large pages fail then fall back to small pages
		*/
		if (answer == NULL)
			{
			bytes = short_page_size * ((*size + short_page_size - 1) / short_page_size);
			if (allocated + bytes <= memory_ceiling)		// check for overflow of the soft memory limt
				answer = VirtualAlloc(NULL, (size_t)bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			}

		if (answer == NULL)
			bytes = 0;		// couldn't allocate any memory.
		else
			{
			/*
				This code is commented out at the moment because when I ran this over the
				INEX Wikipedia 2009 collection it caused a VISTA hang!  It looks like you
				can't lock more than 4GB of memory - but this is a guess.
			*/
//			VirtualLock(answer, (size_t)bytes);		// lock the pages in memory so that it can't page to disk
			}

		/*
			If we haven't been able to allocate memory then we're out of memory (either soft or hard)
		*/
		*size = bytes;		// number of bytes we allocated
		return answer;
	#endif
#else
	/*
		We're not on windows so call the C run time library malloc()
	*/
	if (allocated + *size > memory_ceiling)
		return NULL;			// we've overflowed the soft limit
	else
		return ::malloc((size_t)*size);
#endif
}

/*
	ANT_MEMORY::DEALLOC()
	---------------------
	Give pages back to the Operating System.  In Windows this works because VirtualAlloc() and
	VirtualFree() actually do this.  On other operating systems the C/C++ run time library will
	typically hold on to those pages in case they are needed later.  The Windows way is better
	becuase it makes the pages available again later rather then fragmenting the C RTL heap.
*/
void ANT_memory::dealloc(char *buffer)
{
#ifdef _MSC_VER
	#ifdef PURIFY
		::free(buffer);
	#else
		VirtualFree(buffer, 0, MEM_RELEASE);
	#endif
#else
	::free(buffer);
#endif
}

/*
	ANT_MEMORY::GET_CHAINED_BLOCK()
	-------------------------------
	The bytes parameter is passed to this routine simply so that we can be sure to
	allocate at least that number of bytes.
*/
void *ANT_memory::get_chained_block(long long bytes)
{
char **chain;
long long request, *size;

#ifdef PURIFY
	request = bytes + sizeof(*chain) + sizeof(bytes);
#else
	if (bytes > block_size)
		block_size = bytes;			// extend the largest allocate block size

	request = block_size + sizeof(*chain) + sizeof(bytes);
#endif

if ((chain = (char **)alloc(&request)) == NULL)
	return NULL;

/*
	Create a linked list of large blocks of memory.
	the first sizeof(char *) bytes are the pointer to the previous block
	whereas the next sizeof{bytes) bytes are the size of this block
*/
*chain = chunk;
chunk = (char *)chain;
size = (long long *)(chunk + sizeof(*chain));
*size = bytes;

/*
	Mark the current location we are to allocate from (which we call at)
	and the end of the block (chunk_end)
*/
at = chunk + sizeof(*chain) + sizeof(bytes);
chunk_end = chunk + request;		// request is the amount it allocated (which might be more than we asked for)

/*
	This is the amount we have allocated in total
*/
allocated += request;

return chunk;
}

/*
	ANT_MEMORY::REALIGN()
	---------------------
	realign() does two things.  First, it aligns the next block of memory on the correct boundary for the largest
	type we know about (a 64-bit long long) to avoid memory miss-allignment overheads.  Second, and as a consequence,
	it cache-line aligns the next memory allocation (Intel uses a 64-byte cache line) thus reducing the number of
	cache misses if we process the memory sequentially.
*/
void ANT_memory::realign(void)
{
long long padding;

padding = (used % sizeof(long long) == 0) ? 0 : sizeof(long long) - used % sizeof(long long);

/*
	this might overflow if we're at the end of a block but that doesn't matter because the next call to malloc()
	will notice that the overflow and allocate a new block.
*/
at += padding;
allocated += padding;
used += padding;
}

/*
	ANT_MEMORY::REWIND()
	--------------------
*/
void ANT_memory::rewind(void)
{
char *chain, *killer;

/*
	Free all memory blocks
*/
for (chain = chunk; chain != NULL; chain = killer)
	{
	killer = *(char **)chain;
	dealloc(chain);
	}

/*
	zero the counters and pointers
*/
chunk_end = at = chunk = NULL;
used = 0;
allocated = 0;
}
