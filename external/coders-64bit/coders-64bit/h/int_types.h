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


/* int_types.h
   defs of integer types and in/out formats 
   NOT required .c */


/* ------------- THE LEGACY OF INT TYPES ----------------------------
   Why? 
	  - To avoid the confusion between different C int types 
		  across machine architechtures
		

Usage:
  #include "basic_types" in your program, it will invoke int_types.h

   -- Elementary int types are defined as I1,I2,I4,I8,U1,U2,U4,U8
	    If you are certain of the size of int you want, use one of them.
			However, see next.

	 -- Type INT/UINT is normally set to I4/U4. However, the compiler flag:
	      _USE_LARGE_INT
			will force them to be I8/U8. If you care about efficiency and also
			want a value to be very large when needed, declare it at INT/UINT.

	 -- Type WORD/UWORD represent the pice of data that can be manipulated 
	    one machine command. Normaly WORD/UWORD represent the real
			machine word (and is platform-dependent). However, you can redefine 
			WORD to I1/I2/I4/I8 for the simulation purpose. These data types
			are not meant to keep application data, and must be sed with care.
			In absolute term, the redefinition does not reflect a full 
			simulation, it only means that some specific operations are
			done in a WORD-BY-WORD manner.
			By default WORD/UWORD are defined as INT/UINT
			To rest use the compiler flag:
			  _BYTES_PER_WORD=1/2/4/8
	
*/
	 

#ifndef _INT_TYPES_H_
#define _INT_TYPES_H_

#include "basic_archs.h"


/* -------- Level 1 abstraction: define I1-I8, U1-U8 ----------- 
      Supposing that all up-to-64-bit ints are available in any platform */
 
typedef char CHAR;
typedef unsigned char UCHAR;

typedef char I1;
typedef unsigned char U1;
typedef short int I2;
typedef unsigned short int U2;
typedef int I4;
typedef unsigned int U4;

#define MAX_I1 Ox7F
#define MAX_U1 0xFF
#define MAX_I2 0x7FFF
#define MAX_U2 0xFFFF

#define MAX_I4 0x7FFFFFFFu
#define MAX_U4 0xFFFFFFFFu

#ifdef _ARCH_32BITS

  #ifdef _HAVE_64BIT_INT

     typedef long long int I8;
		 typedef unsigned long long int U8;
		 #define MAX_I8 0x0FFFFFFFFFFFFFFFll
     #define MAX_U8 0xFFFFFFFFFFFFFFFFull

     #define U8_FO 0xFFFFFFFFFFFFFFF0ull
     #define I8_FO 0xFFFFFFFFFFFFFFF0ll
     #define I8_F8 0xFFFFFFFFFFFFFFF8ll
     #define U8_F8 0xFFFFFFFFFFFFFFF8ull
     #define I8_FF 0xFFFFFFFFFFFFFFFFll
     #define U8_FF 0xFFFFFFFFFFFFFFFFull
     #define U8_FFFFFFFF 0xFFFFFFFFull
		 #define U8_1 1ull
		 #define I8_1 1ll
		 #define MASK60 1152921504606846975ull
	#else
     #warning Data types I8 and U8 cannot be defined
	#endif

#else
		 
	typedef long int I8;
	typedef unsigned long int U8;
 
  #define MAX_I8 0x0FFFFFFFFFFFFFFFl
  #define MAX_U8 0xFFFFFFFFFFFFFFFFul
  #define I8_FO 0xFFFFFFFFFFFFFFF0l
  #define U8_FO 0xFFFFFFFFFFFFFFF0ul
  #define I8_F8 0xFFFFFFFFFFFFFFF8l
  #define U8_F8 0xFFFFFFFFFFFFFFF8ul
  #define I8_FF 0xFFFFFFFFFFFFFFFFl
  #define U8_FF 0xFFFFFFFFFFFFFFFFul
  #define U8_FFFFFFFF 0xFFFFFFFFul
	#define U8_1 1ul
	#define I8_1 1l
  #define MASK60 1152921504606846975ul

#endif 

/* ------------- End of Level 1 ---------------- */



/* Level 2 Abstraction: for system tools (eg coders) ---------------------
 
	 -- WORD, UWORD should be used by low-level coders to implement their
	    logical (shifts, mask...) operations
	 -- WORD_SIZE is defined in accordance with WORD, and might be different
	    from the system's __WORDSIZE
	
	 ---------------------------------------------------------------------- */		
	
#ifndef _BYTES_PER_WORD
#ifdef _ARCH_32BITS
  #define _BYTES_PER_WORD 4
#endif
#ifdef _ARCH_64BITS
  #define _BYTES_PER_WORD 8
#endif
#endif

#if _BYTES_PER_WORD == 8
  #ifndef _HAVE_64BIT_INT
	  #error "No 64-bit int type detected - _BYTES_PER_WORD innapropriate."
	#endif
	#define WORD I8
	#define UWORD U8
	#define WORD_SIZE 64
  #define MAX_WORD MAX_I8
  #define MAX_UWORD MAX_U8
#endif
#if _BYTES_PER_WORD == 4
	#define WORD I4
	#define UWORD U4
	#define WORD_SIZE 32
  #define MAX_WORD MAX_I4
  #define MAX_UWORD MAX_U4
#endif
#if _BYTES_PER_WORD == 2
	#define WORD I2
	#define UWORD U2
	#define WORD_SIZE 16
  #define MAX_WORD MAX_I2
  #define MAX_UWORD MAX_U2
#endif
#if _BYTES_PER_WORD == 1
	#define WORD I1
	#define UWORD U1
	#define WORD_SIZE 8
  #define MAX_WORD MAX_I1
  #define MAX_UWORD MAX_U1
#endif
#ifndef WORD_SIZE
  #error "Innapropriate _BYTES_PER_WORD."
#endif

	 

/* Level 3 Abstraction: for applications           ---------------------

	-- By default, application use 32-bit int as the basic type;
	   if 64-bit required, must compiled with _USE_LARGE_INT
  -- KISS: application should use only CHAR, INT, UCHAR, UINT 
	   although it's possible to use WORD-style types
	
	--------------------------------------------------------------------- */

#ifdef _USE_LARGE_INT
	#define INT I8
	#define UINT U8
	#define MAX_INT MAX_I8
	#define MAX_UINT MAX_U8
	#define UINT_1 U8_1
	#define INT_1 I8_1
#else
	#define INT I4
	#define UINT U4
	#define MAX_INT MAX_I4
	#define MAX_UINT MAX_U4
	#define UINT_1 1
	#define INT_1 1
#endif

 
#ifdef USING_U5
  #include "U5_types.h"
#endif /* USING_U5 */


#endif /* _INT_TYPES_H_ */	
