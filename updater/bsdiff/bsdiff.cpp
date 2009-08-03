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

#include "bsdiff.h"
#include <errno.h>

extern int errno;

void err(char a, char* format, char* param)
{
    printf(format,param);
    exit(1);
}
void err(char a, char* format)
{
    printf(format);
    exit(1);
}

void split(off_t *I,off_t *V,off_t start,off_t len,off_t h)
{
	off_t i,j,k,x,tmp,jj,kk;

	if(len<16) {
		for(k=start;k<start+len;k+=j) {
			j=1;x=V[I[k]+h];
			for(i=1;k+i<start+len;i++) {
				if(V[I[k+i]+h]<x) {
					x=V[I[k+i]+h];
					j=0;
				};
				if(V[I[k+i]+h]==x) {
					tmp=I[k+j];I[k+j]=I[k+i];I[k+i]=tmp;
					j++;
				};
			};
			for(i=0;i<j;i++) V[I[k+i]]=k+j-1;
			if(j==1) I[k]=-1;
		};
		return;
	};

	x=V[I[start+len/2]+h];
	jj=0;kk=0;
	for(i=start;i<start+len;i++) {
		if(V[I[i]+h]<x) jj++;
		if(V[I[i]+h]==x) kk++;
	};
	jj+=start;kk+=jj;

	i=start;j=0;k=0;
	while(i<jj) {
		if(V[I[i]+h]<x) {
			i++;
		} else if(V[I[i]+h]==x) {
			tmp=I[i];I[i]=I[jj+j];I[jj+j]=tmp;
			j++;
		} else {
			tmp=I[i];I[i]=I[kk+k];I[kk+k]=tmp;
			k++;
		};
	};

	while(jj+j<kk) {
		if(V[I[jj+j]+h]==x) {
			j++;
		} else {
			tmp=I[jj+j];I[jj+j]=I[kk+k];I[kk+k]=tmp;
			k++;
		};
	};

	if(jj>start) split(I,V,start,jj-start,h);

	for(i=0;i<kk-jj;i++) V[I[jj+i]]=kk-1;
	if(jj==kk-1) I[jj]=-1;

	if(start+len>kk) split(I,V,kk,start+len-kk,h);
}

void qsufsort(off_t *I,off_t *V,u_char *old,off_t oldsize)
{
	off_t buckets[256];
	off_t i,h,len;

	for(i=0;i<256;i++) buckets[i]=0;
	for(i=0;i<oldsize;i++) buckets[old[i]]++;
	for(i=1;i<256;i++) buckets[i]+=buckets[i-1];
	for(i=255;i>0;i--) buckets[i]=buckets[i-1];
	buckets[0]=0;

	for(i=0;i<oldsize;i++) I[++buckets[old[i]]]=i;
	I[0]=oldsize;
	for(i=0;i<oldsize;i++) V[i]=buckets[old[i]];
	V[oldsize]=0;
	for(i=1;i<256;i++) if(buckets[i]==buckets[i-1]+1) I[buckets[i]]=-1;
	I[0]=-1;

	for(h=1;I[0]!=-(oldsize+1);h+=h) {
		len=0;
		for(i=0;i<oldsize+1;) {
			if(I[i]<0) {
				len-=I[i];
				i-=I[i];
			} else {
				if(len) I[i-len]=-len;
				len=V[I[i]]+1-i;
				split(I,V,i,len,h);
				i+=len;
				len=0;
			};
		};
		if(len) I[i-len]=-len;
	};

	for(i=0;i<oldsize+1;i++) I[V[i]]=i;
}

off_t matchlen(u_char *old,off_t oldsize,u_char *pnew,off_t newsize)
{
	off_t i;

	for(i=0;(i<oldsize)&&(i<newsize);i++)
		if(old[i]!=pnew[i]) break;

	return i;
}

off_t search(off_t *I,u_char *old,off_t oldsize,
		u_char *pnew,off_t newsize,off_t st,off_t en,off_t *pos)
{
	off_t x,y;

	if(en-st<2) {
		x=matchlen(old+I[st],oldsize-I[st],pnew,newsize);
		y=matchlen(old+I[en],oldsize-I[en],pnew,newsize);

		if(x>y) {
			*pos=I[st];
			return x;
		} else {
			*pos=I[en];
			return y;
		}
	};

	x=st+(en-st)/2;
	if(memcmp(old+I[x],pnew,MIN(oldsize-I[x],newsize))<0) {
		return search(I,old,oldsize,pnew,newsize,x,en,pos);
	} else {
		return search(I,old,oldsize,pnew,newsize,st,x,pos);
	};
}

