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



/* this defines the format for printf and scanf, and also ATOL
   NOT YET fully tested                        
	 PURPOSE: to get rid of different format specification for 32/64 bit machine
	 USAGE: For any printf/scanf command: 
	         - need first to decompose the command into several 
					   so that each print *only one* value, for example,
						 printf("%d x= %f\n", a,b) will be decomposed into 4
						 (the last one is just print a new line character)
					 - then, for each elementary, replace it by an appropriate
					   command from the following list:
 PRN_CHAR(_x) printf(OUTFMT_CHAR,_x);
 PRN_STR(_x) printf(OUTFMT_STR,_x);
 PRN_LSTR(_x,_l) printf(OUTFMT_LSTR,_l,_x);
 PRN_RSTR(_x,_l) printf(OUTFMT_RSTR,_l,_x);
 PRN_CR printf("\n");

 PRN_FLOAT(_x,_l,_d) printf(OUTFMT_FLOAT,_l,_d,_x);
 PRN_DOUBLE(_x,_l,_d) printf(OUTFMT_FLOAT,_l,_d,_x);

 PRN_INT(_x,_l) printf(OUTFMT_INT,_l,_x);
 PRN_UINT(_x,_l) printf(OUTFMT_UINT,_l,_x);
 PRN_I4(_x,_l) printf(OUTFMT_I4,_l,_x);
 PRN_I8(_x,_l) printf(OUTFMT_I8,_l,_x);
 PRN_GINT(_x,_l) print generic int, unknown about actual size, very innefective  

----*/

#ifndef _BASIC_IO_H_
#define _BASIC_IO_H_

#include "basic_archs.h"

/* 1. Formats for in/out integers, OUTFMT= output format, INFMT= input format */ 

#ifdef _ARCH_32BITS
  #define OUTFMT_I4 "%*d" 
  #define OUTFMT_U4 "%*ud" 
  #define FMT_I4 "%d" 
  #define FMT_U4 "%ud" 
  #define INFMT_I4 " %d" 
  #define INFMT_U4 " %ud" 
  #ifdef _HAVE_64BIT_INT
    #define OUTFMT_I8 "%*lld"
    #define OUTFMT_U8 "%*ulld"
    #define INFMT_I8 " %lld"
    #define INFMT_U8 " %ulld"
    #define FMT_I8 "%lld"
    #define FMT_U8 "%ulld"

		#define ATOL atoll
  #else
    #define OUTFMT_I8 "%*d"
    #define OUTFMT_U8 "%*ud"
    #define INFMT_I8 " %d"
    #define INFMT_U8 " %ud"
    #define FMT_I8 "%d"
    #define FMT_U8 "%ud"
		#define ATOL atol
  #endif
#else
  #define OUTFMT_I4 "%*d" 
  #define OUTFMT_U4 "%*ud" 
  #define INFMT_I4 " %d" 
	#define INFMT_U4 " %ud" 
  #define FMT_I4 "%d" 
	#define FMT_U4 "%ud" 
	#define OUTFMT_I8 "%*ld" 
	#define OUTFMT_U8 "%*uld" 
	#define INFMT_I8 " %ld" 
  #define INFMT_U8 " %uld" 
	#define FMT_I8 "%ld" 
  #define FMT_U8 "%uld" 
	#define ATOL atol
#endif

#ifdef _USE_LARGE_INT
  #define OUTFMT_INT OUTFMT_I8
  #define OUTFMT_UINT OUTFMT_U8
  #define FMT_INT FMT_I8
  #define FMT_UINT FMT_U8
#else
  #define OUTFMT_INT OUTFMT_I4
  #define OUTFMT_UINT OUTFMT_U4
  #define FMT_INT FMT_I4
  #define FMT_UINT FMT_U4
#endif
  
