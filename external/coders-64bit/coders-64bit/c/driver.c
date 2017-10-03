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
 * drive.c -- driver for an integer compression package
 * 
 **************************************************************************/



#define _MAIN_PROG

#include "driver.h"
#include "uncompress_io.h"
#include "global_vars.h"




iface *new_iface() 
{
  iface *sys;
	Tmalloc(sys,iface,1);

  sys->coding_type = ENCODING;
	sys->sequence_type = DOCGAP;
	sys->print_stats= no;
	*sys->ifile = '\0';
	*sys->ofile = '\0';
	sys->text_file = no;
	sys->skip= 0;

	sys->epb= STD_ELEMS_PER_BLOCK;
	sys->epb_bits= STD_BITS_FOR_ELEMS_PER_BLOCK;
	sys->global_max= 0;
	sys->global_sum=0;
	sys->global_n=0;
	sys->word_buffer_size= WORD_BUFFER_SIZE;
	sys->byte_buffer_size= BYTE_BUFFER_SIZE;
	sys->total_elems=0;

	return sys;
}

iface *delete_iface(iface *sys) {
  if (sys) {
	  free(sys);
	}
	return NULL;
}



fpos_type FileSize(char* filename)
{
  struct stat stbuf;
  stat(filename, &stbuf);
  return stbuf.st_size;
}




void
ProcessArguments( iface *sys, int argc, CHAR *argv[]);


int main(int agrc, CHAR *argv[])
{

  U8 len;
  fpos_type tmp;
  UINT *a= NULL;
	iface *sys;

	sys= new_iface();
  ProcessArguments(sys, agrc, argv);
    
  
  if (!(a=(UINT *) malloc (sizeof(UINT)*(sys->epb+BLKSZE_ADJUST))))
  {
    fprintf(stderr,"Cannot allocate memory for the array of docnums\n");
    exit(1);
  }
  
  if (sys->coding_type==ENCODING)
  {
    if (( len = EncodeFile(a, sys) )< 0)
      return 1;
 
    if (sys->print_stats)
    {
      I4 i;
      UINT max=0;
      FLOAT bpd;
      fpos_type fsize= FileSize(sys->ofile);   /* sugar, fsize should be the same as len */
			len= fsize;
      bpd = sys->global_n? (len<<3)/(FLOAT)sys->global_n : 0;

			fprintf (stderr, "Number of symbols       = ");
			FPRNLN_GINT(stderr,sys->global_n)
      fprintf (stderr, "Average symbol          = %.2f\n",
                sys->global_n? sys->global_sum/(FLOAT)sys->global_n : 0);
			if (sys->global_n) {
        fprintf (stderr, "Maximum symbol	        = ");
				 FPRNLN_GINT(stderr,sys->global_max)
			}
      if (bpd>0 && bpd<100)
      {
			  fprintf (stderr, "Total output bytes      = ");
				FPRNLN_GINT(stderr,len)
        fprintf (stderr, "Average bits per symbol = %.2f\n",bpd);
      }
      else
	      fprintf (stderr, "Bits per symbol not available,\n"
	                 "  to have it, you need to use parameter -o\n");
    }
  }
  else if (!sys->skip)
  {
    if (!DecodeFile(a, sys->ifile, sys->ofile, sys->text_file, sys->sequence_type,sys->timing)) return 1;
  }    
	else {
    if (!SkipDecodeFile(sys->skip, a, sys->ifile, sys->ofile, sys->text_file, sys->sequence_type,sys->timing))
			return 1;
	}

  if (a) free(a);
	sys= delete_iface(sys);
  return 0;
}  


  
/*-------------------------
  Processing arguments to the main()
-------------------------*/

void
ProcessArguments(iface *sys, int argc, CHAR *argv[])
{
  I4 error = 0;
  int ch;

	CLOG2TAB_VAR;


 
  while ((ch = getopt (argc, argv, "Tvxdhti:o:s:b:B:O:")) != -1) {
    switch (ch)
    {
      case 'T':
        sys->timing= 1;   /* decoder will not output */
        break;
      case 'B':
        sys->byte_buffer_size= atoi(optarg);
				sys->byte_buffer_size *= 1024*1024;
				if (sys->byte_buffer_size < (MIN_DEC_BUF_LEN_BYTE<<3)) {
				  fprintf(stderr,"-B buffer_size: buffere size too small\n");
					error= 1;
				}
				sys->word_buffer_size= sys->byte_buffer_size / _BYTES_PER_WORD;
        break;
      case 'x':
        sys->coding_type = DECODING;
        break;
      case 'd':
        sys->sequence_type = DOCNUM;
        break;
      case 't':
        sys->text_file = yes;
        break;
      case 'v':
        sys->print_stats = yes;
        break;
      case 'O':
	      sys->opt= atoi(optarg);
	      break;
      case 's':
        sys->skip= atoi(optarg);
        break;
      case 'b':
			  #ifdef CBLOCK
				  org_blk= atoi(optarg);
				#else
          sys->epb= atoi(optarg);
				  QCEILLOG_2(sys->epb,sys->epb_bits);
				#endif
        break;
      case 'i':
        strncpy(sys->ifile,optarg,MAXFILENAME);
        break;
      case 'o':
        strncpy(sys->ofile,optarg,MAXFILENAME);
        break;
      case 'h':
      default :
			  fprintf(stderr,"Option -%c is not valid\n",ch);
        error = 1;
	      break;
    }
		}
	if (sys->skip) sys->coding_type = DECODING;
  if (error)
  {
    fprintf(stderr,
	"-------------------------------------------------------------\n"
	"   This program compress/decompress sequence of document\n"
	"numbers/gaps using an integer coding method\n"
	"-------------------------------------------------------------\n");
	
    fprintf( stderr, 
       "Usage:\n%s\t[-x] : extract file instead of compress \n"
			 "\t[-8] : uncompressed data in the form of 64-bit numbers (instead of 32)\n"
       "\t[-d] : uncompressed sequence is of docnums, instead of d-gaps \n"
       "\t[-t] : uncompressed sequence is of text form instead of binary \n"
       "\t\t[-i input_file_name] : default= stdin \n"
       "\t[-o output_file_name] : default= stdout\n"
       "\t[-v] : (verbose:) print statistics (not working with -x option)\n"
			 "\t[-T] : for decoder, will not output the decoded items\n"
			 "\t[-B] : decoding buffer size, in MB, default= 512"
       "\t[-h] : print this message \n", argv [0]);
    fprintf( stderr,
	"-------------------------------------------------------------\n"
        "Examples:	\n"
	"\tslide -i SMALL -o BIG        : compress file \"SMALL\", write to \"BIG\"\n"
	"\tslide < SMALL > BIG          : same as above\n"
	"\tslide -t < SMALL > BIG       : same as above,\n"
	"\t                               but \"SMALL\" is in text format\n"
	"\tslide -t -d < SMALL > BIG    : same as above,\n"
	"\t                               but \"SMALL\" contains sequence\n"
	"\t                               of document numbers, not gaps\n"
	"\tslide -x -t < BIG            : Decompress file \"BIG\",\n"
	"\t                               and write text output to stdout\n");
    fprintf( stderr,
	"-------------------------------------------------------------\n\n");
    exit(1);
  }
  
  return;
}
