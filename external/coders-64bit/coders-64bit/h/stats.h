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


/* stats.h - for statistics of word-aligned compression */

#ifndef _STATS_H_
#define _STATS_H_
#include <string.h>
#include "basic_types.h"
#include "basic_io.h"



#define S_MAXS 10
#define S_MAXBIT 32
#define S_MAX_MAXBIT 32


/* index to blockstats.val */
#define SYM 0          /* number of symbols */
#define MAX_BITS 1     /* max bit of the block */
#define WORDS 2        /* number of output words */
#define SELS 3         /* number of selectors/groups */ 
#define SELBITS 4      /* bits for selectors */
#define UBITS 5        /* bits that not used in neither data field nor selector*/
#define INEFBITS 6     /* leading zero bits in any data items */
#define REALBITS 7     /* meaningful bits in data */


#define SHEADER "bsize    Syms maxbit  words   sels selbit  ubits i-bits r-bits a-bits\n"

#ifdef _STATS_   /* ------------------------------------------------- */
typedef I8 stats;
/* - defines vars for statistics    */
#ifdef _MAIN_PROG
stats bsizestats[S_MAXBIT][S_MAXS];
stats allbsizestats[S_MAXBIT][S_MAXS];
stats blkstats[S_MAXS];
stats allstats[S_MAXS];
stats mbstats[S_MAX_MAXBIT][S_MAXBIT][S_MAXS]; /* stats by maxbits */
I4 mbblks[S_MAX_MAXBIT];
I4 blkcount= 0;
#else   /* not _MAIN_PROG */
extern stats allbsizestats[S_MAXBIT][S_MAXS];
extern stats bsizestats[S_MAXBIT][S_MAXS];
extern stats blkstats[];
extern stats allstats[];
extern I4 blkcount;
extern stats mbstats[S_MAX_MAXBIT][S_MAXBIT][S_MAXS]; /* stats by maxbits */
extern I4 mbblks[S_MAX_MAXBIT];
#endif  /* _MAIN_PROG */




#ifdef BBLOCK
  #define USE_WORD_STATS
#endif
 
#ifdef SIMPLE
  #define USE_WORD_STATS
#endif
 
#ifdef CARRY
  #define USE_WORD_STATS
#endif
 
#ifdef SLIDE
  #define USE_WORD_STATS
#endif
 
#ifdef USE_WORD_STATS
#define UPDATE_WORD                          \
{                                            \
  I4 s=WORD_SIZE, i;                         \
	for (i=0; i<__pvalue; i++) s-= __bits[i];  \
  blkstats[WORDS]++;                         \
	blkstats[UBITS] += s;                      \
}
#else
#define UPDATE_WORD                          \
  blkstats[WORDS]++;     
#endif


#define UPDATE_GROUP(bitsize,selsize,elems,ibase)  \
{                                            \
  I4 rbits=0, ibits=0, k;                    \
  blkstats[SELS]++;                          \
	blkstats[SYM] = blkstats[SYM] + elems;                                  \
	blkstats[SELBITS] = blkstats[SELBITS] + selsize;                        \
	bsizestats[bitsize][SELS]++;                                            \
	bsizestats[bitsize][SYM] = bsizestats[bitsize][SYM] + elems;            \
	bsizestats[bitsize][SELBITS] = bsizestats[bitsize][SELBITS] + selsize;  \
	for (k=0; k<elems; k++) {                                               \
	  rbits = rbits + bits[ibase+k];                                            \
		ibits = ibits + bitsize - bits[ibase+k];                                  \
	}                                                                       \
	blkstats[INEFBITS] = blkstats[INEFBITS] + ibits;                        \
	blkstats[REALBITS] = blkstats[REALBITS] + rbits;                        \
	bsizestats[bitsize][INEFBITS] = bsizestats[bitsize][INEFBITS] + ibits;  \
	bsizestats[bitsize][REALBITS] = bsizestats[bitsize][REALBITS] +rbits;   \
}


