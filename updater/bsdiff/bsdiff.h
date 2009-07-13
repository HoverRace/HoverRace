/*
  bsdiff.c -- Binary patch generator.

  Copyright 2003 Colin Percival

  For the terms under which this work may be distributed, please see
  the adjoining file "LICENSE".
  
  Changes 23 October 2004 by Timotheus Pokorra (timotheus@pokorra.de):
  	compiles now under Dev-C++ on Windows.
  	uses bzlib, available as a package for Dev-C++
  
*/

/* this needs to be compiled with a C++ compiler because even though it is
 * by default named bspatch.c it uses C++-specific functions.
 * - ryan */

#ifndef BSDIFF_BSDIFF_H
#define BSDIFF_BSDIFF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	
	// for ssize_t; don't use SSIZE_T, or we don't retain original Linux compatibility
	#include <BaseTsd.h>
	typedef SSIZE_T ssize_t;
	typedef long off_t;
#else
	#include <unistd.h>
#endif

#include <fcntl.h>
#include "../bzlib/bzlib.h"

const int blockSize100k = 5; 
    // It should be a value between 1 and 9 inclusive, 
    // and the actual block size used is 100000 x this figure. 
    // 9 gives the best compression but takes most memory.

typedef unsigned char u_char;

void err(char a, char* format, char* param);
void err(char a, char* format);

#define MIN(x,y) (((x)<(y)) ? (x) : (y))

void split(off_t *I,off_t *V,off_t start,off_t len,off_t h);

void qsufsort(off_t *I,off_t *V,u_char *old,off_t oldsize);

off_t matchlen(u_char *old,off_t oldsize,u_char *pnew,off_t newsize);

off_t search(off_t *I,u_char *old,off_t oldsize,
		u_char *pnew,off_t newsize,off_t st,off_t en,off_t *pos);

void offtout(off_t x,u_char *buf);

/*int main(int argc,char *argv[])*/
/**
 * I have rewritten this function so it can be called from other places;
 * I have also rewritten it to not use a patch file but instead a passed
 * buffer; the bsdiff patch is already in memory.
 *
 * ryan, 7.2.09
 */
int bsdiff_create_patch(char *patchFile, char *oldFile, char *newFile);

#endif