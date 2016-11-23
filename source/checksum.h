/*
	CHECKSUM.H
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Checksum routines
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

namespace JASS
	{
	class checksum
		{		  
		/*!

		@details see: 
						https://en.wikipedia.org/wiki/Fletcher's_checksum
						http://www.drdobbs.com/database/fletchers-checksum/184408761
						Fletcher, J. G., “An Arithmetic Checksum for Serial Transmissions”, IEEE Trans. on Comm., Vol. COM-30, No. 1, January 1982, pp 247-252.
		*/
		
		uint16_t fletcher_16(void *data, int length);
		static void unittest(void);
		};

	}