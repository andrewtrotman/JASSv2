/*
	DISK.H
	------
*/
#ifndef DISK_H_
#define DISK_H_

class ANT_disk_internals;
class ANT_disk_directory;

/*
	class ANT_DISK
	--------------
*/
class ANT_disk
{
public:
	ANT_disk() {}
	virtual ~ANT_disk() {}

	static char *read_entire_file(char *filename, long long *len = 0);
	static char **buffer_to_list(char *buffer, long long *lines);
	static char *write_entire_file(char *filename, char *string, long length);
	static long is_directory(char *filename);
} ;

#endif  /* DISK_H_ */
