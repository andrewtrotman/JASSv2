/* --------------------------------------------------------
  Copyright (C) 2009 Vo Ngoc Anh and Alistair Moffat, The University
	  of Melbourne, Australia.

	This program is free software; you can use, redistribute it and/or modify
	it, providing that these heading lines remain.
  There is no warranty, either express or implied, that it is fit for
	any purpose whatsoever, and neither the authors nor The University of
	Melbourne accept any responsibility for any consequences that may
	arise from your use of this software.

  We ask that, if you use this software to derive experimental results
	that are reported in any way, you cite the original work in which the
	underlying processes are described by referencing to the paper:
	  ...


  AUTHORS
	Vo Ngoc Anh and Alistair Moffat,
	Department of Computer Science and Software Engineering,
	The University of Melbourne,
	Victoria 3010, Australia.
	Email: vo@csse.unimelb.edu.au, alistair@csse.unimelb.edu.au.

	BUG & FEEDBACK REPORT: Please send to vo@csse.unimelb.edu.au


Released at: Thu Aug 27 14:52:57 EST 2009

 -------------------------------------------------------- */


/* 64 */

/* ------------------------------------------------------------------------
basic_types.h system-indepentdent basic types and related constant
  -- built from mt_types.h
  -- suggestion: mt_types must be defined in the next levl of abstraction
   ------------------------------------------------------------------------ */
 
/* Usage:
   -- Important compiler flags:
	    _USE_LARGE_INT          when at least one of the value used need >32 bits
		                         this applied to INT/UINT types
		 _BYTES_PER_WORD=1/2/4/8 To define logical wordsize (in bytes)
		                         Default= 4/8 for 32-/64-bit machines 
   -- Types defined incl. I1,I2,I4,I8,U1,U2,U4,U8
	    and WORD, UWORD which are machine word, can be 32 or 64 bits
*/
	 

#ifndef _BASIC_TYPES_H_
#define _BASIC_TYPES_H_

#include "int_types.h"

typedef float FLOAT;
typedef double DOUBLE;
	
#endif /* _BASIC_TYPES_H_ */
