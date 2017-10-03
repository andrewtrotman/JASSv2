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
  byte.h: macro for byte coding
	Notes: Buffer management could be a big suffer in decoding speed
	       for byte. The FAST_DECODE flag easies this problem by ensuring
				 that the buffer is large enough, and there will be no need
				 to check for end-of-buffer during elementary decode.

*/

#ifndef _BYTES_H_
#define _BYTES_H_

#ifdef BYTE
#include "defs.h"
#include "qlog2.h"

#define COMP_METHOD "byte"



#define BYTE_CODING_VARS           \
FILE *__wfile;                     \
U4 maskbit8= 128, mask127=127;     \
U1 *__buffer;   \
U1 *__buffend;                     \
U1 *__wpos;                        \
U1 __wval; 



#define BYTE_ENCODE_VARS                    \
BYTE_CODING_VARS                            \
U1 _bytes[9]; I4 _bi;                       \


#define BYTE_ENCODE(x)							\
{				     				              	\
  UWORD _x= (x)-1;     							\
  _bytes[0]= (_x & mask127) | maskbit8;          \
  BYTE_UPDATE_STATS(_x)             \
  _x >>= 7;                         \
  for (_bi=1; _x>0; _bi++)          \
  {                                 \
    _bytes[_bi] =  _x & mask127;    \
    BYTE_UPDATE_STATS(_x)           \
    _x >>= 7;                       \
  }                                 \
  for (_bi--; _bi>=0; _bi--) {      \
  if (!fwrite(&_bytes[_bi], 1, 1, __wfile)) { \
    fprintf(stderr,"Cannot write file - out of space?\n");  \
    exit(1);                        \
  }                                 \
  }                                 \
} 
  

#define BYTE_ENCODE_START(f)        \
{                                   \
  BYTE_ENCODE_VARS                  \
  __wfile= (f);

#define BYTE_ENCODE_END  }



// Decoding ==========================================

#define BYTE_DECODE_START(f)  \
{                             \
  BYTE_CODING_VARS            \
	Tmalloc(__buffer,*__buffer,sys->byte_buffer_size+1); \
  __wfile= (f);               \
  __wpos=__buffend= __buffer;      

#ifndef FAST_DECODE
#define GET_NEW_BYTE													        		\
  if (__wpos<__buffend) __wval= *__wpos++;		        		\
  else																				        		\
  {																						        		\
    I4 tmp;								  									        		\
    if ( (tmp=fread(__buffer,sizeof(U1),sys->byte_buffer_size,__wfile))<1) { \
		  fprintf (stderr, "sys->byte_buffer_size= %d\n",sys->byte_buffer_size); \
      fprintf (stderr, "Error when reading input file\n");    \
      exit(1);                                                \
    }                                                     \
    __wpos= __buffer;                             \
    __buffend= __buffer+tmp;                      \
    __wval= *__wpos++;                            \
  }
#define BYTE_BUFFER_ADJUST

#else
#define GET_NEW_BYTE __wval= *__wpos++;
#define BYTE_BUFFER_ADJUST {                            \
  if ( (__buffend - __wpos < MIN_DEC_BUF_LEN_BYTE) && (!feof(__wfile))) {   \
	  I4 tmp;                                             \
	  I4 len= __buffend - __wpos;                         \
		if (len) memcpy(__buffer,__wpos, len*sizeof(*__buffer));  \
		__wpos= __buffer + len;                             \
		len= sys->byte_buffer_size - len;                        \
    if ( (tmp=fread(__wpos,sizeof(U1),len,__wfile))<1) { \
			fprintf(stderr,"Cannot read file\n");       \
      exit(1);																		\
    }																							\
    __buffend= __wpos+tmp;								  			\
    __wpos= __buffer;															\
  }                                               \
}
#endif


#ifdef FAST_DECODE

#define BYTE_DECODE(x)                            \
{                                                 \
  (x)= *__wpos & mask127;                         \
	if (!( *__wpos++ & maskbit8) ) {                \
	  while(!( *__wpos & maskbit8) )                \
		  (x) = ((x) << 7) | *__wpos++ ;              \
		(x) = (((x) << 7) | (*__wpos++ & mask127));   \
	}                                               \
	(x)++;                                          \
} 

#else

#define BYTE_DECODE(x)                           \
{                                                 \
  (x) = 0;                                        \
  GET_NEW_BYTE;                                   \
  while( !((__wval) & maskbit8) ) {               \
    (x) = ((x) << 7) + ((__wval) & mask127);      \
    GET_NEW_BYTE;                                 \
  }                                               \
  (x) = ((x) << 7) + ((__wval) & mask127) + 1;    \
} 


#endif

#define BYTE_DECODE_END free(__buffer); }

#define BYTE_SKIP(_n)




/* ------------------ Interface -------------------- */

#define SCHEME_ENCODE_START(f) BYTE_ENCODE_START(f)
#define E_ENCODE BYTE_ENCODE(a[i])
#define SCHEME_ENCODE_END(f) BYTE_ENCODE_END

#define SPECIFIC_DECODING_VARS 
#define SCHEME_DECODE_START(f) BYTE_DECODE_START(f)

#define E_DECODE BYTE_DECODE(a[i])

#define SCHEME_SKIP(_n) BYTE_SKIP(_n)

#define SCHEME_DECODE_END BYTE_DECODE_END




#endif   /* def BYTE */
	
#endif    
    



