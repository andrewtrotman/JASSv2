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
  carry4.h = part of carry.h, for 32-bit
 
*/


/* constant */
#define TRANS_TABLE_STARTER 33

#define COMP_METHOD "carry4"

/* ========================================================
 Coding variables:
   trans_B1_30_big[], trans_B1_32_big are left and right transition
     tables (see the paper) for the case when the largest elements 
     occupies more than 16 bits.
   trans_B1_30_small[], trans_B1_32_small are for the otherwise case

   __pc30, __pc32 is points to the left, right tables currently used
   __pcbase points to either __pc30 or __pc32 and represents the
     current transition table used for coding
   ======================================================== */ 

#define CARRY_CODING_VARS					                         	\
UCHAR *__pcbase;						                        \
        /* point to current transition table 		  */		    \
register I4  __wbits= TRANS_TABLE_STARTER;                  \
I4 selinprev;                                               \
CARRY_TABLES


#define CARRY_TABLES /* unmodified and can be global */           \
/* *_big is transition table for the cases when number of bits		\
 needed to code the maximal value exceeds 16.				              \
 *_small are used otherwise.						                          \
 NOTE: this version does not work for gaps exceeding 2^28         \
*/									                                              \
UCHAR __pc30[]={					                        \
	 0,0,0,0, 1,2,3,28, 1,2,3,28, 2,3,4,28, 3,4,5,28, 4,5,6,28, 	\
	 5,6,7,28, 6,7,8,28, 6,7,10,28, 8,10,15,28, 9,10,14,28, 	\
	 0,0,0,0, 0,0,0,0, 0,0,0,0, 10,15,16,28, 10,14,15,28, 		\
	 7,10,15,28, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,          \
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,             \
   0,0,0,0, 0,0,0,0, 6,10,16,28, 0,0,0,0, 0,0,0,0,          \
    0,0,0,0, 0,0,0,0, 4,9,15,28}; 	\
	 								\
UCHAR __pc32[]={					\
 	 0,0,0,0, 1,2,3,28, 1,2,3,28, 2,3,4,28, 3,4,5,28, 4,5,6,28,	\
	 5,6,7,28, 6,7,8,28, 7,9,10,28, 7,10,15,28, 8,10,15,28,		\
	 0,0,0,0, 0,0,0,0, 0,0,0,0, 7,10,15,28, 10,15,16,28,		\
	 10,14,15,28, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,	\
	 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		\
	 6,10,16,28, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 4,10,16,28};	\
	 								\
 





/* ======================= MACROS FOR carryover12 ENCODING ===== */


#define CARRY_BLOCK_ENCODE_START(n,max_bits)				

#define CARRY_BLOCK_DECODE_START(n)	              				\
{					                                        				\
  __pcbase= __pc30;						                            \
	__wbits= TRANS_TABLE_STARTER;                         	\
	selinprev= 0;                                           \
}  						


/* ======================= MACROS FOR carryover12 DECODING ===== */

  
#ifndef FAST_DECODE
#define CARRY_DECODE_GET_SELECTOR			            		\
    if (__wremaining>=2) {			                   		\
      __pcbase= __pc32;		                  					\
      __wbits= __pcbase[(__wbits<<2)+(__wval & 3)];	  \
	    GET_NEW_WORD			                      				\
	    __wremaining = WORD_SIZE;			    	        		\
    }	else {				                                	\
      __pcbase= __pc30;	                   						\
      GET_NEW_WORD				                      			\
      __wbits= __pcbase[(__wbits<<2)+(__wval & 3)];		\
      __wval >>= 2;					                      		\
      __wremaining = WORD_SIZE-2; 				        		\
    }								

#define E_BLK_DECODE                    \
{                                       \
  for (i=0; i<n; i++) {                 \
    if (__wremaining < __wbits)	{	  		\
      CARRY_DECODE_GET_SELECTOR			 		\
    }				                    				\
		WORD_DECODE(a[i],__wbits)           \
	}                                     \
	__wremaining=0;                       \
}

#define CARRY_DECODE(x)

#else

#include "carry_unpack_30.h"
#include "carry_unpack_32.h"

#define E_BLK_DECODE		      		    	\
{								                      	\
  UINT *p= a;                           \
	while (p<a+n) {                       \
    if (selinprev) {                      \
	    CARRY_UNPACK_32(p)                \
		} else {                            \
      CARRY_UNPACK_30(p)                \
		}                                   \
	}                                     \
}

#define CARRY_DECODE(x)

#endif

// skips "_n" elems in the decoding stream
#define CARRY_SKIP(_n)	      	    	               \
{									                                   \
	register I4 n= (_n), k;                           \
	while (n) {                                        \
		if (k= __wremaining/__wbits) {                   \
			if (k>=n) {                                    \
				n *= __wbits;                                \
				__wval >>= n;                                \
				__wremaining -= n;                           \
				break;                                       \
			}                                              \
			n -= k;                                        \
			k *= __wbits;                                  \
			__wval >>= k;                                  \
			__wremaining -= k;                             \
		}                                                \
		CARRY_DECODE_GET_SELECTOR                        \
	}                                                  \
} 



/* ===== interface for w64 ====== the above part should be as in mt ==== */

/* ------------------ Encoding Interface -------------------- */


#define SPECIFIC_ENCODING_VARS       \
  U1 *bits;                          \
	U4 size, max_bits;                 \
	U1 *table, *base;                  \
	U4 avail, elems;                  \
	if (! (bits=(U1*)malloc(sys->epb*sizeof(U1)))) {  \
	  fprintf (stderr, "Out of memory in carry encoding\n"); \
		exit(1);                         \
	}

#define ENCODE_PARAMETERS                                         \
max_bits= CalcMinBits(a,bits,n);                                  \
CARRY_BLOCK_ENCODE_START(n,max_bits);                             \


#define E_BLK_ENCODE                                              \
{                                                                 \
  I4 k;                                                           \
  size = TRANS_TABLE_STARTER;                                     \
  selinprev=0;                                                    \
    for (i=0; i<n; ) {                                            \
			table= selinprev? __pc32 : __pc30;                          \
			avail= selinprev? 32 : 30;                                  \
			                                                            \
      base= table+(size<<2);       /* row in trans table */       \
                                                                  \
      /* 1. Modeling: Find j= the first-fit column in base */	    \
      for (j=0; j<4; j++) {                                       \
        size = base[j];                                           \
				elems= avail/size;                                        \
				if (elems>n-i) elems= n-i;                                \
				for (k=0; k<elems; k++)                                   \
				  if (bits[i+k] > size) break;                            \
				if (k==elems) break;                                      \
      }                                                           \
                                                                  \
			UPDATE_GROUP(size,2,elems,i)                                \
      /* 2. Coding: Code elements using row "base" & column "j" */\
      WORD_ENCODE(j+1,2);             /* encoding column */       \
			if (selinprev) WORD_ENCODE_WRITE                            \
      for ( ; elems ; elems--, i++)   /* encoding d-gaps */       \
        WORD_ENCODE(a[i],size);                                   \
			selinprev= __wremaining > 1;                                \
			if (!selinprev) WORD_ENCODE_WRITE                           \
    }                                                             \
		if (__wremaining<WORD_SIZE) WORD_ENCODE_WRITE                  \
		BLKSTATS(stderr,max_bits)                                     \
}

