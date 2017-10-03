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



#ifndef _U4D_CODERH
#define _UFD_CODERH

#ifdef UFORDELTA  /* this is specific for w64 */

/* Definition: PForDelta
 
   FAST_DECODE resembles the implementation of Zhang et al; it
	   also won't check for the end of buffer;
	 Other options is not as fast as FAST_DECODE due to:
	   - control of loop
     - need to control the buffer

  
-----------------------------------------------------  */

#include "word_coder.h"
#include "defs.h"
#include "qlog2.h"

#define B_BITS 8



I4
CalcMinBits(INT *gaps, UCHAR *bits, I4 n);


/* NOT USED 0 selector */

#if WORD_SIZE==32
#define COMP_METHOD "u4d4"
#else
#define COMP_METHOD "u4d8"
#endif

#define U4D_CODING_VARS I4 b, exc_size, num_exc; I4 *excs;


#ifdef FAST_DECODE
  #define P4D_ENCODE_ELEM(_X,_B) WORD_CROSS_ENCODE(_X,_B)
  #define P4D_DECODE_ELEM(_X,_B) WORD_CROSS_DECODE(_X,_B)
	#define P4D_S32_DECODE(_X,_B)  WORD_DECODE(_X,_B)
	#define ENCODE_ELEMS_START WORD_ENCODE_WRITE
	#define DECODE_ELEMS_START GET_NEW_WORD
	#define ENCODE_EXER_START {                        \
	  U1 *pe1; U2 *pe2; U4 *pe4; I4 len;               \
	  if (__wremaining < WORD_SIZE) WORD_ENCODE_WRITE  \
		if (exc_size==8) pe1= (U1 *) ebuf;               \
		if (exc_size==16) pe1= (U2 *) ebuf;              \
		if (exc_size==32) pe1= (U4 *) ebuf;             

	#define P4D_ENCODE_EXER(_X,_B) switch(_B) {       \
	  case 8: *pe1++= (U1) (_X); break;               \
	  case 16: *pe2++=(U2) (_X); break;               \
	  case 32: *pe4++=(U4) (_X); break;               \
	}
	#define ENCODE_EXER_END                        \
	  len= exc_size==8? (UWORD *) pe1 - ebuf + 1 :        \
		    (exc_size==16? (UWORD*)pe2-ebuf+1:(UWORD*)pe4-ebuf+1);  \
		if (fwrite(ebuf,sizeof(*ebuf),len,__wfile) );		 \
	}

	#define DECODE_EXER_START {                        \
	  U1 *pe1; U2 *pe2; U4 *pe4;                       \
		if (exc_size==8) pe1= (U1 *) __wpos;               \
		if (exc_size==16) pe1= (U2 *) __wpos;              \
		if (exc_size==32) pe1= (U4 *) __wpos;               
	#define P4D_ENCODE_EXER(_X,_B) switch(_B) {       \
	  case 8: _X= *pe1++; break;                      \
	  case 16:_X= *pe2++; break;                      \
	  case 32:_X= *pe4++; break;                      \
	}
	#define DECODE_EXER_END                            \
	  __wpos += (exc_size==8? (UWORD *) pe1 - __wpos + 1 :        \
		    (exc_size==16? (UWORD*)pe2-__wpos+1:(UWORD*)pe4-__wpos+1));  \
	}
#else
  #ifdef FASTER_DECODE
    #define P4D_ENCODE_ELEM(_X,_B) WORD_ENCODE(_X,_B)
    #define P4D_DECODE_ELEM(_X,_B) WORD_DECODE(_X,_B)
		#define P4D_S32_DECODE(_X,_B)  WORD_DECODE(_X,_B)
		#define ENCODE_ELEMS_START
		#define DECODE_ELEMS_START
		#define ENCODE_EXER_START
	  #define P4D_ENCODE_EXER(_X,_B) WORD_ENCODE(_X,_B)
		#define ENCODE_EXER_END
		#define DECODE_EXER_START
	  #define P4D_DECODE_EXER(_X,_B) WORD_DECODE(_X,_B)
		#define DECODE_EXER_END
  #else
    #define P4D_ENCODE_ELEM(_X,_B) WORD_CROSS_ENCODE(_X,_B)
    #define P4D_DECODE_ELEM(_X,_B) WORD_CROSS_DECODE(_X,_B)
		#define ENCODE_ELEMS_START
		#define DECODE_ELEMS_START
		#define ENCODE_EXER_START
	  #define P4D_ENCODE_EXER(_X,_B) WORD_CROSS_ENCODE(_X,_B)
		#define ENCODE_EXER_END
		#define DECODE_EXER_START
		#define P4D_DECODE_EXER(_X,_B) S32_WORD_CROSS_DECODE(_X,_B)
		#define DECODE_EXER_END
	#endif
