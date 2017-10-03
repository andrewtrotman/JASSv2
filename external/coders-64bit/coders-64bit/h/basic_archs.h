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


/* --------------------------------------------
 basic_archs.h
   top header file, the standard use:
	    #include "basic_types.h" (which incluide basic_archs
	 
	 Important compiler flags:
	   _USE_LARGE_INT          when at least one of the value used need >32 bits
		                         this applied to INT/UINT types
		 _BYTES_PER_WORD=1/2/4/8 To define logical wordsize (in bytes)
		                         Default= 4/8 for 32-/64-bit machines 
*/


#ifndef _BASIC_ARCHS_H_
#define _BASIC_ARCHS_H_

#include <sys/types.h>
#include <limits.h>

#define _ARCH_32 32
#define _ARCH_64 64

/* The following fragment might be dangerous - check with OS */
#ifndef __WORDSIZE         /* normally __WORDSIZE is defined */
  #define __WORDSIZE 32
  #ifndef __GLIBC_HAVE_LONG_LONG
    #define __GLIBC_HAVE_LONG_LONG
  #endif
#endif

#if __WORDSIZE == 32
  #define _ARCH_32BITS
  #define _ARCH _ARCH_32
  #ifdef __GLIBC_HAVE_LONG_LONG
    #define _HAVE_64BIT_INT
  #endif
#elif __WORDSIZE == 64
  #define _ARCH_64BITS
  #define _ARCH _ARCH_64
  #define _HAVE_64BIT_INT
#else
  #error "basic_archs.h : System word size and arch not recognised"
#endif
  

#endif /* _BASIC_ARCHS_H_ */
