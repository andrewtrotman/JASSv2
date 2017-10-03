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

/* ---------------------------------------------------------------------- 
 
  word_coder.h -- tools for coding at word levels
	                applied to both 32- and 64-bit architectures

   --------------------------------------------------------------------- */
									

#ifndef _WORD_CODERH
#define _WORD_CODERH

#include "basic_types.h"									
#include "defs.h"



#define word_buffer_size sys->word_buffer_size   /* :-) */

#ifdef _BIT_COUNT_   /* if wanted to follow the bit_so_far values */
                     /* __bs* for bits actually used so far */
										 /* __wbs* for wasted bits (because of alignment,
											         end of stream                       
										    __is* for data item */
  #define BS_VARS U8 __bs=0,__bs_then=0,__wbs=0,__wbs_then=0,  \
	                   __is=0, __is_then=0;
	#define BS_SET __bs_then= __bs;
	#define BS_GET(_X) {_X= __bs-__bs_then; __bs_then=__bs;}
	#define BS_ADD(_X) {_X+= (__bs-__bs_then); __bs_then=__bs;}
	#define BS_GET_ALL(_X) _X=__bs;
	#define BS_UPDATE(_X) {__bs += (_X); __is++; }
	#define IS_SET __is_then= __is;
	#define IS_GET(_X) {_X= __is-__is_then; __is_then=__is;}
	#define IS_ADD(_X) {_X += (__is-__is_then); __is_then=__is;}
	#define IS_GET_ALL(_X) _X=__is;
	#define WBS_SET __wbs_then= __wbs;
	#define WBS_GET(_X) {_X=__wbs-__wbs_then; __wbs_then=__wbs;}
	#define WBS_GET_ALL(_X) _X=__wbs;
	#define WBS_UPDATE(_X) __wbs += (_X);
#else
  #define BS_VARS  
	#define BS_SET 
	#define BS_GET(_X)
	#define BS_ADD(_X)
	#define BS_UPDATE(_X)
	#define BS_GET_ALL(_X) 
	#define IS_SET 
	#define IS_GET(_X)
	#define IS_ADD(_X)
	#define IS_GET_ALL(_X) 
	#define WBS_SET 
	#define WBS_GET(_X)
	#define WBS_UPDATE(_X)
	#define WBS_GET_ALL(_X) 
#endif

#define WORD_CODING_VARS_DEF                             \
I4 __wremaining;                                \
UINT __value[WORD_SIZE], __bits[WORD_SIZE];             \
I4 __pvalue;                                             \
FILE *__wfile;                                           \
/* specific for decoding */                              \
UWORD __wval;				   	                       \
UWORD *__buffer;                        \
UWORD *__buffend;                                        \
UWORD *__wpos;                                           \
UWORD ONE = 1;                                           \
BS_VARS                                                  \
UNARY_VARS

#define WORD_CODING_VARS                                 \
WORD_CODING_VARS_DEF                                     \
INIT_WORD_CODING_VARS


#define INIT_WORD_CODING_VARS                            \
do {                                                     \
	UINT i;                                               \
	UWORD pattern= 0;                                      \
	Tmalloc(__buffer, UWORD, word_buffer_size);   \
	for (i=0; i<WORD_SIZE+1; i++) {                        \
		__mask[i]= pattern;                                  \
		__maskbit[i]= i && i<=WORD_SIZE ? ONE << (i-1) :  0; \
		pattern = (pattern<<1) | ONE;                        \
	}                                                      \
} while(0);
	

#define WORD_ENCODE_START(f)	                 					\
	WORD_CODING_VARS                                      \
	GLOBAL_WORD_ENCODE_START(f)

#define GLOBAL_WORD_ENCODE_START(f)	           					\
  INIT_WORD_CODING_VARS                                 \
  __wremaining = WORD_SIZE;	  	                  	\
  __pvalue = 0;								                        	\
  __wfile= (f);
  
#define GET_AVAILABLE_BITS __wremaining

