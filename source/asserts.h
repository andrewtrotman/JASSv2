/*
	ASSERTS.H
	---------
*/

namespace JASS
	{
	#define assert(condition) ((condition) ? (void)0 : __assert(__func__, __FILE__, __LINE__, #condition))
	}