/* 2. Format for float, double, char, string */
#define OUTFMT_FLOAT "%*.*f"
#define OUTFMT_DOUBLE "%*.*f"
#define INFMT_FLOAT " %f"
#define INFMT_DOUBLE " %f"
#define FMT_FLOAT "%f"
#define FMT_DOUBLE "%f"

#define OUTFMT_CHAR "%c"
#define INFMT_CHAR "%c"

#define OUTFMT_STR "%s"
#define INFMT_STR "%s"
#define OUTFMT_RSTR "%*s"    /* format string to the right of field */
#define OUTFMT_LSTR "%-*s"    /* format string to the left of field */

/* A1. printf to stdout, no format */
#define PRN_CHAR(_x) printf(OUTFMT_CHAR,_x);
#define PRN_STR(_x) printf(OUTFMT_STR,_x);
#define PRN_LSTR(_x,_l) printf(OUTFMT_LSTR,_l,_x);
#define PRN_RSTR(_x,_l) printf(OUTFMT_RSTR,_l,_x);
#define PRN_CR printf("\n");

#define PRN_FLOAT(_x) printf(FMT_FLOAT,_x);
#define PRN_DOUBLE(_x) printf(FMT_FLOAT,_x);

#define PRN_INT(_x) printf(FMT_INT,_x);
#define PRN_UINT(_x) printf(FMT_UINT,_x);
#define PRN_I4(_x) printf(FMT_I4,_x);
#define PRN_I8(_x) printf(FMT_I8,_x);
#define PRN_GINT(_x) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	PRN_I8(x);                                                       \
}
/*    printf to stdout, with format */ 
#define PRNF_FLOAT(_x,_l,_d) printf(OUTFMT_FLOAT,_l,_d,_x);
#define PRNF_DOUBLE(_x,_l,_d) printf(OUTFMT_FLOAT,_l,_d,_x);

#define PRNF_INT(_x,_l) printf(OUTFMT_INT,_l,_x);
#define PRNF_UINT(_x,_l) printf(OUTFMT_UINT,_l,_x);
#define PRNF_I4(_x,_l) printf(OUTFMT_I4,_l,_x);
#define PRNF_I8(_x,_l) printf(OUTFMT_I8,_l,_x);
#define PRNF_GINT(_x,_l) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	PRNF_I8(x,_l);                                                       \
}

/* A2 - as A1, with appending new line */
#define PRNLN_CHAR(_x) {printf(OUTFMT_CHAR,_x); printf("\n");}
#define PRNLN_STR(_x) {printf(OUTFMT_STR,_x); printf("\n");}
#define PRNLN_LSTR(_x,_l) {printf(OUTFMT_LSTR,_l,_x); printf("\n");}
#define PRNLN_RSTR(_x,_l) {printf(OUTFMT_RSTR,_l,_x); printf("\n");}
#define PRNLN printf("\n");

#define PRNLN_FLOAT(_x) {printf(FMT_FLOAT,_x); printf("\n");}
#define PRNLN_DOUBLE(_x) {printf(FMT_FLOAT,_x); printf("\n");}

#define PRNLN_INT(_x) {printf(FMT_INT,_x); printf("\n");}
#define PRNLN_UINT(_x) {printf(FMT_UINT,_x); printf("\n");}
#define PRNLN_I4(_x) {printf(FMT_I4,_x); printf("\n");}
#define PRNLN_I8(_x) {printf(FMT_I8,_x); printf("\n");}
#define PRNLN_GINT(_x) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	PRNLN_I8(x)                                         \
}

#define PRNFLN_FLOAT(_x,_l,_d) {printf(OUTFMT_FLOAT,_l,_d,_x); printf("\n");}
#define PRNFLN_DOUBLE(_x,_l,_d) {printf(OUTFMT_FLOAT,_l,_d,_x); printf("\n");}

