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


/* ------------------------------------------------------------------------
 *
 * uncompress_io.h -- io uncompressed files
 * used with uncompress_io.c
 * 
   ------------------------------------------------------------------------*/

#ifndef _RBUC_INTERFACE
#define _RBUC_INTERFACE

#include "basic_types.h"

#define LIMIT 10000000   /* max number of elems created by CreateDocGaps */
#define DOCGAP 0
#define DOCNUM 1
#define ENCODING 0
#define DECODING 1
#define MAXFILENAME 255

FILE *
OpenFile(CHAR *filename, CHAR *mode);

I4
ReadDocGaps(UINT *a, I4 *n, FILE *f,
  I4 text_file, I4 flag, UINT *global_curr, I4 epb,
	UINT *gmax, U8 *gsum);

int
WriteDocGaps(FILE *f, UINT *a, I4 n, CHAR *filename,
  I4 text_file, I4 flag, UINT *global_curr);

int
CreateDocGaps(UINT *a, I4 *nn, DOUBLE Pr, I4 epb);
  
void
CumulateArray(UINT *a, I4 n);

void
DecumulateArray(UINT *a, I4 n);


#endif  
