/*
	STEM.H
	------
*/
#pragma once

#include <string>

namespace JASS
	{
	/*
		STEM
		----
	*/
	class stem
		{
		public:
			stem()
				{
				/* Nothing */
				}
			virtual ~stem()
				{
				/* Nothing */
				}

			virtual size_t tostem(char *destination, const char *term) = 0;

			virtual std::string name(void) = 0;
		} ;
	}