#endif


#define E_BLK_ENCODE                                              \
if (n) {                                                          \
    I4 bs[33];                                         \
		I4 max_std_size,sum,j,first, first_exc, exc_step,usedbits=0;  \
		UWORD ebuf[128];                    \
    b = CalcMinBits(a,bits,n);                                    \
		/* 1. bs[i]= number of elems of exact size i                  \
		      exc_size= size for exceptional elems, can be 8, 16, or 32 */ \
		for (i=0; i<33; i++) bs[i]=0;                                 \
		exc_size=1;                                                      \
    for (i=0; i<n; i++) {                                         \
		  bs[bits[i]]++;                                              \
			if (bits[i]>exc_size) exc_size = bits[i];                   \
		}                                                             \
		exc_size= exc_size<=8? 8 :(exc_size<=16? 16 : 32);            \
		/* 2. max_std_size is at least 90% of n                       \
		      b - the bitsize that cover at least 90% elems */        \
		max_std_size= (I4) (n * 0.9 + 0.99); /* 90 % */                   \
		for (i=0, sum=0; sum<max_std_size && i<33; i++)               \
		  sum += bs[i];                                               \
		b= i>1? i-1 : 1;                                              \
		b= n<sys->epb? 31 : b;  /* ad-hoc solution */   \
		if (0) fprintf(stderr,"==> max_std_size= %d ib=%d sum= %d i=%d\n",max_std_size,b,sum,i); \
		/* we'll be in trouble with exceptional list if b==0 */ \
		if (sum < n && b<31) {                                               \
		  exc_step= 1<<b; /* exc_step is max distance between two adjacent exc */ \
		  /* 3. build "excs[]" as list of index to exceptions */   \
		  first= -1;                                                    \
      for (i=0,j=0; i<n; i++) {                                     \
		    if (bits[i]>b || (i-first >= exc_step)) { \
			    /* fprintf(stderr,"j=%d i=%d bitsi=%d ",j,i,bits[i]); */ \
			    excs[j++]= i;                                             \
				  /* fprintf(stderr," excs= %d\n",excs[j-1]); */ \
				  first= i;                                                 \
			  }                                                           \
		  }                                                             \
      sum= j; /* sum now reused for num of excs */                  \
		} else sum=0;                                                   \
		if (sum) {                                                    \
		  excs[sum]= excs[sum-1]+1;                                   \
			first_exc= excs[0];                                         \
		} else first_exc= n+1;                                        \
		/* 4. turns excs[] to list of exceptional values              \
		             a[] to include exc index             */          \
		for (j=0; j<sum; j++) {                                       \
		  first= excs[j];                                             \
			excs[j]= a[first];                                          \
		  a[first]= excs[j+1] - first;                                \
		}                                                             \
		/* 5. coding, first elems always within a word  */            \
		WORD_ENCODE(b,7)                                         \
		WORD_ZERO_ENCODE(sum,sys->epb_bits)                \
		if (sum) {                                                    \
		  WORD_ZERO_ENCODE(first_exc,b)                            \
		  j= exc_size==8? 1 : (exc_size==16?  2 : 3);                   \
		  WORD_ENCODE(j,2)                                              \
		}                                                               \
		ENCODE_ELEMS_START                                            \
	/*	fprintf(stderr,"b=%d exer=%d first=%d exc_size=%d n=%d\n",b,sum,first_exc,exc_size,n); */ \
		for (i=0; i<n; i++) {                                         \
		  P4D_ENCODE_ELEM(a[i],b)                                   \
		}                                                             \
		if (sum) {                                                    \
		ENCODE_EXER_START                                             \
		for (i=0,j=0; i<sum; i++) {                                       \
		  /*j += excs[i];  \
			if (j>=n) {    \
			  I4 *p=0;     \
				fprintf(stderr,"Trouble in u4d encode: i=%d j=%d\n",i,j); \
				*p=0;        \
			} */             \
		  P4D_ENCODE_EXER(excs[i],exc_size)                         \
		}                                                             \
		ENCODE_EXER_END                                               \
		}                                                             \
		/* 8 bits for block length is calculated as "wasted" */       \
		usedbits= n*b + sum*exc_size + 8;                             \
		/* forces  writing */                                         \
		if (__wremaining < WORD_SIZE) WORD_ENCODE_WRITE               \
		DIRECT_UPDATE_GROUP(b,24,n,0,usedbits)                        \
		BLKSTATS(stderr,b)                                            \
}


