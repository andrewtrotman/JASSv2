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


/* 64: NOT YET worked for x>2^32         FIXME */

#ifndef _QLOG2_H_
#define _QLOG2_H_

#include "basic_types.h"

#ifdef _USE_LARGE_INT
#define CEILLOG_2(x,v)                     \
{                                          \
  register U8 _B_x  = (x) - 1;             \
  (v) = 0;                                 \
  for (; _B_x ; _B_x>>=1, (v)++);          \
} 
#else
#define CEILLOG_2(x,v)                     \
{                                          \
  register U4 _B_x  = (x) - 1;             \
  (v) = 0;                                 \
  for (; _B_x ; _B_x>>=1, (v)++);          \
} 
#endif

#ifdef _USE_LARGE_INT
#define QCEILLOG_2(x,v)					           \
{                                          \
  register U8 _B_x  = (x) - 1;             \
	(v) = _B_x>>32 ?                         \
	     (                                   \
			   (_B_x>>=32) >> 16?                \
         (_B_x>>24 ? 56 + CLOG2TAB[_B_x>>24] : 48 | CLOG2TAB[_B_x>>16]) \
	       :					                       \
	       (_B_x>>8 ? 40 + CLOG2TAB[_B_x>>8] : 32 | CLOG2TAB[_B_x]) 		\
			 )                                   \
			 :                                   \
			 (                                   \
         _B_x>>16 ?						             \
         (_B_x>>24 ? 24 + CLOG2TAB[_B_x>>24] : 16 | CLOG2TAB[_B_x>>16]) \
	       :					                       \
	       (_B_x>>8 ? 8 + CLOG2TAB[_B_x>>8] : CLOG2TAB[_B_x]) 		\
	     );                                  \
}
      
#define QZEROCEILLOG_2(x,v)						     \
{                                          \
  register U8 _B_x  = (x);                 \
	(v) = _B_x>>32 ?                         \
	     (                                   \
			   (_B_x>>=32) >> 16?                \
         (_B_x>>24 ? 56 + CLOG2TAB[_B_x>>24] : 48 | CLOG2TAB[_B_x>>16]) \
	       :					                       \
	       (_B_x>>8 ? 40 + CLOG2TAB[_B_x>>8] : 32 | CLOG2TAB[_B_x]) 		\
			 )                                   \
			 :                                   \
			 (                                   \
         _B_x>>16 ?						             \
         (_B_x>>24 ? 24 + CLOG2TAB[_B_x>>24] : 16 | CLOG2TAB[_B_x>>16]) \
	       :					                       \
	       (_B_x>>8 ? 8 + CLOG2TAB[_B_x>>8] : CLOG2TAB[_B_x]) 		\
	     ) ;                                  \
} 
#else
#define QCEILLOG_2(x,v)					           \
{                                          \
  register U4 _B_x  = (x) - 1;          \
  (v) = _B_x>>16 ?						             \
         (_B_x>>24 ? 24 + CLOG2TAB[_B_x>>24] : 16 | CLOG2TAB[_B_x>>16]) \
	 :								                       \
	 (_B_x>>8 ? 8 + CLOG2TAB[_B_x>>8] : CLOG2TAB[_B_x]) ;		\
}
      
#define QZEROCEILLOG_2(x,v)						     \
{                                          \
  register U4 _B_x  = x;                \
  (v) = _B_x>>16 ?					               \
         (_B_x>>24 ? 24 + CLOG2TAB[_B_x>>24] : 16 | CLOG2TAB[_B_x>>16]) \
	 :							                         \
	 (_B_x>>8 ? 8 + CLOG2TAB[_B_x>>8] : CLOG2TAB[_B_x]) ;		\
} 
#endif

/* auto-generated */
#define CLOG2TAB_VAR							\
unsigned char CLOG2TAB[]={						\
0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };


#ifdef _USE_LARGE_INT
#define FLOORLOG_2(x,v)					    		\
{						                      			\
  register U8 _B_x  = (x);				  		\
  (v) = -1;	              							\
  for (; _B_x ; _B_x>>=1, (v)++);				\
} while(0);
#else
#define FLOORLOG_2(x,v)					    		\
{						                      			\
  register U4 _B_x  = (x);						\
  (v) = -1;	              							\
  for (; _B_x ; _B_x>>=1, (v)++);				\
} while(0);
#endif


#endif