/* auto-generated */
#define UNARY_VARS                                       \
static U1 __umask[256]= {						      							 \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,6,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,7,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,6,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,8,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,6,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,7,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,6,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,5,                       \
	1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,9};                      

/* Write one coded word */  
#define WORD_ENCODE_WRITE												\
{		    																				\
    register UWORD word; UWORD w;		      			\
		UPDATE_WORD                                 \
    word= __value[--__pvalue];									\
    for (--__pvalue; __pvalue >=0; __pvalue--)	\
    {																						\
      word <<= __bits[__pvalue];								\
      word |= __value[__pvalue];								\
    }																						\
    w= word;																		\
    if (fwrite((char*)&w,sizeof(w),1,__wfile) );\
		WBS_UPDATE(WORD_SIZE-__wremaining)          \
    __wremaining = WORD_SIZE;				  			\
    __pvalue = 0;																\
} 	
  
/* Encode int x>0 in _b bits */
#define WORD_ENCODE(x,_b)		            				\
{   							                          		\
  if (__wremaining<_b) WORD_ENCODE_WRITE;				\
  __value[__pvalue]= (x)-1;			          			\
  __bits[__pvalue++]= (_b);				           		\
  __wremaining -= (_b);				            			\
	BS_UPDATE(_b)                                 \
}

#define DWORD_ENCODE(x,_b)		            				\
{   							                          		\
  if (__wremaining<_b) WORD_ENCODE_WRITE;				\
  __value[__pvalue]= (x)-1;			          			\
  __bits[__pvalue++]= (_b);				           		\
  __wremaining -= (_b);				            			\
	BS_UPDATE(_b)                                 \
	fprintf(stderr," %d-%d ",x,_b);             \
}

#define WORD_ZERO_ENCODE(x,_b)					      	\
{	    						                          		\
  if (__wremaining<_b) WORD_ENCODE_WRITE				\
  __value[__pvalue]= (x);			            			\
  __bits[__pvalue++]= (_b);			          			\
  __wremaining -= (_b);					            		\
	BS_UPDATE(_b)                                 \
} 

#define DWORD_ZERO_ENCODE(x,_b)					      	\
{	    						                          		\
  if (__wremaining<_b) WORD_ENCODE_WRITE				\
  __value[__pvalue]= (x);			            			\
  __bits[__pvalue++]= (_b);			          			\
  __wremaining -= (_b);					            		\
	BS_UPDATE(_b)                                 \
	fprintf(stderr,"Z %d - %d\n",x,_b);             \
} 


#define WORD_CROSS_ENCODE(x,_b)			      			\
{	    																      		\
  register UWORD __x= (x)-1;				        		\
  register UWORD __b= (_b);					      	  	\
  if (__wremaining<__b) {						      			\
    if (__wremaining>0)	{							      		\
      __value[__pvalue]= __x >> (__b-__wremaining);			\
      __bits[__pvalue++]= __wremaining;	      	\
      __x <<= (WORD_SIZE-((__b-__wremaining)));	\
      __x >>= (WORD_SIZE-((__b-__wremaining)));	\
      __b -=  __wremaining;						      		\
    }														      					\
    WORD_ENCODE_WRITE 							      			\
  }																		      		\
	if (__b) {											      				\
  __value[__pvalue]= __x;							      		\
  __bits[__pvalue++]= __b;					      			\
  __wremaining -= __b;							      			\
	BS_UPDATE(_b)                                 \
	}																		      		\
} 

#define WORD_ZERO_CROSS_ENCODE(x,_b)		    		\
{	    																      		\
  register UWORD __x= (x);						        	\
  register UWORD __b= (_b);				        			\
  if (__wremaining<__b) {					      				\
    if (__wremaining>0)	{						      			\
      __value[__pvalue]= __x >> (__b-__wremaining);			\
      __bits[__pvalue++]= __wremaining;	      	\
      __x <<= (WORD_SIZE-((__b-__wremaining)));	\
      __x >>= (WORD_SIZE-((__b-__wremaining)));	\
      __b -=  __wremaining;					      			\
    }																	      		\
    WORD_ENCODE_WRITE 								      		\
  }																		      		\
	if (__b) {													      		\
  __value[__pvalue]= __x;							      		\
  __bits[__pvalue++]= __b;						      		\
  __wremaining -= __b;								      		\
	BS_UPDATE(_b)                                 \
	}																		      		\
} 


