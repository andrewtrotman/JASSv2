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



#ifndef _SIMPLE_CODERH
#define _SIMPLE_CODERH

#ifdef SIMPLE    /* this is specific for w64 */

/* Definition: SIMPLE is the naive version with the properties:
   1. 4-bit absolute selector which is always at the end of machine word
	 2. Coding always begin from the word border
	 3. zero-bit size is possible
-----------------------------------------------------  */

#include "word_coder.h"
#include "defs.h"
#include "qlog2.h"

I4
CalcMinBits(INT *gaps, UCHAR *bits, I4 n);

#ifndef _NO_TOTAL_ELEMS
  #define BLK_ENC_ADJUST 
#else
  #define BLK_ENC_ADJUST WORD_ENCODE_WRITE
#endif

#define SIMPLE_CODING_VARS CODE_TABLE                             \
                           I4 selsize = 4;


#if WORD_SIZE==32
  #define COMP_METHOD "simple4"
  #define CODE_TABLE                                            \
	UCHAR sel2bit[]=  {  0,  0, 0, 0, 0, 0, 0, 1 ,2,3,4,5,7,9,14,28};       \
	U4 sel2elems[]=   {256,120,90,60,50,40,32,28,14,9,7,5,4,3, 2, 1};       \

  #define BIT_2_SEL                                             \
	CHAR bit2sel[]=   { 0,7,8,9,10,11,12,12,13,13,14,14,14,14,14,           \
	                    15,15,15,15,15,15,15,15,15,15,15,15,15,15,          \
											-1,-1,-1,-1};
  #define MAX_BIT 28
#else
  #define COMP_METHOD "simple8"
  #define CODE_TABLE                                            \
	/*                   0   1  2  3  4  5  6  7  8 9 10 11 12 13 14 15 */ \
	UCHAR sel2bit[]=  {  0,  0, 1 ,2 ,3 ,4 ,5 ,6 ,7,8,10,12,15,20,30,60};   \
	U4 sel2elems[]=   {256,120,60,30,20,15,12,10, 8,7, 6, 5, 4, 3, 2, 1};  

#define BIT_2_SEL CHAR bit2sel[]=                                    \
        {0,2,3,4,5,6,7,8, 9,10,10,11,11,12,12,12,                    \
				13,13,13,13,13,14,14,14, 14,14,14,14,14,14,14,15,            \
				15,15,15,15,15,15,15,15, 15,15,15,15,15,15,15,15,            \
				15,15,15,15,15,15,15,15, 15,15,15,15,15,-1, -1, -1, -1};
				 
  #define MAX_BIT 60 
#endif

/* ======================= MACROS FOR ENCODING ===== */
#define E_BLK_ENCODE                                              \
{                                                                 \
    I4 sel, bit,tmp;                                           \
    BLK_ENC_ADJUST                                                \
    for (i=0; i<n; ) {                                            \
			j= i;                                                       \
			sel= bit2sel[bits[j]];                                      \
			bit= sel2bit[sel];                                          \
			elems= 1;                                                   \
			while (++j<n) {                                             \
			  if (elems == sel2elems[sel]) break;                       \
			  if (bits[j] > bit) {                                      \
				  tmp= bit2sel[bits[j]] ;                                 \
				  if (elems < sel2elems[ tmp ]) {                         \
					  sel= tmp;                                             \
				    bit= sel2bit[sel];                                    \
					} else {                                                \
						while ( elems < sel2elems[sel] ) sel++;               \
						elems= sel2elems[sel];                                \
						bit= sel2bit[sel];                                    \
						break;                                                \
					}                                                       \
				}                                                         \
				elems++;                                                  \
			}                                                          \
			if (bit==0) { /* can be downgrade to bit=1 */         \
			  if (i+elems<n) {                                          \
			    for (sel=0; sel2elems[sel] > elems; sel++);             \
					elems=sel2elems[sel];                                   \
				  bit= sel2bit[sel];                                   \
				} else sel= 0; /* what a waste! */                        \
			} else {                                                    \
			  sel = bit2sel[bit];                                    \
		    bit= sel2bit[sel];                                     \
			}                                                           \
			UPDATE_GROUP(bit,selsize,elems,i)                          \
      WORD_ZERO_ENCODE(sel,selsize);                              \
			if (bit) {                                               \
        for ( ; elems ; elems--, i++)   /* encoding d-gaps */     \
          WORD_ENCODE(a[i],bit);                               \
			} else {                                                    \
				i += elems;                                               \
			}                                                           \
      WORD_ENCODE_WRITE /* flush the word */                      \
    }                                                             \
		BLKSTATS(stderr,max_bits)                                     \
}

  


