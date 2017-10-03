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


#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H


#ifdef _MAIN_PROG
#include "defs.h"

U4 org_blk=STD_BITS_FOR_ELEMS_PER_BLOCK;
I4 b;

UWORD __mask[WORD_SIZE+1];			
UWORD __maskbit[WORD_SIZE+1];  
#else

extern U4 org_blk;
extern I4 b;
extern UWORD __mask[];				
extern UWORD __maskbit[];    

#endif


#endif
