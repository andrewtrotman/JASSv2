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
 * codes.c :- file encoding/decoding 
 * 
 **************************************************************************/


#include <stdio.h>
#include "driver.h"
#include "defs.h"
#include "uncompress_io.h"
#include "block.h"
#include <sys/stat.h>
#include <unistd.h>


#include "codes.h"


#include "global_vars.h"



UINT CalcMax(UINT *a, I4 n) 
{
	I4 i;
	UINT max=0;
	for (i=0; i<n; i++) 
		if (max < a[i]) max= a[i];
	return max;
}

U8 CalcSum(UINT *a, I4 n) 
{
	I4 i;
	U8 sum= 0;
	for (i=0; i<n; i++) 
		sum += a[i];
	return sum;
}



I4 icmp(const void *a, const void *b)
{
  const U4 *aa= (U4 *) a;
  const U4 *bb= (U4 *) b;
  return *aa < *bb ? -1: (*aa==*bb? 0: 1);
}


U8
EncodeFile(UINT *a, iface *sys)
{
  FILE *f=NULL,*inf=NULL;      /* output, input file */
  I4 i=0,j;
  fpos_type len= 0;   /* total codelength in machine words */
  DOUBLE Pr=0;
  I4 n=0;
	U8 flen;
	UINT curr;
	#ifdef INTERP
	  I4 logn;
	#endif
	SPECIFIC_ENCODING_VARS

  /* opening input file if needed */
  {  
	  fpos_type endf;
	  if (*sys->ifile) {
       if (!(inf = OpenFile(sys->ifile,"r"))) exit(1);
		} else inf= stdin;
		#ifndef _NO_TOTAL_ELEMS_
      len= ftello(inf); 
	    fseeko (inf, 0, SEEK_END);
	    endf = ftello(inf);
	    fseeko (inf, len, SEEK_SET);
		  sys->global_n= endf>>2;
		  sys->total_elems= sys->global_n;
		#endif
  } 
  /* open output file */
	if (*sys->ofile)
     f = OpenFile(sys->ofile,"w") ;
	else f= stdout;
	if (fwrite(&sys->total_elems,sizeof(sys->total_elems),1, f) <1) {
	  fprintf(stderr, "Cannot write to file. Disk full?\n");
		exit(1);
	}

  len= 0;

	 
  INITSTATS
	SCHEME_ENCODE_START(f)
	
  while (1)
  {
	  BLOCK_ENCODE_START(n)
    #ifdef INTERP
      ReadDocGaps(a+1,&n,inf,sys->text_file,sys->sequence_type,&curr,sys->epb,&(sys->global_max), &(sys->global_sum));
    #else
      #ifndef VARIABLE_LENGTH_BLOCK
        ReadDocGaps(a,&n,inf,sys->text_file,sys->sequence_type,&curr,sys->epb,&(sys->global_max), &(sys->global_sum));
			#else  /* VARIABLE_LENGTH_BLOCK  defined */

			  /* here, we have n-i<BLKSZE_ADJUST or n=i=0 */
				if (n-i>0) {    /* move the last elems to the front */
				  for (j=0; j<n-i; j++) a[j]= a[i+j];
				} 
				n= n-i;
			  if (sys->total_elems>0) {
					I4 tmp;
				  ReadDocGaps(a+n,&tmp,inf,sys->text_file,sys->sequence_type,&curr,sys->epb,&(sys->global_max), &(sys->global_sum));
					n+= tmp;
					sys->total_elems -= tmp;
				}
				if (n<=0) break;
      #endif
    #endif		
		

		ENCODE_PARAMETERS

		E_BLK_ENCODE

    BLOCK_ENCODE_END(n)
  }
  
  SCHEME_ENCODE_END(f)
	ENDSTATS(stderr)

  fflush(f);
	FILE_GETPOS(f,len)
	
  if (*sys->ofile) fclose(f);
  if (*sys->ifile) fclose(inf);
	flen= len;
  return flen;
}

	
I4
DecodeFile(UINT *a,  iface *sys)
{
  FILE *f;
  FILE *outf=NULL;
  I4 n;
  UINT curr= 0;
	#ifdef INTERP
	 I4 logn;
	#endif
	I4 bundle=1;
  
  SPECIFIC_DECODING_VARS
	 
  /* open input file */
	if (*sys->ifile) 
    f = OpenFile(sys->ifile,"r") ;
	else f=stdin;
	#ifndef _NO_TOTAL_ELEMS_
	  if (fread(&sys->total_elems,sizeof(sys->total_elems),1, f) <1) {
		  fprintf(stderr,"Cannot read file\n");
			exit(1);
		}
		// fprintf(stderr,"XXX Decoding %d elems\n",sys->total_elems);
  #endif

	if (*sys->ofile) {
    if (!(outf = OpenFile(sys->ofile, "w"))) return 0;
	} else outf= stdout;
  
  SCHEME_DECODE_START(f)	

  while(1)
  {
    register I4 i;
		// fprintf(stderr,"Decoding bundle %d\n",bundle++);
    #ifdef BYTE
		  BYTE_BUFFER_ADJUST
		#else
		  WORD_BUFFER_ADJUST
		#endif

    BLOCK_DECODE_START(n)
		
		GET_PARAMETERS
   
	  E_BLK_DECODE
    
		
    if (!sys->timing) {
		  #ifdef VARIABLE_LENGTH_BLOCK
		    if (n>sys->total_elems) n= sys->total_elems;
			#endif
    #ifdef INTERP
      if (!WriteDocGaps(outf, a+1, n, sys->ofile, sys->text_file, sys->sequence_type,&curr)) return 0;
    #else
      if (!WriteDocGaps(outf, a, n, sys->ofile, sys->text_file, sys->sequence_type,&curr)) return 0;
    #endif
		}
		BLOCK_DECODE_END(n)
  }


	SCHEME_DECODE_END
  if (*sys->ifile) fclose(f);
  if (*sys->ofile) fclose(outf);

  
	
	#ifdef NO_DECODING
	  fprintf(stderr,"# check sum %d\n",sys->global_sum &255);
	#endif


  return 1;
}