/* ======================= MACROS FOR ENCODING ===== */

#define U4D_ENCODE_START(f) {			  \
  U4D_CODING_VARS							      \
  WORD_ENCODE_START(f)		          \
  Tmalloc(excs,I4,sys->epb); 


/* Finish encoding, writing working word to output */
#define U4D_ENCODE_END				   		  \
  free(excs);                          \
  WORD_ENCODE_END 							        \
}
 

/* ======================= MACROS FOR DECODING ===== */

#define U4D_DECODE_START(f) 			    \
{   									                  \
  U4D_CODING_VARS;							      \
  WORD_DECODE_START(f);	
  

#define U4D_DECODE_END						\
  WORD_DECODE_END;							\
}

/* ------------------ Encoding Interface -------------------- */

#define SCHEME_ENCODE_START(f)                       \
U4D_ENCODE_START(f)                               

#define SPECIFIC_ENCODING_VARS       \
  U1 *bits;                          \
	if (! (bits=(U1*)malloc(sys->epb*sizeof(U1)))) {  \
	  fprintf (stderr, "Out of memory in carry encoding\n"); \
		exit(1);                         \
	}

#define SCHEME_ENCODE_END(f) U4D_ENCODE_END


/* ------------------ Decoding Interface -------------------- */

#define SPECIFIC_DECODING_VARS
#define SCHEME_DECODE_START(f) U4D_DECODE_START(f)


#define E_DECODE WORD_CROSS_DECODE(a[i],b)  


#ifndef _WORD_ALIGN_C_
#ifdef FAST_DECODE
  // Notes: this part supposes that the whole file has been
	//        fetched to the buffer. Beware!
  #include "unpack_32.h"
  #include "unpack_64.h"
#define E_BLK_DECODE if (n) {                               \
    I4 first, tmp;                                          \
		WORD_DECODE(b,7)                                        \
		WORD_ZERO_DECODE(num_exc,sys->epb_bits)      \
		if (num_exc) {                                          \
		  WORD_ZERO_DECODE(first,b)                             \
		  WORD_DECODE(tmp,2)                                    \
		  exc_size= tmp==1? 8 : (tmp==2? 16 : 32);              \
		}                                                       \
		/* fprintf(stderr,"b=%d exer=%d first=%d exc_size=%d n=%d\n",b,num_exc,first,exc_size,n); */ \
		__wpos= unpacks[b](a,__wpos);                           \
		/* not much can be improved for the next few lines */   \
		__wremaining= 0;                          \
		DECODE_EXER_START                                       \
		for (i=0; i<num_exc; i++) {                             \
		  tmp= first;                                           \
			first += a[tmp];                                      \
		  P4D_DECODE_EXER(a[tmp],exc_size)                      \
		}                                                       \
		DECODE_EXER_END                                         \
		__wremaining= 0;                   \
}
#else

/* at the beginning of E_BLK_DECODE, selector already decoded 
   also note that block begin frm word border */
#define E_BLK_DECODE if (n) {                               \
    I4 first, tmp;                                          \
		WORD_DECODE(b,7)                                        \
		WORD_ZERO_DECODE(num_exc,sys->epb_bits)      \
		if (num_exc) {                                          \
		  WORD_ZERO_DECODE(first,b)                             \
		  WORD_DECODE(tmp,2)                                    \
		  exc_size= tmp==1? 8 : (tmp==2? 16 : 32);              \
		}                                                       \
		DECODE_ELEMS_START                                      \
		for (i=0; i<n; i++) {                                   \
		  P4D_DECODE_ELEM(a[i],b)                               \
		}                                                       \
		if (num_exc) { DECODE_EXER_START                         \
		for (i=0; i<num_exc; i++) {                             \
		  tmp= first;                                           \
			first += a[tmp];                                      \
		  P4D_DECODE_EXER(a[tmp],exc_size)                      \
		} DECODE_EXER_END }                                     \
		__wremaining= 0;                                        \
}
#endif

#endif 


#define SCHEME_SKIP(_n) 

#define SCHEME_DECODE_END U4D_DECODE_END

#endif /* ifdef UFORDELTA */

#endif
	
