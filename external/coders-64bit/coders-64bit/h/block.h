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


/* 64: might need to chnage sys->epb, sys->epb_bits */

#ifndef _BLOCK_H_
#define _BLOCK_H_

#ifdef ANDREW
#define BIT_LEVEL     /* for Adrew's implemntation of interp */
#else
#define WORD_LEVEL
#endif




#ifndef VARIABLE_LENGTH_BLOCK
#define BLOCK_ENCODE_START(n)                \
{                                            \
  if (sys->total_elems>sys->epb) {            \
	  n= sys->epb;                      \
		sys->total_elems -= sys->epb;             \
	} else {                                   \
	  n= sys->total_elems;                          \
		sys->total_elems= 0;                          \
	}                                          \
	if (n==0) break;                           \
}


#define BLOCK_DECODE_START(n)                \
BLOCK_ENCODE_START(n)

#define BLOCK_ENCODE_END(n) if (n<sys->epb) break;
#define BLOCK_DECODE_END(n) if (n<sys->epb) break;


#else

#define BLOCK_ENCODE_START(n)   
  
#define BLOCK_DECODE_START(n)   

#define BLOCK_ENCODE_END(n) 
#define BLOCK_DECODE_END(n) if ( (sys->total_elems -= n)<=0) break;  

#endif

#endif
