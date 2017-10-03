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


/* look for non-zero version at slide.nonzero.h */

#ifdef _EXPERIMENT
#include "slide_exp.h"

#else

#ifndef _SLIDE_H
#define _SLIDE_H

#ifdef SLIDE    

/* Definition: SIMPLE is the naive version with the properties:
   1. 4-bit absolute selector which is always at the end of machine word
	 2. Coding always begin from the word border
	 3. zero-bit size is possible, and used when the zero pattern length
	    l satisfies  28[or 60] <= l <= 65536   
-----------------------------------------------------  */



#include "word_coder.h"
#include "defs.h"
#include "qlog2.h"


I4
CalcMinBits(INT *gaps, UCHAR *bits, I4 n);


#define MAX_BIT WORD_SIZE

#if WORD_SIZE==32
#define COMP_METHOD "slide4"
#define MBIT 5
#else
#define COMP_METHOD "slide8"
#define MBIT 6
#endif


#define SLIDE_CODING_VARS                         \
  I4 __sbits, __mbits, __slide, __half, __threshold, __halfthres, __sel,__elems;                               \
	register I4 __wbits;




#define SLIDE_GET_PARMETERS                       \
  __elems=0;                                      \
  __sbits= __mbits>6? 3 : (__mbits>1? 2 : 1);     \
  __slide= (1<<__sbits) - 1;                      \
  __half = __slide>>1;                            \
  __threshold= __mbits - __slide;                 \
  __halfthres= __mbits - __half;                  \
  __wbits= WORD_SIZE+1;  /* so that begin with sel encoding */  


#define SLIDE_ENCODE_START(f)     \
{                                 \
  SLIDE_CODING_VARS               \
  WORD_ENCODE_START(f);	

/* Finish encoding, writing working word to output */
#define SLIDE_ENCODE_END						\
  WORD_ENCODE_END;							    \
}



#define SLIDE_MIN_MAX(min,max)                       \
	min= __wbits < __half? 0 :                       \
      (__wbits > __halfthres? __threshold : __wbits - __half); \
  max= min + __slide - 1;


// gets __sel at next selector value
// returns number of elems coded
// start..end is the range of bit size
#define GetNextSelector(org_start, end) \
{                                         \
	I4 min, max, curr;                        \
	I4 avail= __wremaining - __sbits;         \
	I4 total_bits;                            \
	U1 *start;                             \
	                                           \
	elems = 1;       /* at least 1 elems in a block anyway */       \
	SLIDE_MIN_MAX(min, max);  /* find min, max usng __wbits etc */  \
	curr = min < *(org_start)? *(org_start) : min;   /* first elem encountered */ \
	if (curr>max) curr= __mbits;               \
                                             \
	if (avail < curr) avail += WORD_SIZE;      \
                                             \
start_calc:                                  \
	start= (org_start)+1;                      \
	total_bits= avail;                         \
	avail -= curr;                             \
	elems= 1;                                  \
                                             \
	while (avail>=curr && start<=(end)) {      \
		if (curr >= *start) {                    \
			elems++; start++; avail-= curr;        \
			if (elems >= MIN_ZERO_PATTERN && curr==0) {   \
				while(*start==curr && start<=(end) && elems<MAX_ELEM_PER_WORD) {   \
					elems++; start++;                  \
				}                                    \
				break;                               \
			}                                      \
			continue;                              \
		}                                        \
		                                         \
		curr= *start;                            \
		if (curr>max) curr= __mbits;             \
		if (total_bits < curr) {avail= total_bits+WORD_SIZE; goto start_calc;}  \
		                                         \
		if (total_bits - curr*(elems+1) >= 0) {  \
			avail= total_bits - curr*(++elems);    \
			start++;                               \
			continue;                              \
		}                                        \
                                             \
		elems= total_bits/curr;                  \
		break;                                   \
	}                                          \
	__wbits= curr;                             \
	__sel= curr<=max ? curr - min : __slide;   \
	/* fprintf(stderr, "%d ",__wbits); */           \
}                                          
		



#define E_BLK_ENCODE                         \
  if (__mbits) {                             \
  for (i=0; i<n; ) {                         \
	  GetNextSelector(bits+i,bits+n-1)         \
		UPDATE_GROUP(__wbits,__sbits,elems,i)      \
		WORD_ZERO_CROSS_ENCODE(__sel,__sbits);   \
		if (__wbits)                             \
		  while (elems--) {                      \
		    WORD_CROSS_ENCODE(a[i],__wbits);     \
			  i++;                                 \
		  }                                      \
		else {                                   \
		  WORD_CROSS_ENCODE(elems,BIT_FOR_ZERO_LEN);  \
			i += elems;                            \
		}                                        \
	}                                          \
	BLKSTATS(stderr,__mbits)                   \
	/* fprintf(stderr, "\n"); */                     \
	}
/* ======================= MACROS FOR DECODING ===== */


