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


/* accompanying codes.c */

I4 icmp(const void *a, const void *b);


#include "driver.h"
#include "carry.h"
#include "simple.h"
#include "slide.h"
#include "byte.h"
#include "u4d.h"


#ifdef UNARY
#define E_ENCODE        UNARY_ENCODE(a[i])
#define E_DECODE        UNARY_DECODE(a[i])
#define E_SKIP
#endif

#ifdef BINARY
#define ENCODE_PARAMETERS b= CalcMax(a,n); WORD_CROSS_ENCODE(b,31); if (b==1) continue;
#define E_ENCODE        BINARY_ENCODE(a[i],b)
#define GET_PARAMETERS  WORD_CROSS_DECODE(b,31)
#define E_DECODE        BINARY_DECODE(a[i],b)
#define E_SKIP          BINARY_SKIP_ONE(b)
#define SCHEME_SKIP(_n) BINARY_SKIP(_n,b)
#endif

#ifdef GAMMA
#define E_ENCODE        GAMMA_ENCODE(a[i])
#define E_DECODE        GAMMA_DECODE(a[i])
#define E_SKIP
#endif

#ifdef GOLOMB
#define ENCODE_PARAMETERS {     \
  U8 NN;                        \
	NN= CalcSum(a,n);             \
	GOLOMB_INIT(NN,n,b);          \
	WORD_CROSS_ENCODE(b,31);      \
}
#define E_ENCODE        GOLOMB_ENCODE(a[i],b)
#define GET_PARAMETERS  WORD_CROSS_DECODE(b,31)
#define E_DECODE        GOLOMB_DECODE(a[i],b)
#define E_SKIP          GOLOMB_SKIP_ONE(b)
#endif

#ifdef AGOLOMB
#define ENCODE_PARAMETERS AGOLOMB_INIT(b);
#define E_ENCODE        { GOLOMB_ENCODE(a[i],b) AGOLOMB_NEXT_b(a[i],b) }
#define GET_PARAMETERS  AGOLOMB_INIT(b)
#define E_DECODE        { GOLOMB_DECODE(a[i],b) AGOLOMB_NEXT_b(a[i],b) }
#define E_SKIP
#endif

#ifdef DELTA
#define E_ENCODE        DELTA_ENCODE(a[i])
#define E_DECODE        DELTA_DECODE(a[i])
#define E_SKIP
#endif

/* for non-parameterised codes */
#ifndef ENCODE_PARAMETERS
#define ENCODE_PARAMETERS
#endif

#ifndef GET_PARAMETERS
#define GET_PARAMETERS
#endif

/* for codes that don't have any specific vars */
#ifndef SPECIFIC_ENCODING_VARS
#define SPECIFIC_ENCODING_VARS CLOG2TAB_VAR
#endif

#ifndef SPECIFIC_DECODING_VARS
#define SPECIFIC_DECODING_VARS CLOG2TAB_VAR
#endif

#ifndef SCHEME_ENCODE_START
#define SCHEME_ENCODE_START(f) WORD_ENCODE_START(f) 
#endif

#ifndef SCHEME_ENCODE_END
#define SCHEME_ENCODE_END(f) WORD_ENCODE_END 
#endif

#ifndef SCHEME_DECODE_START
#define SCHEME_DECODE_START(f) WORD_DECODE_START(f) 
#endif

#ifndef SCHEME_DECODE_END
#define SCHEME_DECODE_END WORD_DECODE_END 
#endif

/* for codes that are not groupped such as gamma, golomb */
#ifndef E_BLK_ENCODE
#define E_BLK_ENCODE for (i=0; i<n; i++) E_ENCODE 
#endif

#ifndef E_BLK_DECODE
#define E_BLK_DECODE for (i=0; i<n; i++) E_DECODE
#endif

#ifndef SCHEME_SKIP
#define SCHEME_SKIP(_n)
#endif