#define PRNFLN_INT(_x,_l) {printf(OUTFMT_INT,_l,_x); printf("\n");}
#define PRNFLN_UINT(_x,_l) {printf(OUTFMT_UINT,_l,_x); printf("\n");}
#define PRNFLN_I4(_x,_l) {printf(OUTFMT_I4,_l,_x); printf("\n");}
#define PRNFLN_I8(_x,_l) {printf(OUTFMT_I8,_l,_x); printf("\n");}
#define PRNFLN_GINT(_x,_l) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	PRNFLN_I8(x,_l)                                         \
}

/* A3. as A1, but printf to file _f */
#define FPRN_CHAR(_f,_x) fprintf(_f,OUTFMT_CHAR,_x);
#define FPRN_STR(_f,_x) fprintf(_f,OUTFMT_STR,_x);
#define FPRN_LSTR(_f,_x,_l) fprintf(_f,OUTFMT_LSTR,_l,_x);
#define FPRN_RSTR(_f,_x,_l) fprintf(_f,OUTFMT_RSTR,_l,_x);
#define FPRN_CR(_f) fprintf(_f,"\n");

#define FPRN_FLOAT(_f,_x) fprintf(_f,FMT_FLOAT,_x);
#define FPRN_DOUBLE(_f,_x) fprintf(_f,FMT_FLOAT,_x);

#define FPRN_INT(_f,_x) fprintf(_f,FMT_INT,_x);
#define FPRN_UINT(_f,_x) fprintf(_f,FMT_UINT,_x);
#define FPRN_I4(_f,_x) fprintf(_f,FMT_I4,_x);
#define FPRN_I8(_f,_x) fprintf(_f,FMT_I8,_x);
#define FPRN_GINT(_f,_x) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	FPRN_I8(_f,x)                                                       \
}

#define FPRNF_FLOAT(_f,_x,_l,_d) fprintf(_f,OUTFMT_FLOAT,_l,_d,_x);
#define FPRNF_DOUBLE(_f,_x,_l,_d) fprintf(_f,OUTFMT_FLOAT,_l,_d,_x);

#define FPRNF_INT(_f,_x,_l) fprintf(_f,OUTFMT_INT,_l,_x);
#define FPRNF_UINT(_f,_x,_l) fprintf(_f,OUTFMT_UINT,_l,_x);
#define FPRNF_I4(_f,_x,_l) fprintf(_f,OUTFMT_I4,_l,_x);
#define FPRNF_I8(_f,_x,_l) fprintf(_f,OUTFMT_I8,_l,_x);
#define FPRNF_GINT(_f,_x,_l) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	FPRNF_I8(_f,x,_l)                                                       \
}

/* A4. as A2 - printf to file _f */
#define FPRNLN_CHAR(_f,_x) {fprintf(_f,OUTFMT_CHAR,_x); fprintf(_f,"\n");}
#define FPRNLN_STR(_f,_x) {fprintf(_f,OUTFMT_STR,_x); fprintf(_f,"\n");}
#define FPRNLN_LSTR(_f,_x,_l) {fprintf(_f,OUTFMT_LSTR,_l,_x); fprintf(_f,"\n");}
#define FPRNLN_RSTR(_f,_x,_l) {fprintf(_f,OUTFMT_RSTR,_l,_x); fprintf(_f,"\n");}
#define FPRNLN fprintf(_f,"\n");

#define FPRNLN_FLOAT(_f,_x) {fprintf(_f,FMT_FLOAT,_x); fprintf(_f,"\n");}
#define FPRNLN_DOUBLE(_f,_x) {fprintf(_f,FMT_FLOAT,_x); fprintf(_f,"\n");}

#define FPRNLN_INT(_f,_x) {fprintf(_f,FMT_INT,_x); fprintf(_f,"\n");}
#define FPRNLN_UINT(_f,_x) {fprintf(_f,FMT_UINT,_x); fprintf(_f,"\n");}
#define FPRNLN_I4(_f,_x) {fprintf(_f,FMT_I4,_x); fprintf(_f,"\n");}
#define FPRNLN_I8(_f,_x) {fprintf(_f,FMT_I8,_x); fprintf(_f,"\n");}
#define FPRNLN_GINT(_f,_x) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	FPRNLN_I8(_f,x)                                                       \
}