/* DIRECT_UPDATE_GROUP(bitsize,selsize=control_bits, elems, base, regular=
                                       total_non-control_bits)  */
#define DIRECT_UPDATE_GROUP(bitsize,selsize,elems,ibase,regular)  \
{                                            \
  I4 rbits=0, ibits=0, k;                    \
  blkstats[SELS]++;                          \
	blkstats[SYM] = blkstats[SYM] + elems;                                  \
	blkstats[SELBITS] = blkstats[SELBITS] + selsize;                        \
	bsizestats[bitsize][SELS]++;                                            \
	bsizestats[bitsize][SYM] = bsizestats[bitsize][SYM] + elems;            \
	bsizestats[bitsize][SELBITS] = bsizestats[bitsize][SELBITS] + selsize;  \
	for (k=0; k<elems; k++) {                                               \
	  rbits = rbits + bits[ibase+k];                                            \
	}                                                                       \
	ibits= regular - rbits;                                                 \
	blkstats[INEFBITS] = blkstats[INEFBITS] + ibits;                        \
	blkstats[REALBITS] = blkstats[REALBITS] + rbits;                        \
	bsizestats[bitsize][INEFBITS] = bsizestats[bitsize][INEFBITS] + ibits;  \
	bsizestats[bitsize][REALBITS] = bsizestats[bitsize][REALBITS] +rbits;   \
}

/* ---- for Golomb and interp ---- */
#define UPDATE_SEL(_sel,_selbits) allstats[SELS] = allstats[SELS] + _sel; allstats[SELBITS] = allstats[SELBITS] + _selbits;

#define UPDATE_ELEM(_rbits,_ibits) allstats[REALBITS] =allstats[REALBITS] + _rbits; allstats[INEFBITS] = allstats[INEFBITS] + _ibits;

#define UPDATE_LOW_ELEM_MSG(_val,tmp, _bit) {        \
  if (tmp>_bit) {                                   \
    if (sizeof(_val) > 4)                            \
      fprintf(stderr,"WRONG val=%ld ceilog=%d coded with bit=%d\n",_val,tmp, _bit);  \
		else                                             \
		  fprintf(stderr,"WRONG val=%d ceilog=%d coded with bit=%d\n",_val,tmp, _bit);  \
	}                                                  \
}

#define UPDATE_LOW_ELEM(_val,_bit) {                 \
  I4 tmp;                                            \
  QZEROCEILLOG_2(_val, tmp)                          \
	UPDATE_LOW_ELEM_MSG(_val,tmp, _bit)                \
  UPDATE_ELEM(tmp, _bit-tmp)                         \
}
/* --------- END for Golomb and interp ---------- */


/* ---- for byte ----------------- */
#define BYTE_UPDATE_STATS(_x)      \
  allstats[SELS]++;                \
  allstats[SELBITS]++;             \
  if (_x>=128) {                   \
    allstats[REALBITS] += 7;       \
  } else {                         \
    I4 _tmp;                       \
    allstats[SYM]++;               \
    QZEROCEILLOG_2(_x,_tmp);       \
    allstats[REALBITS] += _tmp;    \
    allstats[INEFBITS] += (7 - _tmp); \
  }
/* --------- END for byte ----------------- */



#define PRINTSTATS(ofile,st,bst)             \
{                                            \
  I4 i,j;                                    \
  fprintf(ofile,"all   ");                  \
	for (j=SELBITS; j<8; j++) st[8] += st[j];        \
	for (j=0; j<9; j++) {                      \
	  fprintf(ofile," ");                      \
	  FPRNF_GINT(ofile,st[j],10);               \
	}                                          \
	fprintf(ofile,"\n\n");                     \
  for (i=0; i<S_MAXBIT; i++) {               \
	  for (j=SELBITS; j<8; j++) bst[i][8] += bst[i][j]; \
	  if (bst[i][8]) {                         \
		  FPRNF_GINT(ofile,i,6);                 \
			for (j=0; j<9; j++) {                  \
			  fprintf(ofile," ");                  \
				FPRNF_GINT(ofile,bst[i][j],10);       \
			}                                      \
			fprintf(ofile,"\n");                   \
		}                                        \
  }                                          \
	fprintf(ofile,"\n");                       \
}