#define WORD_ENCODE_END							\
  if (__pvalue) WORD_ENCODE_WRITE	  \
	free(__buffer);

#define GLOBAL_WORD_ENCODE_END	WORD_ENCODE_END


/* ======================= MACROS FOR WORD DECODING ============ */

#ifdef FAST_DECODE
#define GLOBAL_WORD_DECODE_START(f)	\
  INIT_WORD_CODING_VARS             \
	__wfile= (f);						      		\
  __wpos=__buffend= __buffer;				\
	__wval= 0;                        \
	__wremaining= 0;
#else
#define GLOBAL_WORD_DECODE_START(f)	\
  INIT_WORD_CODING_VARS             \
	__wfile= (f);						      		\
  __buffend= __buffer;		      		\
  __wpos=__buffer+1;	     			  	\
	__wval= 0;                        \
	__wremaining= 0;

#endif


//  GET_NEW_WORD;								\
//  __wremaining = WORD_SIZE;   
 
#define WORD_DECODE_START(f)	\
	WORD_CODING_VARS            \
	GLOBAL_WORD_DECODE_START(f) 
 
#ifndef BLK_LEVEL_BUFFER_MANAGEMENT
#define GET_NEW_WORD															\
  if (__wpos<__buffend) __wval= *__wpos++;				\
  else																						\
  {																								\
    I4 tmp;			    															\
    if ( (tmp=fread(__buffer,sizeof(UWORD),word_buffer_size,__wfile))<1) { \
		  I4 *p=0; \
		  /* fprintf(stderr,"Attempted to read %d items of size %d from file %p to field %p\n", word_buffer_size, sizeof(UWORD), __wfile, __buffer); \
      fprintf (stderr, "Error when reading input file\n");		\
			*p=1;  */ \
			fprintf(stderr,"Cannot read file\n");       \
      exit(1);																		\
    }																							\
    __wpos= __buffer;															\
    __buffend= __buffer+tmp;											\
    __wval= *__wpos++;														\
  }
#else
#define GET_NEW_WORD __wval= *__wpos++;
#endif

#ifndef BLK_LEVEL_BUFFER_MANAGEMENT
#define WORD_BUFFER_ADJUST
#else
#define WORD_BUFFER_ADJUST {                            \
  if ( (__buffend - __wpos < MIN_DEC_BUF_LEN_WORD) && (!feof(__wfile))) {   \
	  I4 tmp;                                             \
	  I4 len= __buffend - __wpos;                         \
		if (len) memcpy(__buffer,__wpos, len*sizeof(*__buffer));  \
		__wpos= __buffer + len;                             \
		len= word_buffer_size - len;                        \
    if ( (tmp=fread(__wpos,sizeof(UWORD),len,__wfile))<1) { \
			fprintf(stderr,"Cannot read file\n");       \
      exit(1);																		\
    }																							\
    __buffend= __wpos+tmp;					  						\
    __wpos= __buffer;															\
  }                                               \
}
#endif



#define WORD_DECODE(x,_b)						\
{		    						               	\
  if (__wremaining < (_b)) {			  \
    GET_NEW_WORD					      		\
    __wremaining = WORD_SIZE;		  	\
  }								                	\
  (x) = (__wval & __mask[_b]) + 1;  \
  __wval >>= (_b);					    		\
  __wremaining -= (_b);							\
} 

#define FAKE_WORD_DECODE(x,_b)		  \
{		    						               	\
  if (__wremaining < (_b)) {			  \
    GET_NEW_WORD					      		\
    __wremaining = WORD_SIZE;		  	\
  }								                	\
  (x) = __wval;  \
  __wremaining -= (_b);							\
} 

