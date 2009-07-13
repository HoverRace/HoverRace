/*
  .c -- Binary patcher

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
//#ifndef BZIP2
//#define BZIP2 "/usr/bin/bzip2"
//#endif

#ifndef BSDIFF_BSPATCH_H
#define BSDIFF_BSPATCH_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* allow Windows-based systems to use this */
#ifdef _WIN32
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

typedef unsigned char u_char;

void errx(char a, char* format, char* param);
void errx(char a, char* format);

ssize_t loopread(BZFILE* d,int*bzerror,u_char *buf,size_t nbytes);

FILE* bz2read(BZFILE** bz, int* bzerror, off_t offset,off_t len,char * fname);

off_t offtin(u_char *buf);

//int main(int argc,char * argv[])
/**
 * I have rewritten this function so it can be called from other places;
 * I have also rewritten it to not use a patch file but instead a passed
 * buffer; the bsdiff patch is already in memory.
 *
 * ryan, 7.2.09
 */
int bsdiff_patch_file(char *patchFile, char *oldFile, char *newFile);

#endif