#define FPRNFLN_FLOAT(_f,_x,_l,_d) {fprintf(_f,OUTFMT_FLOAT,_l,_d,_x); fprintf(_f,"\n");}
#define FPRNFLN_DOUBLE(_f,_x,_l,_d) {fprintf(_f,OUTFMT_FLOAT,_l,_d,_x); fprintf(_f,"\n");}

#define FPRNFLN_INT(_f,_x,_l) {fprintf(_f,OUTFMT_INT,_l,_x); fprintf(_f,"\n");}
#define FPRNFLN_UINT(_f,_x,_l) {fprintf(_f,OUTFMT_UINT,_l,_x); fprintf(_f,"\n");}
#define FPRNFLN_I4(_f,_x,_l) {fprintf(_f,OUTFMT_I4,_l,_x); fprintf(_f,"\n");}
#define FPRNFLN_I8(_f,_x,_l) {fprintf(_f,OUTFMT_I8,_l,_x); fprintf(_f,"\n");}
#define FPRNFLN_GINT(_f,_x,_l) {  /* print generic int, whatever int type */ \
	I8 x= (_x);                                                         \
	FPRNFLN_I8(_f,x,_l)                                                       \
}


/* B1. scanf from stdin */
#define SCAN_CHAR(_x) scanf(INFMT_CHAR,&(_x));
#define SCAN_STR(_x) scanf(INFMT_STR,&(_x));
#define SCAN_LSTR(_x,_l) scanf(INFMT_LSTR,_l,&(_x));
#define SCAN_RSTR(_x,_l) scanf(INFMT_RSTR,_l,&(_x));

#define SCAN_FLOAT(_x,_l,_d) scanf(INFMT_FLOAT,_l,_d,&(_x));
#define SCAN_DOUBLE(_x,_l,_d) scanf(INFMT_FLOAT,_l,_d,&(_x));

#define SCAN_INT(_x,_l) scanf(INFMT_INT,_l,&(_x));
#define SCAN_UINT(_x,_l) scanf(INFMT_UINT,_l,&(_x));
#define SCAN_I4(_x,_l) scanf(INFMT_I4,_l,&(_x));
#define SCAN_I8(_x,_l) scanf(INFMT_I4,_l,&(_x));
#define SCAN_GINT(_x,_l) {  /* scanf generic int, whatever int type */ \
	I8 x;                                                                \
	SCAN_I8(x,_l);                                                       \
	(_x) = x;                                                            \
}


/* B2. scanf from file _f */
#define FSCAN_CHAR(_f,_x) scanf(_f,INFMT_CHAR,&(_x));
#define FSCAN_STR(_f,_x) scanf(_f,INFMT_STR,&(_x));
#define FSCAN_LSTR(_f,_x,_l) scanf(_f,INFMT_LSTR,_l,&(_x));
#define FSCAN_RSTR(_f,_x,_l) scanf(_f,INFMT_RSTR,_l,&(_x));

#define FSCAN_FLOAT(_f,_x,_l,_d) scanf(_f,INFMT_FLOAT,_l,_d,&(_x));
#define FSCAN_DOUBLE(_f,_x,_l,_d) scanf(_f,INFMT_FLOAT,_l,_d,&(_x));

#define FSCAN_INT(_f,_x,_l) scanf(_f,INFMT_INT,_l,&(_x));
#define FSCAN_UINT(_f,_x,_l) scanf(_f,INFMT_UINT,_l,&(_x));
#define FSCAN_I4(_f,_x,_l) scanf(_f,INFMT_I4,_l,&(_x));
#define FSCAN_I8(_f,_x,_l) scanf(_f,INFMT_I4,_l,&(_x));
#define FSCAN_GINT(_f,_x,_l) {  /* scanf generic int, whatever int type */ \
	I8 x;                                                                \
	FSCAN_I8(_f,x,_l);                                                   \
	(_x) = x;                                                            \
}