void offtout(off_t x,u_char *buf)
{
	off_t y;

	if(x<0) y=-x; else y=x;

		buf[0]=y%256;y-=buf[0];
	y=y/256;buf[1]=y%256;y-=buf[1];
	y=y/256;buf[2]=y%256;y-=buf[2];
	y=y/256;buf[3]=y%256;y-=buf[3];
	y=y/256;buf[4]=y%256;y-=buf[4];
	y=y/256;buf[5]=y%256;y-=buf[5];
	y=y/256;buf[6]=y%256;y-=buf[6];
	y=y/256;buf[7]=y%256;

	if(x<0) buf[7]|=0x80;
}

/*int main(int argc,char *argv[])*/
/**
 * I have rewritten this function so it can be called from other places;
 * I have also rewritten it to not use a patch file but instead a passed
 * buffer; the bsdiff patch is already in memory.
 *
 * ryan, 7.2.09
 */
int bsdiff_create_patch(char *patchFile, char *oldFile, char *newFile)
{
	FILE* fd = NULL;
	BZFILE * bz = NULL;
	u_char *old,*pnew;
	off_t oldsize,newsize;
	off_t *I,*V;

	off_t scan,pos,len;
	off_t lastscan,lastpos,lastoffset;
	off_t oldscore,scsc;

	off_t s,Sf,lenf,Sb,lenb;
	off_t overlap,Ss,lens;
	off_t i;

	off_t dblen,eblen;
	u_char *db,*eb;

	u_char buf[8];
	u_char header[32];

	int bzerror;

	/*if(argc!=4) err(1,"usage: %s oldfile newfile patchfile\n",argv[0]);*/

	/* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd = fopen(oldFile, "rb")) <= 0) ||
		(fseek(fd, 0, SEEK_END) != 0) ||
		((oldsize = ftell(fd)) == 0) ||
		((old = (u_char *) malloc(oldsize + 1)) == NULL) ||
		(fseek(fd, 0, SEEK_SET) != 0) ||
		(fread(old,1, oldsize, fd) != oldsize) ||
		(fclose(fd) == -1)) err(1, "%s", oldFile);

	if(((I = (off_t*) malloc((oldsize + 1) * sizeof(off_t))) == NULL) ||
		((V = (off_t*) malloc((oldsize + 1) * sizeof(off_t))) == NULL)) err(1, NULL);

	qsufsort(I, V, old, oldsize);

	free(V);

	/* Allocate newsize+1 bytes instead of newsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd = fopen(newFile, "rb")) < 0) ||
		(fseek(fd, 0, SEEK_END) != 0) ||
		((newsize = ftell(fd)) == 0) ||
		((pnew = (u_char *) malloc(newsize + 1)) == NULL) ||
		(fseek(fd, 0, SEEK_SET) != 0) ||
		(fread(pnew, 1, newsize, fd) != newsize) ||
		(fclose(fd) == -1)) err(1, "%s", newFile);

	if(((db=(u_char*)malloc(newsize+1))==NULL) ||
		((eb=(u_char*)malloc(newsize+1))==NULL)) err(1,NULL);
	dblen=0;
	eblen=0;

	fd = fopen(patchFile, "wb");
	if(fd == NULL)
		err(1,"%s",patchFile);

	/* Header is
		0	8	 "BSDIFF40"
		8	8	length of bzip2ed ctrl block
		16	8	length of bzip2ed diff block
		24	8	length of new file */
	/* File is
		0	32	Header
		32	??	Bzip2ed ctrl block
		??	??	Bzip2ed diff block
		??	??	Bzip2ed extra block */
	memcpy(header,"BSDIFF40",8);
	memset(header+8,0,24);
	if(fwrite(header,1,32,fd)!=32) err(1,"%s",patchFile);


	scan=0;len=0;
	lastscan=0;lastpos=0;lastoffset=0;

	bz = BZ2_bzWriteOpen ( &bzerror, fd, 
                         blockSize100k, 0, 30);
    if (bzerror != BZ_OK) err(1, "Problem bzWriteOpen");
    
	while(scan<newsize) {
		oldscore=0;

		for(scsc=scan+=len;scan<newsize;scan++) {
			len=search(I,old,oldsize,pnew+scan,newsize-scan,
					0,oldsize,&pos);

			for(;scsc<scan+len;scsc++)
			if((scsc+lastoffset<oldsize) &&
				(old[scsc+lastoffset] == pnew[scsc]))
				oldscore++;

			if(((len==oldscore) && (len!=0)) || 
				(len>oldscore+8)) break;

			if((scan+lastoffset<oldsize) &&
				(old[scan+lastoffset] == pnew[scan]))
				oldscore--;
		};

		if((len!=oldscore) || (scan==newsize)) {
			s=0;Sf=0;lenf=0;
			for(i=0;(lastscan+i<scan)&&(lastpos+i<oldsize);) {
				if(old[lastpos+i]==pnew[lastscan+i]) s++;
				i++;
				if(s*2-i>Sf*2-lenf) { Sf=s; lenf=i; };
			};

			lenb=0;
			if(scan<newsize) {
				s=0;Sb=0;
				for(i=1;(scan>=lastscan+i)&&(pos>=i);i++) {
					if(old[pos-i]==pnew[scan-i]) s++;
					if(s*2-i>Sb*2-lenb) { Sb=s; lenb=i; };
				};
			};

			if(lastscan+lenf>scan-lenb) {
				overlap=(lastscan+lenf)-(scan-lenb);
				s=0;Ss=0;lens=0;
				for(i=0;i<overlap;i++) {
					if(pnew[lastscan+lenf-overlap+i]==
					   old[lastpos+lenf-overlap+i]) s++;
					if(pnew[scan-lenb+i]==
					   old[pos-lenb+i]) s--;
					if(s>Ss) { Ss=s; lens=i+1; };
				};

				lenf+=lens-overlap;
				lenb-=lens;
			};

			for(i=0;i<lenf;i++)
				db[dblen+i]=pnew[lastscan+i]-old[lastpos+i];
			for(i=0;i<(scan-lenb)-(lastscan+lenf);i++)
				eb[eblen+i]=pnew[lastscan+lenf+i];

			dblen+=lenf;
			eblen+=(scan-lenb)-(lastscan+lenf);

			offtout(lenf,buf);
			if((8 != BZ2_bzwrite(bz,buf,8)) || (bzerror != BZ_OK)) err(1,"bzwrite");
			offtout((scan-lenb)-(lastscan+lenf),buf);
			if((8 != BZ2_bzwrite(bz,buf,8)) || (bzerror != BZ_OK)) err(1,"bzwrite");
			offtout((pos-lenb)-(lastpos+lenf),buf);
			if((8 != BZ2_bzwrite(bz,buf,8)) || (bzerror != BZ_OK)) err(1,"bzwrite");

			lastscan=scan-lenb;
			lastpos=pos-lenb;
			lastoffset=pos-scan;
		};
	};
	BZ2_bzWriteClose ( &bzerror, bz, 0, NULL, NULL);

	if((fseek(fd,0,SEEK_END))!=0 || ((len=ftell(fd))==-1)) err(1,"problem 0: %s",patchFile);
	offtout(len-32,buf);
	if((fseek(fd,8,SEEK_SET)!=0) || (ftell(fd) != 8) || (fwrite(buf,1,8,fd)!=8))
		err(1,"problem 1: %s",patchFile);
	offtout(newsize,buf);
	if((fseek(fd,24,SEEK_SET)!=0) || (ftell(fd) != 24)|| (fwrite(buf,1,8,fd)!=8))
		err(1,"problem 2:%s",patchFile);

	if(fseek(fd,0,SEEK_END)!=0) err(1,"problem 3: %s",patchFile);

	bz = BZ2_bzWriteOpen ( &bzerror, fd, 
                         blockSize100k, 0, 30);
    if (bzerror != BZ_OK) err(1, "Problem bzWriteOpen");
	if((dblen !=  BZ2_bzwrite(bz,db,dblen)) || (bzerror != BZ_OK)) 
	{
	    // printf("%s", BZ2_bzerror ( bz, &bzerror ));
       err(1,"bzwrite");
    }   
	BZ2_bzWriteClose ( &bzerror, bz, 0, NULL, NULL);



	if((fseek(fd,0,SEEK_END)!=0) || ((newsize=ftell(fd))==-1)) err(1,"problem 4: %s",patchFile);
	offtout(newsize-len,buf);
	if((fseek(fd,16,SEEK_SET)!=0) || (ftell(fd)!=16) || (fwrite(buf,1,8,fd)!=8))
		err(1,"problem 5:%s",patchFile);

	if(fseek(fd,0,SEEK_END)!=0 || (ftell(fd) == -1)) err(1,"problem 6: %s",patchFile);

	bz = BZ2_bzWriteOpen ( &bzerror, fd, 
                         blockSize100k, 0, 30);
    if (bzerror != BZ_OK) err(1, "Problem bzWriteOpen");
	if((eblen != BZ2_bzwrite(bz,eb,eblen)) || (bzerror != BZ_OK)) err(1,"bzwrite");
	BZ2_bzWriteClose ( &bzerror, bz, 0, NULL, NULL);
 	fclose(fd);
	free(db);
	free(eb);
	free(I);
	free(old);
	free(pnew);

	return 0;
}