#define PRINTMBSTATS(ofile)                  \
{                                            \
  I4 i,j,k;                                  \
	stats st[S_MAXS];                          \
  fprintf(ofile,"\n### %s Statistics by maxbit of each block\n",COMP_METHOD);      \
	fprintf(ofile,SHEADER);                    \
	for (k=0; k<S_MAX_MAXBIT; k++)             \
	if (mbblks[k]) {                           \
	  fprintf(ofile,"max_bit= %d blks= %d. Details: \n",k,mbblks[k]); \
		bzero(st,sizeof(stats)*S_MAXS);          \
    for (i=0; i<S_MAXBIT; i++)               \
		  for (j=0; j<S_MAXS; j++)               \
			  st[j] += mbstats[k][i][j];           \
		PRINTSTATS(ofile,st,mbstats[k])          \
	}                                          \
	fprintf(ofile,"\n");                       \
}


#define PRINTBLKSTATS(ofile,max_bits)        \
{                                            \
  fprintf(ofile,"Statistics for blk %d max_bit= %d\n",blkcount,max_bits); \
	fprintf(ofile,SHEADER);                    \
	PRINTSTATS(ofile,blkstats,bsizestats);     \
}


#define PRINTALLSTATS(ofile)                 \
{                                            \
  fprintf(ofile,"### %s Statistics for all %d blks\n",COMP_METHOD,blkcount); \
	fprintf(ofile,SHEADER);                    \
	PRINTSTATS(ofile,allstats,allbsizestats);  \
}




#define ZEROSTATS                            \
{                                            \
  bzero(bsizestats, sizeof(stats)*S_MAXBIT*S_MAXS);     \
	bzero(blkstats, sizeof(stats)*S_MAXS);         \
}

#define INITSTATS                            \
{                                            \
  bzero(allbsizestats, sizeof(stats)*S_MAXBIT*S_MAXS);  \
	bzero(allstats, sizeof(stats)*S_MAXS);         \
	bzero(mbstats,sizeof(stats)*S_MAX_MAXBIT*S_MAXBIT*S_MAXS); \
	bzero(mbblks,sizeof(I4)*S_MAX_MAXBIT);     \
	ZEROSTATS                                  \
}

#define BLKSTATS(ofile,max_bits)             \
{                                            \
  I4 i,j;                                    \
	for (j=0; j<S_MAXS; j++)                   \
	  allstats[j] += blkstats[j];              \
	for (i=0; i<S_MAXBIT; i++)                 \
	  for (j=0; j<S_MAXS; j++) {               \
		  allbsizestats[i][j] += bsizestats[i][j];     \
			mbstats[max_bits][i][j] += bsizestats[i][j]; \
	}                                          \
	mbblks[max_bits]++;                        \
	blkcount++;                                \
	/* PRINTBLKSTATS(ofile,max_bits) */             \
	ZEROSTATS                                  \
}

#define ENDSTATS(ofile) PRINTALLSTATS(ofile)




#else   /* ifdef _STATS_ */


#define INITSTATS
#define BLKSTATS(ofile,max_bits)
#define ENDSTATS(ofile)

#define UPDATE_WORD
#define UPDATE_GROUP(bitsize,selsize,elems,ibase)  
#define DIRECT_UPDATE_GROUP(bitsize,selsize,elems,ibase,regular)  

/* ---- for Golomb and interp ---- */
#define UPDATE_SEL(_sel,_selbits)
#define UPDATE_ELEM(_rbits,_ibits)
#define UPDATE_LOW_ELEM(_x,_logb)
/* --------- END for Golomb and interp ---------- */

/* ---- for byte ----------------- */
#define BYTE_UPDATE_STATS(_x)      



#endif  /* ifdef _STATS_ */

#endif   /* ifdef _STATS_H_  */