/* Basic file operation */
// very system dependent!
typedef off_t fpos_type;

#define FILE_SEEK(_file,_offset,_whence) \
  if (fseeko(_file,_offset,_whence)) {   \
	  fprintf(stderr,"FILE_SEEK error \n");\
		exit(1);                             \
	}                                     
#define FILE_GETPOS(_file,_offset) _offset= ftello(_file);
#define FILE_POSGAP(_len,_offset1,_offset2) _len= _offset2 - _offset1;
#define FILE_INCPOS(_offset,_len) _offset += _len;
#define FILE_DECPOS(_offset,_len) _offset -= _len; 
/* same as FILE_POSGAP, but takes care of negative distance */
#define FILE_DISTANCE(_len,_offset1,_offset2) _len= _offset1 <= _offset2? _offset2 - _offset1 : -(_offset1 - _offset2);


#define _FAST_FILE_READ_

#ifdef _FAST_FILE_READ_
  #include <sys/mman.h>
  #define FFILE_DEF(_fd,val) int _fd= val;
	#define FOPEN(_path,_fd) {                    \
	  if ( (_fd= open(_path,O_RDONLY)) < 0) {     \
		  fprintf(stderr,"Cannot open file %s\n",path); \
			exit(1);                                  \
		}                                           \
	}
	#define FMAP(_fd,_fdpos,_buflen,_buffer) {           \
	  if ( (_buffer= mmap((void*) 0, _buflen, PROT_READ, \
		      MAP_PRIVATE, _fd, _fdpos)) == MAP_FAILED ) { \
			fprintf(stderr,"Cannot read file by FMAP\n");    \
			exit(1);                                         \
		}                                                  \
  }       

	/* same as FMAP, but try to reuse buffer (note: of the same length) */
	#define FREMAP(_fd,_fdpos,_buflen,_buffer) {                 \
	  if ( (_buffer= mmap((void*) _buffer, _buflen, PROT_READ,   \
		      MAP_PRIVATE, _fd, _fdpos)) == MAP_FAILED ) {         \
			fprintf(stderr,"Cannot read file by FMAP\n");            \
			exit(1);                                                 \
		}                                                          \
  }       
  #define FUNMAP(_buffer,_buflen) munmap( (void *) (_buffer),_buflen);
  #define FUNREMAP(_buffer,_buflen) munmap( (void *) (_buffer),_buflen);

#else
	#define FFILE_DEF(_fd,val) FILE *_fd= val;
	#define FOPEN(_path,_fd) {                    \
	  if ( !(_fd= fopen(_path,"r")) ) {           \
		  fprintf(stderr,"Cannot open file %s\n",path); \
			exit(1);                                  \
		}                                           \
	}

  #define FMAP(_fd,_fdpos,_buflen,_buffer) {                   \
	  if (! (_buffer= malloc(_buflen))) {                        \
		  fprintf(stderr,"Cannot allocate mem for FILE FMAP\n");   \
			exit(1);                                                 \
		}                                                          \
		if ( fread(_buffer, _buflen, 1, _fd) < _buflen ) ) {       \
		  fprintf(stderr,"Cannot read %d bytes in FLIE FMAP\n", _buflen); \
			exit(1);                                                 \
		}                                                          \
	}
  #define FREMAP(_fd,_fdpos,_buflen,_buffer) {                   \
		if ( fread(_buffer, _buflen, 1, _fd) < _buflen ) ) {       \
		  fprintf(stderr,"Cannot read %d bytes in FLIE FMAP\n", _buflen); \
			exit(1);                                                 \
		}                                                          \
	}
	#define FUNMAP(_buffer,_buflen) free(_buffer);
	#define FUNREMAP(_buffer,_buflen) 
#endif


#endif /* _BASIC_IO_H_ */
