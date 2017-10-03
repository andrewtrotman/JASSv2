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
 *
 * cpoied from carry_coder.c -- of skip_carry
 * 
*/

#define _WORD_ALIGN_C_
#include <stdio.h>
#include <stdlib.h>

#include "qlog2.h"

static warning=0;

/* bits[i]= bits needed to code gaps[i]
   return max(bits[i])
*/
I4
CalcMinBits(INT *gaps, UCHAR *bits, I4 n)
{
  register I4 i;
  register I4 max=0;
  U8 sum= 0;
  CLOG2TAB_VAR;
  
  for (i=0; i<n; i++) { 
    QCEILLOG_2(gaps[i], bits[i]);
    sum +=  bits[i];
    if (max<bits[i]) max= bits[i];
  }

  if (max>28 && !warning) {
    fprintf(stderr, "Warning: At least one gap exceeds 2^28.\n");
		warning= 1;
    #ifdef SIMPLE
       #if WORD_SIZE < 64
         exit(1);
       #endif
    #endif
  }
  if (max>28 && warning<2) {
    fprintf(stderr, "Warning: This vesrsion of carry requires all gaps <=2^28.\n");
		warning= 2;
    #ifdef CARRY
       #if WORD_SIZE < 64
         exit(1);
       #endif
    #endif
  }
  return max;
}





#ifdef NCARRY
#define CARRY_ELEMSCODED

I4
elems_coded(I4 avail, I4 len, UCHAR *bits,
                I4 start, I4 end)
{
  register I4 i, real_end, max;
  if (len) {
    max= avail/len;
    real_end= start + max - 1 <= end ? start + max: end+1; 
    for (i=start; i<real_end && bits[i]<=len; i++);
    if (i<real_end) return 0;
    return real_end-start;
  } else {
    for (i=start; i<start+MAX_ELEM_PER_WORD && i<=end && bits[i]<=len; i++);
    if (i-start<2) return 0;
    return i-start;
  }  
}
  
#endif





