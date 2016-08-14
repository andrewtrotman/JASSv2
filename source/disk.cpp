/*
	DISK.C
	------
*/
#ifdef _MSC_VER
	#include <windows.h>
    #include <direct.h>
#else
	#include <string.h>
#endif
#include <new>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "disk.h"
#include "file_internals.h"
#include "bool.h"

/*
	ANT_DISK::READ_ENTIRE_FILE()
	----------------------------
*/
char *ANT_disk::read_entire_file(char *filename, long long *file_length)
{
long long unused;
char *block = NULL;
#ifdef _MSC_VER
	HANDLE fp;
	LARGE_INTEGER details;
	#if defined(UNICODE) || defined(_UNICODE)
		LPCWSTR true_filename = (LPCWSTR)filename;					// If we're in UNICODE land then the filename is actually a wide-string
	#else
		char *true_filename = filename;									// else we're in ASCII land and so the filename is a c-string
	#endif
#else
	FILE *fp;
	struct stat details;
#endif

if (filename == NULL)
	return NULL;

if (file_length == NULL)
	file_length = &unused;

#ifdef _MSC_VER
	fp = CreateFile(true_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fp == INVALID_HANDLE_VALUE)
		{
//		DWORD error_code = GetLastError();			// put a break point on this in the debugger to work out what went wrong.
		return NULL;
		}

	if (GetFileSizeEx(fp, &details) != 0)
		if ((*file_length = details.QuadPart) != 0)
			if ((block = new (std::nothrow) char [(size_t)(details.QuadPart + 1)]) != NULL)		// +1 for the '\0' on the end
				{
				if (ANT_file_internals::read_file_64(fp, block, details.QuadPart) != 0)
					block[details.QuadPart] = '\0';
				else
					{
					delete [] block;
					block = NULL;
					}
				}

	CloseHandle(fp);
#else
	if ((fp = fopen(filename, "rb")) == NULL)
		return NULL;

	if (fstat(fileno(fp), &details) == 0)
		if ((*file_length = details.st_size) != 0)
			if ((block = new (std::nothrow) char [(size_t)(details.st_size + 1)]) != NULL)		// +1 for the '\0' on the end
				if (ANT_file_internals::read_file_64(fp, block, details.st_size) != 0)
					block[details.st_size] = '\0';
				else
					{
					delete [] block;
					block = NULL;
					}
	fclose(fp);
#endif

return block;
}

/*
	ANT_DISK::WRITE_ENTIRE_FILE()
	-----------------------------
*/
char *ANT_disk::write_entire_file(char *filename, char *buffer, long length)
{
FILE *fp;
size_t success;

if ((fp = fopen(filename, "wb")) == NULL)
	return NULL;

success = fwrite(buffer, length, 1, fp);

fclose(fp);

return success == 1 ? buffer : NULL;
}

/*
	ANT_DISK::BUFFER_TO_LIST()
	--------------------------
*/
char **ANT_disk::buffer_to_list(char *buffer, long long *lines)
{
char *pos, **line_list, **current_line;
long n_frequency, r_frequency;

n_frequency = r_frequency = 0;
for (pos = buffer; *pos != '\0'; pos++)
	if (*pos == '\n')
		n_frequency++;
	else if (*pos == '\r')
		r_frequency++;

*lines = r_frequency > n_frequency ? r_frequency : n_frequency;
current_line = line_list = new (std::nothrow) char * [(size_t)(*lines + 2)]; 		// +1 in case the last line has no \n; +1 for a NULL at the end of the list

if (line_list == NULL)		// out of memory!
	return NULL;

*current_line++ = pos = buffer;
while (*pos != '\0')
	{
	if (*pos == '\n' || *pos == '\r')
		{
		*pos++ = '\0';
		while (*pos == '\n' || *pos == '\r')
			pos++;
		*current_line++ = pos;
		}
	else
		pos++;
	}
/*
	We have a nasty case here.  If the last line has no CR/LF then we need to include it
	but shove a NULL on the next line, but if the last line has a CR/LF then we need to avoid
	adding a blank line to the end of the list.
	NOTE: its 2012 and its a bit late to be finding this bug!!!
*/
if (**(current_line - 1) == '\0')
	*(current_line - 1) = NULL;
*current_line = NULL;

*lines = current_line - line_list - 1;		// the true number of lines

return line_list;
}

/*
	ANT_DISK::IS_DIRECTORY()
	--------------------------
*/
long ANT_disk::is_directory(char* filename) {
#ifdef _MSC_VER
	return (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(filename)) ? TRUE : FALSE;
#else
	struct stat st;
	if(stat(filename, &st) == 0)
		return S_ISDIR(st.st_mode);
	return FALSE;
#endif
}


