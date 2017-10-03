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
 * uncompress_io.c -- function for io uncompressed files
 *                    part of "slide"
 *
 
 *
 * 
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "uncompress_io.h"

#include "global_vars.h"

FILE *
OpenFile(CHAR *filename, CHAR *mode)
{
  FILE *f= NULL;
  if (*filename)
    {
    if (!(f = fopen(filename, mode)))
      fprintf(stderr, "Cannot open file %s\n", filename);
    }
  else 
    f= *mode=='r'? stdin : stdout ;

  return f;
}  
    



/*-------------------------
   Read at most "*n" intergers from "filename" to "a"
   flag = 0 : input numbers intepreted as document gaps
   flag = 1 : input numbers intepreted as document number
   text_file = 1 if reading from a text file
   return 1 if Ok, 0 otherwise; 
          *n = number of items read;
-------------------------*/
   
int
ReadDocGaps(UINT *a, I4 *n, FILE *f, 
    I4 text_file, I4 flag, UINT *global_curr, I4 epb,
		UINT *gmax, U8 *gsum)
{
  I4 i;
  UINT curr=*global_curr;
#ifdef _LARGE_SOURCE_DATA
	UINT tmp;
#else
  U4 tmp;
#endif
  I4 bytes;
	UINT max=0;
	U8 sum=0;

	// fprintf(stderr, "Attempting to read %d elems\n",epb);

  for (i=0; !feof(f) && i<epb; i++)
  {
    if (text_file)
      if ( fscanf(f, " %d ", &tmp) != 1) {
         fprintf(stderr, "Errors when reading file\n");
         exit(1);
      }
    if (!text_file) 
      if ( (bytes=fread(  &tmp, sizeof(tmp), 1, f)) != 1) {
			   if (feof(f)) break;
         fprintf(stderr, "Errors when reading file \n");
         exit(1);
      }
    if (flag==DOCNUM) {
      if (tmp <= curr)
      {
	fprintf(stderr, "Error: sequence not in increasing order"
	                " at item number %d\n",i+1);
	fprintf(stderr, "Suggestion: when using -d option for compression "
	    "be sure that the input file is a sequence of positive"
	    " numbers in strictly increasing order\n");
	    
	exit(1);
      }
      a[i] = tmp - curr;
      curr = tmp;
    } else {
      if (tmp <= 0)
      {
	fprintf(stderr, "[ReadDocGaps] Error: invalid d-gap"
	                " at item number %d\n",i+1);
	exit(1);
      }
      a[i]= tmp;
    }
		if (max<a[i]) max= a[i];
		sum += a[i];
  }
  *n = i;
	// fprintf(stderr, " ... read %d elems\n",i);
  *global_curr= curr;
	if (*gmax<max) *gmax= max;
	*gsum += sum;
  return i;
}



/*-------------------------
 Write "n" item from "a" to file "filename"
-------------------------*/   

int
WriteDocGaps(FILE *f, UINT *a, I4 n, CHAR *filename, I4 text_file, I4 flag, UINT *global_curr)
{
  I4 i;
 
  if (flag==DOCNUM)
  { 
    a[0] = *global_curr + a[0];
    for (i=1; i<n; i++) a[i] += a[i-1];
    *global_curr= a[n-1];
  }
      
  if (text_file)
    for (i=0; i<n; i++)
    {
      if ( fprintf(f,"%u\n", a[i]) <1 )
      {
        fprintf(stderr, "Errors when writing file %s\n", filename);
        return 0;
      }
    }
  else {
#ifdef _LARGE_SOURCE_DATA
	  UINT tmp;
#else
    U4 tmp;
#endif

		for (i=0; i<n; i++) {
		  tmp= a[i];
			if ( fwrite( (CHAR*) &tmp, sizeof(tmp), 1, f) != 1) {
			  fprintf(stderr, "Errors when writing file %s\n", filename);
				return 0;
			}
		}
	} 
  
  return 1;
}
  


void
CumulateArray(UINT *a, I4 n) {
	I4 i;
	// printf("sizeof UINT= %d\n",sizeof(UINT));
	// for (i=0; i<n; i++) printf("%ld ",a[i]); printf("\n\n");
	for (i=1; i<n; i++) {
		a[i] += a[i-1];
			// if (i<2) fprintf(stderr,"i=%d a[i-1]= %ld a[i]= %ld\n",i,a[i-1],a[i]);
	}
	//for (i=0; i<n; i++) printf("%ld ",a[i]); printf("\n\n");
}

void
DecumulateArray(UINT *a, I4 n) {
	I4 i;
	for (i=n-1; i>0; i--) 
		a[i] -= a[i-1];
}

      
