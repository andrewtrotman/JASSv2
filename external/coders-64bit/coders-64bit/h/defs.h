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

#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "basic_types.h"


/* generous useful definitions, special attention on INF */ 
#define INF 2000000000    /* infinity */
#define INFI 2000000000    /* infinity */
#define VALID(__X) ( (__X) < INFI )
#define INVALID(__X) ( (__X) >= INFI )
#define MAXVAL INFI     
#define MAKE_INVALID(__X) __X=MAXVAL;
#define DUMP(_x_, _fmt_) fprintf(stderr,"%s:%u: %s=" _fmt_ "\n", __FILE__, __LINE__, #_x_, _x_);

#define ERR_MSG(_msg) fprintf(stderr,"%s:%u: %s\n", __FILE__, __LINE__, _msg);

#define MIN(X,Y) (X<Y? X : Y)
#define MAX(X,Y) (X>Y? X : Y)
/* bit to megqbyte  */
#define B2G(_X) ( (double) _X )/8.0/1024/1024/1024
#define B2M(_X) ( (double) _X )/8.0/1024/1024

#define Sfree(_x) _x= _x? (free(_x),NULL) : NULL; 
// malloc for X, type _T, _n elems 
#define Tmalloc(_X,_T,_n)                 \
{                                      \
	if (_n)                                 \
	if (!(_X= malloc(sizeof(_T)*(_n)))) {  \
		fprintf(stderr,"Cannot allocate memory of ");  \
		FPRNF_GINT(stderr,sizeof(_T)*(_n),8);          \
		fprintf(stderr," bytes requested at file %s line %d\n",__FILE__,__LINE__ );  \
		exit(1);                             \
	}                                      \
}
/* ---------------------------------------------------- */





/* ---- The rest is pretty much specific, and many of them
        are not actually used in this particular distribution ---- */ 

/* enable next line if wanted old_style blocking 
#define _NO_TOTAL_ELEMS_
*/


// #define _STATS_   /* comment this line out if you don't like stats */ 
#include "stats.h"


#ifdef MSIMPLE
  #define VARIABLE_LENGTH_BLOCK
  #ifndef FAST_DECODE
	  #define FAST_DECODE
	#endif
#endif


#ifndef FAST_DECODE
#ifndef FASTER_DECODE
  #define NORMAL_DECODE
#endif
#endif

#ifdef FAST_DECODE
  #define BLK_LEVEL_BUFFER_MANAGEMENT  /* as opposed to word- or byte-level */
#endif

  #define BYTE_BUFFER_SIZE 536870912   /* 500 MB */
  #if BYTES_PER_WORD==4
    #define WORD_BUFFER_SIZE 134217728
  #else
    #define WORD_BUFFER_SIZE 67108864  
  #endif


#define BLKSZE_ADJUST 1024  /* major factor for 1024: see msim_variants.h */

#ifdef UFORDELTA
  #define STD_ELEMS_PER_BLOCK 128
  #define STD_BITS_FOR_ELEMS_PER_BLOCK  8  /* whole byte required */
#else
  #ifdef BBLOCK
    #ifdef CBLOCK
      #define STD_ELEMS_PER_BLOCK (131072 - BLKSZE_ADJUST)
      #define STD_BITS_FOR_ELEMS_PER_BLOCK 17
	  #else
      #define STD_ELEMS_PER_BLOCK 128
      #define STD_BITS_FOR_ELEMS_PER_BLOCK 7
	  #endif
  #else
	  #ifdef GOLOMB
      #define STD_ELEMS_PER_BLOCK 512
	    #define STD_BITS_FOR_ELEMS_PER_BLOCK  9
		#else
      #define STD_ELEMS_PER_BLOCK (131072 - BLKSZE_ADJUST)
	    #define STD_BITS_FOR_ELEMS_PER_BLOCK  17
		#endif
  #endif
#endif

#define MIN_DEC_BUF_LEN_BYTE (131072<<2) 
#if WORD_SIZE == 32 
  #define MIN_DEC_BUF_LEN_WORD 131072 
#else
  #define MIN_DEC_BUF_LEN_WORD 65536 
#endif

#if WORD_SIZE == 32
  #define BIT_FOR_ZERO_LEN 6
  #define MAX_ELEM_PER_WORD 64
  #define MASK_FOR_ZERO_LEN 63
	#define MIN_ZERO_PATTERN 8
#else
  #define BIT_FOR_ZERO_LEN 8
  #define MAX_ELEM_PER_WORD 256
  #define MASK_FOR_ZERO_LEN 255
	#define MIN_ZERO_PATTERN 12
#endif






#if WORD_SIZE==32
#ifdef UNARY
  #define COMP_METHOD "unary4"
#endif
#ifdef BINARY
  #define COMP_METHOD "binary4"
#endif
#ifdef GAMMA
  #define COMP_METHOD "gamma4"
#endif
#ifdef GOLOMB
  #define COMP_METHOD "golomb4"
#endif
#ifdef AGOLOMB
  #define COMP_METHOD "agolomb4"
#endif
#ifdef DELTA
  #define COMP_METHOD "delta4"
#endif
#ifdef INTERP
  #if defined SMALL_SHORT
    #define COMP_METHOD "small_interp4"
	#elif defined INVERSE
	  #define COMP_METHOD "inv_interp4"
	#elif defined ANDREW
	  #define COMP_METHOD "aht4"
	#else
	  #define COMP_METHOD "interp4"
	#endif
#endif

#else

#ifdef UNARY
  #define COMP_METHOD "unary8"
#endif
#ifdef BINARY
  #define COMP_METHOD "binary8"
#endif
#ifdef GAMMA
  #define COMP_METHOD "gamma8"
#endif
#ifdef GOLOMB
  #define COMP_METHOD "golomb8"
#endif
#ifdef AGOLOMB
  #define COMP_METHOD "agolomb8"
#endif
#ifdef DELTA
  #define COMP_METHOD "delta8"
#endif

#ifdef INTERP
  #if defined SMALL_SHORT
    #define COMP_METHOD "small_interp8"
	#elif defined INVERSE
	  #define COMP_METHOD "inv_interp8"
	#elif defined ANDREW
	  #define COMP_METHOD "aht8"
	#else
	  #define COMP_METHOD "interp8"
	#endif
#endif

#endif





#define _DEBUG 0
#define yes 1
#define no 0
#define ENCODE 0
#define DECODE 1

#define OUTPUT_VAL(x)                       \
	a[i++]= x;                                \
  if (i==epb) {                 \
		WriteDocGaps(outf, a, i, ofile, text_file, sequence_type,&curr); \
		i= 0;                                   \
	}


#endif
