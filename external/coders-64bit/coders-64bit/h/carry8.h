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


/* carry8.h :- part of carry.h, for 64 bit  */  
 

/* 
   carry8 :- a 64-bit version of carryover-12
	           However, like simple, it has ABSOLUTE selector, whose
						 value does not depend on that of the previous.
						 But, unlike simple, two selectors can be located in
						 one word. The first selector is at the less-significant
						 end of words. We can force the second selector to the
						 most-significant end if we want fast forward search.

-----------------------------------------------------  */

#define COMP_METHOD "carry8"

#define CODE_TABLE                                                  \
/* sel2bitS/E/B : S(tandard), for 60-bit data, E(xtended) 64-bits,  \
      The 1st word is special: 3bit base, then 4 bit selector,      \
		  bitsize taken from sel2bitS, and elems must be calculated */  \
/*                    0     1   2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  20  21 */  \
  UCHAR sel2bitS[]=  {0  ,  0,  1, 2, 3, 4 ,5, 6, 7, 8, 9,10,11,12,14,15,18,20,28,30, 56, 60};   \
  UCHAR sel2elemsS[]={255,128, 60,30,20,15,12,10, 8, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2,  1,  1};    \
	UCHAR selinprevS[]={  1,  1,  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,  1,  0};    \
/*                    0     1   2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  20  21 */  \
	UCHAR sel2bitE[]=  {  0,  0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,12,15,16,20,21,30,32, 60, 64};     \
	UCHAR sel2elemsE[]={255,180,120,64,32,21,16,12,10, 9, 8, 7, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1};       \
	UCHAR selinprevE[]={  1,  1, 1, 0, 0, 0, 0,  1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0};

#define BIT_2_SEL CHAR bit2selS[]=                                    \
        { 0, 2, 3, 4, 5, 6, 7, 8,   9,10,11,12,13,14,14,15,           \
				 16,16,16,17,17,18,18,18,  18,18,18,18,18,19,19,20,           \
				 20,20,20,20,20,20,20,20,  20,20,20,20,20,20,20,20,           \
				 20,20,20,20,20,20,20,20,  20,21,21,21,21,-1,-1,-1, -1};      \
                  CHAR bit2selE[]=                                    \
        { 0, 3, 4, 5, 6, 7, 8, 9,  10,11,12,13,13,14,14,14,           \
				 15,16,16,16,16,17,18,18,  18,18,18,18,18,18,18,19,           \
				 19,20,20,20,20,20,20,20,  20,20,20,20,20,20,20,20,           \
				 20,20,20,20,20,20,20,20,  20,20,20,20,20,21,21,21, 21};
				
		
				 
#define MAX_BIT 64
#define BASE_BIT 3   /* bit to code the base of coding tables,
                       note that base can be in 0..6 */


#define CARRY_CODING_VARS CODE_TABLE                             \
       register I4  __wbits, __sel, __elems=0;                    \
       I4 selsize = 4, base;                                      \
	     I4 selinprev;       /* YES if sel in prev word  */      \
			 UCHAR *selinprevs, *selinprev60, *selinprev64;             \
			 UCHAR *sel2bit60, *sel2elems60, *sel2bit64, *sel2elems64;  \
       UCHAR *bit2sel, *sel2bit, *sel2elems; /* the "current" coding table  */

#ifndef _NO_TOTAL_ELEMS
  #define BLK_ENC_ADJUST 
#else
  #define BLK_ENC_ADJUST WORD_ENCODE_WRITE
#endif

#define DEC_SET_BASE_SEL                          \
	sel2bit= sel2bit60= sel2bitS+base;              \
	sel2elems= sel2elems60= sel2elemsS+base;        \
	sel2bit64= sel2bitE+base;                       \
	sel2elems64= sel2elemsE+base;                   \
	selinprevs= selinprev60= selinprevS+base;       \
	selinprev64 = selinprevE + base; 


#define SET_BASE_SEL                              \
  DEC_SET_BASE_SEL                                \
	bit2sel= bit2selS;

