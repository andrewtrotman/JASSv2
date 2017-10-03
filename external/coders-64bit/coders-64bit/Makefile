VERSION = 1.0
SHELL = /bin/sh

srcdir = .
top_srcdir = .
cdir = ./c

CC = gcc
CFLAGS = -O3 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -I $(srcdir)/h
SFLAG = -DFAST_DECODE 
BFLAG = -D_BYTES_PER_WORD=8

LIBS = -lm
OBJS = $(cdir)/driver.c $(cdir)/codes.c $(cdir)/uncompress_io.c $(cdir)/word_align.c

EXEC = coders-64bit.ps coders-64bit.pdf Byte PForDelta Slide Carry Simple

all: $(EXEC)

Byte: $(OBJS)
	$(CC) $(CFLAGS) $(SFLAG) -DBYTE -o Byte $(OBJS) $(LIBS)

PForDelta: $(OBJS)
	$(CC) $(CFLAGS) $(SFLAG) $(BFLAG) -DUFORDELTA -o PForDelta $(OBJS) $(LIBS)

Slide: $(OBJS)
	$(CC) $(CFLAGS) $(BFLAG) -DSLIDE -o Slide $(OBJS) $(LIBS)

Carry: $(OBJS)
	$(CC) $(CFLAGS) $(SFLAG) $(BFLAG) -DCARRY -o Carry $(OBJS) $(LIBS)

Simple: $(OBJS)
	$(CC) $(CFLAGS) $(SFLAG) $(BFLAG) -DSIMPLE -o Simple $(OBJS) $(LIBS)

coders-64bit.ps: coders-64bit.1
	groff -man coders-64bit.1 > coders-64bit.ps

coders-64bit.pdf: coders-64bit.ps
	ps2pdf coders-64bit.ps

clean:
	/bin/rm -f $(EXEC) core *.o *.bak

package:
	tar cv Makefile README c/ h/ coders-64bit.1 | gzip > coders-64bit-1.0.tar.gz