#define SLIDE_DECODE_START(f)                        \
{                                                    \
  SLIDE_CODING_VARS                                  \
  WORD_DECODE_START(f);	                             \
	
/*	GET_NEW_WORD                                       \
	__wremaining= WORD_SIZE;
*/


#define SLIDE_DECODE_GET_SELECTOR					           \
	WORD_ZERO_CROSS_DECODE(__sel,__sbits);             \
  __wbits = __sel==__slide ? __mbits :               \
    (__wbits<__half? __sel :                         \
		(__wbits > __halfthres? __threshold + __sel :    \
		   __wbits - __half + __sel ));        

#define SLIDE_DECODE(x)							                 \
do {									                               \
  if (__wremaining < __wbits) {				               \
    SLIDE_DECODE_GET_SELECTOR		  			             \
		if (!__wbits) {                                  \
			WORD_CROSS_DECODE(__elems,BIT_FOR_ZERO_LEN);   \
			x=1;                                           \
			break;                                         \
		}                                                \
	} else                                             \
	if (!__wbits) {                                    \
		/* fprintf(stderr,"wbits=%d elem=%d i=%d n=%d\n",__wbits,__elems,i,n); */ \
		if (__elems > 1) {                               \
			x= 1; __elems--;                               \
			break;                                         \
		} else if (__elems==0) {                         \
			WORD_CROSS_DECODE(__elems,BIT_FOR_ZERO_LEN);   \
			x=1;                                           \
			break;                                         \
		} /* elems==1 means finish */                    \
		__elems= 0;                                      \
		SLIDE_DECODE_GET_SELECTOR                        \
		if (!__wbits) {                                  \
			WORD_CROSS_DECODE(__elems,BIT_FOR_ZERO_LEN);   \
			x=1;                                           \
			break;                                         \
		}                                                \
	}                                                  \
	WORD_CROSS_DECODE(x,__wbits);                      \
} while(0);

/* hey, this relates to compilation of slide8 in 32-bit vine only;
   for which we got a strange message from gcc */

#define SP_SLIDE_DECODE_GET_SELECTOR 


// skips "_n" elems in the decoding stream: This is for non-zero version
#define SLIDE_SKIP(_n)	                           \
{   									                               \
	register I4 n= (_n), k;                           \
	while (n) {                                        \
	  if (k= __wremaining/__wbits) {                   \
		if (k>=n) {                                      \
			n *= __wbits;                                  \
		  __wval >>= n;                        \
			__wremaining -= n;                             \
			break;                                         \
		}                                                \
		n -= k;                                          \
		k *= __wbits;                                    \
		__wval >>= k;                          \
		__wremaining -= k;                               \
		}                                                \
		SLIDE_DECODE_GET_SELECTOR                     \
		if (__wremaining<__wbits) {                      \
			GET_NEW_WORD                                   \
			__wval >>= (__wbits - __wremaining);           \
			__wremaining += WORD_SIZE - __wbits;           \
			n--;                                           \
		}                                               \
	}                                                  \
} 


#define SLIDE_BLOCK_DECODE_START(n)           		\
{	    				                          		   		\
	WORD_ZERO_CROSS_DECODE(__mbits,MBIT)              \
	SLIDE_GET_PARMETERS                             \
}   						


#define SLIDE_DECODE_END						\
  WORD_DECODE_END						      \
}


  


/* ===== interface for w64 ====== the above part should be as in mt ==== */

/* ------------------ Encoding Interface -------------------- */

#define SCHEME_ENCODE_START(f)                       \
SLIDE_ENCODE_START(f)                               

#define SPECIFIC_ENCODING_VARS       \
  U1 *bits;                          \
	I4 avail, elems;                  \
	if (! (bits=(U1*)malloc(sys->epb*sizeof(U1)))) {  \
	  fprintf (stderr, "Out of memory in carry encoding\n"); \
		exit(1);                         \
	}

#define ENCODE_PARAMETERS {                                       \
  __mbits = CalcMinBits(a,bits,n);                                \
	if (__mbits > MAX_BIT) {                                       \
	  fprintf(stderr,"ERROR in SIMPLE: Elem too big with bit size= %d\n",__mbits); \
		exit(1);                                                      \
	}                                                               \
	WORD_ZERO_CROSS_ENCODE(__mbits,MBIT);                              \
	SLIDE_GET_PARMETERS                                            \
}

#define SCHEME_ENCODE_END(f) SLIDE_ENCODE_END


/* ------------------ Decoding Interface -------------------- */

#define SPECIFIC_DECODING_VARS
#define SCHEME_DECODE_START(f) SLIDE_DECODE_START(f)

#define GET_PARAMETERS SLIDE_BLOCK_DECODE_START(n)
#define E_DECODE SLIDE_DECODE(a[i])

#define E_BLK_DECODE                                            \
if (__mbits)                                                    \
  for (i=0; i<n; i++) E_DECODE                                     \
else                                                            \
  for (i=0; i<n; i++) a[i]=1;                                      

#define SCHEME_SKIP(_n) SLIDE_SKIP(_n)

#define SCHEME_DECODE_END SLIDE_DECODE_END


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif /* ifdef SLIDE */

#endif

#endif /* _EXPERIMENT */