#ifdef INTERP
I4
SkipDecodeFile(I4 step, UINT *a,  iface *sys) 
{
	return 0;
}

#else

// skip each step-th elem
// [note might be inconsistent with skip_slide etc]

I4
SkipDecodeFile(I4 step, UINT *a,  iface *sys)
{
  FILE *f;
  FILE *outf=NULL;
  I4 n;
  UINT curr= 0;
	#ifdef INTERP
	  I4 logn;
	  static stack *s    = NULL;
	  static UINT  ss    = 0;
	#endif
	I4 skip= step;
  register I4 count,i,j,k,x;
  fpos_type skips=0, decodes=0, alls=0;
 
	SPECIFIC_DECODING_VARS
	
	
  /* open input file */
  f = OpenFile(sys->ifile,"r") ;
  if (!(outf = OpenFile(sys->ofile, "w"))) return 0;
 
  SCHEME_DECODE_START(f)

  count = 1; 
	i=0;
	
	while(1) {
	  BLOCK_DECODE_START(n)
		j= n;
		GET_PARAMETERS
	  while (j>skip) {
		  SCHEME_SKIP(skip);
			skips += skip;
			E_DECODE
			i++;
			if (i==sys->epb) {
			  WriteDocGaps(outf, a, i, sys->ofile, sys->text_file, sys->sequence_type,&curr);
				decodes += i;
				i=0;
			}
			j= j - skip - 1;
			skip= step;
		}
    if (n==sys->epb) {
		  if (j) {
			  SCHEME_SKIP(j);
				skips += j;
				skip -= j;
			}
		} else break;
	}
	if (i) {
	  WriteDocGaps(outf, a, i, sys->ofile, sys->text_file, sys->sequence_type,&curr);
		decodes += i;
	}

	SCHEME_DECODE_END

  if (*sys->ifile) fclose(f);
  if (*sys->ofile) fclose(outf);
  // 	fprintf(stderr,"SKIPPED %lld DECODED %lld  ALLS=%lld\n",skips,decodes,alls);
  return 1;
}

#endif