#define DEC_SET_BASE_SEL_60                       \
	sel2bit= sel2bit60;                             \
	sel2elems= sel2elems60;                         \
	selinprevs= selinprev60;

#define SET_BASE_SEL_60                           \
  DEC_SET_BASE_SEL_60                             \
	bit2sel= bit2selS;

#define DEC_SET_BASE_SEL_64                       \
	sel2bit= sel2bit64;                             \
	sel2elems= sel2elems64;                         \
	selinprevs= selinprev64;

#define SET_BASE_SEL_64                           \
  DEC_SET_BASE_SEL_64                             \
	bit2sel= bit2selE;


#ifdef FAST_DECODE
  #define ENCODE_BASE {                            \
	  WORD_ZERO_ENCODE(base,BASE_BIT)                \
		WORD_ENCODE_WRITE                              \
	}
  #define DECODE_BASE                             \
	  base= *__wpos++ & __mask[BASE_BIT];
	#define DATA_SIZE WORD_SIZE - (selinprev? 0 : selsize)
#else
  #define ENCODE_BASE WORD_ZERO_ENCODE(base,BASE_BIT)
  #define DECODE_BASE SIMPLE_WORD_ZERO_DECODE(base,BASE_BIT)
	#define DATA_SIZE WORD_SIZE - (i==0? BASE_BIT : 0) - (selinprev? 0 : selsize)
#endif

/* ======================= MACROS FOR ENCODING ===== */
#define E_BLK_ENCODE                                              \
{                                                                 \
  I4 sel, bit,tmp, base;                                        \
  BLK_ENC_ADJUST                                                \
  sel= 15;                                                      \
  base= 0;                                                      \
	selinprev= 0;       /* not having sel in prev word  */       \
  while (sel2bitS[sel] < max_bits) {base++; sel++;}              \
	ENCODE_BASE                                                   \
	SET_BASE_SEL                                                  \
  for (i=0; i<n; ) {                                            \
		avail= DATA_SIZE;                                           \
		j= i;                                                       \
		sel= bit2sel[bits[j]] -base;                                \
		if (sel<0) sel=0;                                           \
		bit= sel2bit[sel];                                          \
		elems= 1;                                                   \
		while (++j<n) {                                             \
			if ((avail-elems*bit<bit) || elems==sel2elems[sel]) break;    \
			if (bits[j] > bit) {                                      \
				tmp= bit2sel[bits[j]] - base ;                          \
				if ( (elems+1)* sel2bit[ tmp ] <= avail )  {            \
					sel= tmp;                                             \
				  bit= sel2bit[sel];                                    \
			  } else {                                                \
				  while ( (elems+1)*sel2bit[sel] <=avail ) sel++;       \
					bit= sel2bit[sel];                                    \
					elems= avail/bit;                                     \
					break;                                                \
				}                                                       \
			}                                                         \
			elems++;                                                  \
		}                                                           \
		if (bit==0) { /* can be downgrade to bit=1 */               \
			if (i+elems<n) {                                          \
			  for (sel=0; sel2elems[sel] > elems; sel++);             \
				bit= sel2bit[sel];                                      \
				if (bit==0)                                             \
					elems=sel2elems[sel];                                 \
				else                                                   \
				  elems= avail/bit;                                    \
			} else sel= 0; /* what a waste! */                       \
		} else {                                                   \
			sel = bit2sel[bit] - base ;                                      \
		  bit= sel2bit[sel];                                       \
		}                                                          \
		UPDATE_GROUP(bit,selsize,elems,i)                          \
    WORD_ZERO_ENCODE(sel,selsize)                              \
		if (selinprev) {                                             \
			WORD_ENCODE_WRITE                                        \
		}                                                          \
		if (bit)                                                  \
      for ( ; elems && i<n; elems--, i++)     \
        WORD_ENCODE(a[i],bit)                               \
		else i += elems;                                             \
		if (__wremaining>=selsize) {                              \
		  if (i>=n) {                                             \
			  WORD_ENCODE_WRITE                                     \
			} else {                                                \
			  selinprev= 1;                                         \
        SET_BASE_SEL_64                                       \
		  }                                                       \
		} else {                                                  \
      WORD_ENCODE_WRITE                                       \
			selinprev= 0;                                            \
      SET_BASE_SEL_60                                        \
		}                                                          \
  }                                                             \
  BLKSTATS(stderr,max_bits)                                     \
}