#define SIMPLE_ENCODE_START(f) {			  \
  SIMPLE_CODING_VARS							      \
	BIT_2_SEL                             \
  WORD_ENCODE_START(f)		

/* Finish encoding, writing working word to output */
#define SIMPLE_ENCODE_END				   		  \
  WORD_ENCODE_END 							        \
}
 

/* ======================= MACROS FOR DECODING ===== */

#define SIMPLE_DECODE_START(f) 			    \
{   									                  \
  SIMPLE_CODING_VARS;							      \
  register I4  __wbits, __sel, __elems=0; \
  WORD_DECODE_START(f);	
  
#define SIMPLE_BLOCK_DECODE_START(n)

#define SIMPLE_DECODE_END						\
  WORD_DECODE_END;							\
}

#define SIMPLE_DECODE_GET_SELECTOR					\
    GET_NEW_WORD                            \
		SIMPLE_WORD_ZERO_DECODE(__sel,4)        \
		__wbits= sel2bit[__sel];                \
		__elems= sel2elems[__sel];


#define SIMPLE_DECODE(x)						      	\
{									                          \
  if (!__elems)	{   		             		    \
    SIMPLE_DECODE_GET_SELECTOR	  					\
  }	                         								\
	SIMPLE_WORD_DECODE(x,__wbits)             \
	__elems--;                                \
}


// skips "_n" elems in the decoding stream
#define SIMPLE_SKIP(_n)	      	    	               \
{									                               \
	register I4 n= (_n), k;                           \
	if (!__elems) {                                     \
	  SIMPLE_DECODE_GET_SELECTOR                      \
	}                                                 \
	while (n) {                                        \
		if (__elems>=n) {                                 \
			n *= __wbits;                                \
			__wval >>= n;                                \
			__elems -= n;                                  \
			break;                                       \
		}                                              \
		n -= __elems;                                    \
		SIMPLE_DECODE_GET_SELECTOR                       \
	}                                                  \
} 


/* ------------------ Decoding Interface -------------------- */

#define SPECIFIC_DECODING_VARS
#define SCHEME_DECODE_START(f) SIMPLE_DECODE_START(f)

#define GET_PARAMETERS SIMPLE_BLOCK_DECODE_START(n)
#define E_DECODE SIMPLE_DECODE(a[i])


#define STD_E_BLK_DECODE {                      \
  __elems=0; \
  for (i=0; i<n; i++)                         \
		SIMPLE_DECODE(a[i])              \
}


#ifdef FAST_DECODE
  #include "simple_unpack_32.h"
  #include "simple_unpack_64.h"
  #define E_BLK_DECODE {                     \
    UINT *p= a;                              \
    while (p<a+n)                            \
	    SIMPLE_UNPACK(p)                       \
  }



#else
/* just satisfying you, but not really faster than STD_E_BLK_DECODE 
   NOTE: you might get out of a[] - prepare for that! */
#define E_BLK_DECODE {                      \
  for (i=0; i<n; ) {                        \
	  /* SIMPLE_DECODE_GET_SELECTOR  */       \
    GET_NEW_WORD                            \
		SIMPLE_WORD_ZERO_DECODE(__sel,4)        \
		__wbits= sel2bit[__sel];                \
		__elems=sel2elems[__sel];          \
		while (__elems--) {  \
		  SIMPLE_WORD_DECODE(a[i++],__wbits)    \
		}                                       \
  }                                         \
}

#endif


#define SCHEME_SKIP(_n) SIMPLE_SKIP(_n)

#define SCHEME_DECODE_END SIMPLE_DECODE_END

/* ------------------ Encoding Interface -------------------- */

#define SCHEME_ENCODE_START(f)                       \
SIMPLE_ENCODE_START(f)                               

#define SPECIFIC_ENCODING_VARS       \
  U1 *bits;                          \
	I4 elems;                  \
	if (! (bits=(U1*)malloc(sys->epb*sizeof(U1)))) {  \
	  fprintf (stderr, "Out of memory in encoding\n"); \
		exit(1);                         \
	}

#define ENCODE_PARAMETERS                                        \
  I4 max_bits = CalcMinBits(a,bits,n);                           \
	if (max_bits > MAX_BIT) {                                       \
	  fprintf(stderr,"ERROR in SIMPLE: Elem too big with bit size= %d\n",max_bits); \
		exit(1);                                                      \
	}                                                               \


#define SCHEME_ENCODE_END(f) SIMPLE_ENCODE_END

#endif /* ifdef SIMPLE */

#endif
	