#define SHORT_FAKE_WORD_DECODE(x,_b)		  \
{		    						               	\
  GET_NEW_WORD	  				      		\
	x &= __wval;                      \
  n -= (WORD_SIZE/(_b));            \
} 


/* this guy not responsible for updating __wremaining */
#define SIMPLE_WORD_DECODE(x,_b)	  \
  (x) = (__wval & __mask[_b]) + 1;  \
  __wval >>= (_b);	

#define DSIMPLE_WORD_DECODE(x,_b)	  \
{ I4 XX;                   \
  XX= (x) = (__wval & __mask[_b]) + 1;  \
  __wval >>= (_b);					    	  \
	fprintf(stderr," %d-%d ",XX,_b); \
	}

#define SIMPLE_WORD_ZERO_DECODE(x,_b)	  \
  (x) = (__wval & __mask[_b]);  \
  __wval >>= (_b);	

#define DSIMPLE_WORD_ZERO_DECODE(x,_b)	  \
{ I4 XX;                   \
  XX= (x) = (__wval & __mask[_b]);  \
  __wval >>= (_b);					    	\
	fprintf(stderr,"Z %d - %d\n",XX,_b); \
}



/* special case applied when _b can go to WORDSIZE;
   it's less efficient, & the reason is that x<<WORD_SIZE does not work */ 
#define S32_WORD_CROSS_DECODE(x,_b)							\
{                                           \
  if (__wremaining<_b) {										\
		x= __wremaining? __wval << (__wremaining= (_b) - __wremaining) : \
		                            (__wremaining= (_b),0);              \
		GET_NEW_WORD														\
		x = (x | (__wval & __mask[__wremaining])) + 1;  \
    __wval >>= __wremaining;								\
    __wremaining= WORD_SIZE - __wremaining;	\
  }	else {																	\
    x= (__wval & __mask[_b]) + 1;						\
    __wval >>= _b;													\
    __wremaining -= _b;											\
  }                                         \
}


#define WORD_SKIP_BITS(_b)				    			\
  if (__wremaining<_b) {										\
		__wremaining= (_b) - __wremaining;      \
		GET_NEW_WORD														\
    __wval >>= __wremaining;								\
    __wremaining= WORD_SIZE - __wremaining;	\
  }	else {																	\
    __wval >>= _b;													\
    __wremaining -= _b;											\
  }


#define WORD_CROSS_DECODE(x,_b)							\
  if (__wremaining<_b) {										\
		x= __wval << (__wremaining= (_b) - __wremaining);     \
		GET_NEW_WORD														\
		x = (x | (__wval & __mask[__wremaining])) + 1;  \
    __wval >>= __wremaining;								\
    __wremaining= WORD_SIZE - __wremaining;	\
  }	else {																	\
    x= (__wval & __mask[_b]) + 1;						\
    __wval >>= _b;													\
    __wremaining -= _b;											\
  }      


#define WORD_ZERO_CROSS_DECODE(x,_b)					\
  if (__wremaining<_b) {											\
		x= __wval << (__wremaining= (_b) - __wremaining);     \
		GET_NEW_WORD											  			\
		x = (x | (__wval & __mask[__wremaining]));  \
    __wval >>= __wremaining;							  	\
    __wremaining= WORD_SIZE - __wremaining;		\
  }	else {																	 	\
    x= __wval & __mask[_b];	  								\
    __wval >>= _b;														\
    __wremaining -= _b;				  							\
  }	                                          \

#define WORD_ZERO_DECODE(x,_b)			    			\
{		    					                        		\
  if (__wremaining < (_b)) {		            	\
    GET_NEW_WORD					                		\
    __wremaining = WORD_SIZE;				      		\
  }						                          			\
  (x) = __wval & __mask[_b];				      		\
  __wval >>= (_b);						              	\
  __wremaining -= (_b);						          	\
} 

#define WORD_DECODE_END	free(__buffer);					
#define GLOBAL_WORD_DECODE_END	WORD_DECODE_END					


#endif