/* ======================= MACROS FOR DECODING ===== */

  
#define CARRY_BLOCK_DECODE_START(n)

#define GET_SELECTOR              					\
		SIMPLE_WORD_ZERO_DECODE(__sel,4)        \
		__wbits= sel2bit[__sel];                \
		__elems= sel2elems[__sel];

#define CARRY_DECODE_GET_SELECTOR           \
  if (selinprev) {                          \
	  GET_SELECTOR                            \
		GET_NEW_WORD                            \
	} else {                                  \
	  GET_NEW_WORD                            \
		GET_SELECTOR                            \
	}              

#define DEC_UPDATE_BASE                     \
  if (selinprevs[__sel] != selinprev) {     \
	  if (selinprev) {                        \
		  selinprev= 0;                         \
		  DEC_SET_BASE_SEL_60;                  \
		} else {                                \
		  selinprev= 1;                         \
		  DEC_SET_BASE_SEL_64;                  \
		}                                       \
	}                                         


#ifdef FAST_DECODE
#include "carry_unpack_60.h"
#include "carry_unpack_64.h"
#define E_BLK_DECODE {                      \
  /* a bit of ugly code for the first word \
	   of each block :-( */                   \
  UINT *p= a;                               \
  DECODE_BASE                               \
	selinprev=0;                              \
  while (p<a+n) {                           \
    if (selinprev) {                        \
	    CARRY_UNPACK_64(p)                    \
	  } else {                                \
		  __sel= *__wpos & __mask[4];           \
	    CARRY_UNPACK_60(p)                    \
	  }                                       \
	}                                         \
}
#else
#define E_BLK_DECODE {                      \
  /* a bit of ugly code for the first word \
	   of each block :-( */                   \
  GET_NEW_WORD                              \
  DECODE_BASE                               \
	DEC_SET_BASE_SEL                          \
	GET_SELECTOR                              \
	if (__wbits) {                            \
	  __elems=(WORD_SIZE-BASE_BIT-4)/__wbits; \
	  for (i=0; i<__elems; i++) {             \
	    SIMPLE_WORD_DECODE(a[i],__wbits)      \
		}                                       \
		selinprev=                              \
		  WORD_SIZE-BASE_BIT-4 - __elems*__wbits\
			>= 4? 1 : 0;                          \
	} else {                                  \
	  for (i=0; i<__elems; i++) a[i]= 1;      \
		selinprev= 1;                           \
	}                                         \
	if (selinprev) {                          \
	  DEC_SET_BASE_SEL_64                     \
	}                                         \
	/* and now, routine */                    \
  while (i<n) {                             \
	  CARRY_DECODE_GET_SELECTOR               \
		while (__elems-- ) {                     \
		  SIMPLE_WORD_DECODE(a[i++],__wbits)    \
		}                                       \
		DEC_UPDATE_BASE                         \
  }                                         \
}
#endif

#define CARRY_DECODE(_x)

/* ------------------ Encoding Interface -------------------- */

#define SPECIFIC_ENCODING_VARS       \
  U1 *bits;                          \
	I4 avail, elems;                  \
	if (! (bits=(U1*)malloc(sys->epb*sizeof(U1)))) {  \
	  fprintf (stderr, "Out of memory in encoding\n"); \
		exit(1);                         \
	}

#define ENCODE_PARAMETERS                                        \
  I4 max_bits = CalcMinBits(a,bits,n);                           \
	if (max_bits > MAX_BIT) {                                       \
	  fprintf(stderr,"ERROR in CARRY: Elem too big with bit size= %d\n",max_bits); \
		exit(1);                                                      \
	}                                                               \


