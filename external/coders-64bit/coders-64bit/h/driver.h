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


#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       /* getopt */
#include <sys/stat.h>

#include "basic_types.h"
#include "defs.h"
#include "qlog2.h"


typedef struct iface {
  CHAR ifile[FILENAME_MAX+1];    /* input pathname */
  CHAR ofile[FILENAME_MAX+1];    /* output pathname */
  I4 coding_type;                /* DECODING/ENCODING */
  I4 sequence_type;              /* DOCNUM (sorted) or gaps (unsorted) */
  I4 text_file;                  /* yes= output text file (decode only) */
  I4 print_stats;                /* yes= print encoding stats */ 
  I4 skip;
	I4 timing;                     /* yes, if for timing decoding */
	I4 opt;                        /* option, for msim only */
	I4 b;                          /* code parameter, for bblock-alike */ 

        /* ---- various constants ------------- */
	I4 epb;                        /* elems per block */
	I4 epb_bits;                   /* bits to store epb */
	UINT global_max;
	U8 global_sum;
	U8 global_n;
	I4 word_buffer_size;
	I4 byte_buffer_size;
	I4 total_elems;
} iface;






#endif
