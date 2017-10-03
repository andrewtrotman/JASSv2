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


/**************************************************************************
 *
 * copied from carry_coder.h of skip_carry

*/


#ifndef _CARRY_CODERH
#define _CARRY_CODERH

#ifdef NCARRY
#define CARRY
#endif

#ifdef CARRY    /* this is specific for w64 */

#include "word_coder.h"
#include "defs.h"
#include "qlog2.h"
I4
CalcMinBits(INT *gaps, UCHAR *bits, I4 n);


#ifdef NCARRY
  #if WORD_SIZE == 64
    #include "ncarry8.h"
  #else
	  #error "carry.h: ncarry4 is not a valid option"   
  #endif 
#else
  #if WORD_SIZE == 64
    #include "carry8.h"
		/* Notes:
		   carry8.h does not use relative selector
			 org_carry.h does, and not used!
		*/
  #else
    #include "carry4.h"
  #endif 
#endif




/* ------------------ Coding Interface -------------------- */

#ifndef BIT_2_SEL
  #define BIT_2_SEL
#endif

#define CARRY_ENCODE_START(f) {       \
  CARRY_CODING_VARS;                  \
	BIT_2_SEL                           \
	WORD_ENCODE_START(f);

#define CARRY_ENCODE_END              \
  WORD_ENCODE_END                     \
}

#define CARRY_DECODE_START(f)     \
{                                 \
  CARRY_CODING_VARS;              \
	WORD_DECODE_START(f);

#define CARRY_DECODE_END          \
  WORD_DECODE_END;                \
}




#define SCHEME_ENCODE_START(f) CARRY_ENCODE_START(f) 
#define SCHEME_ENCODE_END(f) CARRY_ENCODE_END

#define SPECIFIC_DECODING_VARS
#define SCHEME_DECODE_START(f) CARRY_DECODE_START(f)
#define SCHEME_DECODE_END CARRY_DECODE_END


#define GET_PARAMETERS CARRY_BLOCK_DECODE_START(n)
#define E_DECODE CARRY_DECODE(a[i])

//#define SCHEME_SKIP(_n) CARRY_SKIP(_n)
#define SCHEME_SKIP(_n) 


#endif /* ifdef CARRY */

#endif
	
