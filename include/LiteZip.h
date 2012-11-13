#ifndef _LITEZIP_H
#define _LITEZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <limits.h>
#ifndef DWORD
#define WINAPI
typedef unsigned long DWORD;
typedef short WCHAR;
typedef void * HANDLE;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int BOOL;
#define MAX_PATH	PATH_MAX
#endif
#endif

/*
 * ZIP.H 
 *
 * For creating zip files using LITEZIP.DLL.
 *
 * This file is a repackaged form of extracts from the zlib code available
 * at www.gzip.org/zlib, by Jean-Loup Gailly and Mark Adler. The original
 * copyright notice may be found in LiteZip.c. The repackaging was done
 * by Lucian Wischik to simplify and extend its use in Windows/C++. Also
 * encryption and unicode filenames have been added. Code was further
 * revamped and turned into a DLL (which supports both UNICODE and ANSI
 * C strings) by Jeff Glatt. Linux version by Jeff Glatt.
 */

// An HZIP identifies a zip archive that is being created
#define HZIP	void *

// Functions to create a ZIP archive
DWORD WINAPI ZipCreateFileA(HZIP *, const char *, const char *);
DWORD WINAPI ZipCreateFileW(HZIP *, const WCHAR *, const char *);
#ifdef UNICODE
#define ZipCreateFile ZipCreateFileW
#define ZIPCREATEFILENAME "ZipCreateFileW"
typedef DWORD WINAPI ZipCreateFilePtr(HZIP *, const WCHAR *, const char *);
#else
#define ZipCreateFile ZipCreateFileA
#define ZIPCREATEFILENAME "ZipCreateFileA"
typedef DWORD WINAPI ZipCreateFilePtr(HZIP *, const char *, const char *);
#endif
DWORD WINAPI ZipCreateBuffer(HZIP *, void *, DWORD, const char *);
#define ZIPCREATEBUFFERNAME "ZipCreateBuffer"
typedef DWORD WINAPI ZipCreateBufferPtr(HZIP *, void *, DWORD, const char *);
DWORD WINAPI ZipCreateHandle(HZIP *, HANDLE, const char *);
#define ZIPCREATEHANDLENAME "ZipCreateHandle"
typedef DWORD WINAPI ZipCreateHandlePtr(HZIP *, HANDLE, const char *);

// Functions for adding a "file" to a ZIP archive
DWORD WINAPI ZipAddFileW(HZIP, const WCHAR *, const WCHAR *);
DWORD WINAPI ZipAddFileA(HZIP, const char *, const char *);
DWORD WINAPI ZipAddFileRawW(HZIP, const WCHAR *);
DWORD WINAPI ZipAddFileRawA(HZIP, const char *);
#ifdef UNICODE
#define ZipAddFile ZipAddFileW
#define ZIPADDFILENAME "ZipAddFileW"
typedef DWORD WINAPI ZipAddFilePtr(HZIP, const WCHAR *, const WCHAR *);
#define ZipAddFileRaw ZipAddFileRawW
#define ZIPADDFILERAWNAME "ZipAddFileRawW"
typedef DWORD WINAPI ZipAddFileRawPtr(HZIP, const WCHAR *);
#else
#define ZipAddFile ZipAddFileA
#define ZIPADDFILENAME "ZipAddFileA"
typedef DWORD WINAPI ZipAddFilePtr(HZIP, const char *, const char *);
#define ZipAddFileRaw ZipAddFileRawA
#define ZIPADDFILERAWNAME "ZipAddFileRawA"
typedef DWORD WINAPI ZipAddFileRawPtr(HZIP, const char *);
#endif

DWORD WINAPI ZipAddHandleW(HZIP, const WCHAR *, HANDLE);
DWORD WINAPI ZipAddHandleA(HZIP, const char *, HANDLE);
#ifdef UNICODE
#define ZipAddHandle ZipAddHandleW
#define ZIPADDHANDLENAME "ZipAddHandleW"
typedef DWORD WINAPI ZipAddHandlePtr(HZIP, const WCHAR *, HANDLE);
#else
#define ZipAddHandle ZipAddHandleA
#define ZIPADDHANDLENAME "ZipAddHandleA"
typedef DWORD WINAPI ZipAddHandlePtr(HZIP, const char *, HANDLE);
#endif
DWORD WINAPI ZipAddHandleRaw(HZIP, HANDLE);
#define ZIPADDHANDLERAWNAME "ZipAddHandleRaw"
typedef DWORD WINAPI ZipAddHandleRawPtr(HZIP, HANDLE);

DWORD WINAPI ZipAddPipeW(HZIP, const WCHAR *, HANDLE, DWORD);
DWORD WINAPI ZipAddPipeA(HZIP, const char *, HANDLE, DWORD);
#ifdef UNICODE
#define ZipAddPipe ZipAddPipeW
#define ZIPADDPIPENAME "ZipAddPipeW"
typedef DWORD WINAPI ZipAddPipePtr(HZIP, const WCHAR *, HANDLE, DWORD);
#else
#define ZipAddPipe ZipAddPipeA
#define ZIPADDPIPENAME "ZipAddPipeA"
typedef DWORD WINAPI ZipAddPipePtr(HZIP, const char *, HANDLE, DWORD);
#endif
DWORD WINAPI ZipAddPipeRaw(HZIP, HANDLE, DWORD);
#define ZIPADDPIPERAWNAME "ZipAddPipeRaw"
typedef DWORD WINAPI ZipAddPipeRawPtr(HZIP, HANDLE, DWORD);

DWORD WINAPI ZipAddBufferW(HZIP, const WCHAR *, const void *, DWORD);
DWORD WINAPI ZipAddBufferA(HZIP, const char *, const void *, DWORD);
#ifdef UNICODE
#define ZipAddBuffer ZipAddBufferW
#define ZIPADDBUFFERNAME "ZipAddFolderW"
typedef DWORD WINAPI ZipAddBufferPtr(HZIP, const WCHAR *, const void *, DWORD);
#else
#define ZipAddBuffer ZipAddBufferA
#define ZIPADDBUFFERNAME "ZipAddBufferA"
typedef DWORD WINAPI ZipAddBufferPtr(HZIP, const char *, const void *, DWORD);
#endif
DWORD WINAPI ZipAddBufferRaw(HZIP, const void *, DWORD);
#define ZIPADDBUFFERRAWNAME "ZipAddBufferRaw"
typedef DWORD WINAPI ZipAddBufferRawPtr(HZIP, const void *, DWORD);


DWORD WINAPI ZipAddFolderW(HZIP, const WCHAR *);
DWORD WINAPI ZipAddFolderA(HZIP, const char *);
#ifdef UNICODE
#define ZipAddFolder ZipAddFolderW
#define ZIPADDFOLDERNAME "ZipAddFolderW"
typedef DWORD WINAPI ZipAddFolderPtr(HZIP, const WCHAR *);
#else
#define ZipAddFolder ZipAddFolderA
#define ZIPADDFOLDERNAME "ZipAddFolderA"
typedef DWORD WINAPI ZipAddFolderPtr(HZIP, const char *);
#endif

// Function to get a pointer to the ZIP archive created in memory by ZipCreateBuffer(0, len)
DWORD WINAPI ZipGetMemory(HZIP, void **, unsigned long *, HANDLE *);
#define ZIPGETMEMORYNAME "ZipGetMemory"
typedef DWORD WINAPI ZipGetMemoryPtr(HZIP, void **, DWORD *, HANDLE *);

// Function to reset a memory-buffer ZIP archive to compress a new ZIP archive
DWORD WINAPI ZipResetMemory(HZIP);
#define ZIPRESETMEMORYNAME "ZipResetMemory"
typedef DWORD WINAPI ZipResetMemoryPtr(HZIP);

// Function to close the ZIP archive
DWORD WINAPI ZipClose(HZIP);
#define ZIPCLOSENAME "ZipClose"
typedef DWORD WINAPI ZipClosePtr(HZIP);

// Function to set options for ZipAdd* functions
DWORD WINAPI ZipOptions(HZIP, DWORD);
#define ZIPOPTIONSNAME "ZipOptions"
typedef DWORD WINAPI ZipOptionsPtr(HZIP, DWORD);

#define TZIP_OPTION_GZIP	0x80000000
#define TZIP_OPTION_ABORT	0x40000000

// Function to get an appropriate error message for a given error code return by Zip functions
DWORD WINAPI ZipFormatMessageW(DWORD, WCHAR *, DWORD);
DWORD WINAPI ZipFormatMessageA(DWORD, char *, DWORD);
#ifdef UNICODE
#define ZipFormatMessage ZipFormatMessageW
#define ZIPFORMATMESSAGENAME "ZipFormatMessageW"
typedef DWORD WINAPI ZipFormatMessagePtr(DWORD, WCHAR *, DWORD);
#else
#define ZipFormatMessage ZipFormatMessageA
#define ZIPFORMATMESSAGENAME "ZipFormatMessageA"
typedef DWORD WINAPI ZipFormatMessagePtr(DWORD, char *, DWORD);
#endif

#if !defined(ZR_OK)
// These are the return codes from Zip functions
#define ZR_OK			0		// Success
// The following come from general system stuff (e.g. files not openable)
#define ZR_NOFILE		1		// Can't create/open the file
#define ZR_NOALLOC		2		// Failed to allocate memory
#define ZR_WRITE		3		// A general error writing to the file
#define ZR_NOTFOUND		4		// Can't find the specified file in the zip
#define ZR_MORE			5		// There's still more data to be unzipped
#define ZR_CORRUPT		6		// The zipfile is corrupt or not a zipfile
#define ZR_READ			7		// An error reading the file
#define ZR_NOTSUPPORTED	8		// The entry is in a format that can't be decompressed by this Unzip add-on
// The following come from mistakes on the part of the caller
#define ZR_ARGS			9		// Bad arguments passed
#define ZR_NOTMMAP		10		// Tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
#define ZR_MEMSIZE		11		// The memory-buffer size is too small
#define ZR_FAILED		12		// Already failed when you called this function
#define ZR_ENDED		13		// The zip creation has already been closed
#define ZR_MISSIZE		14		// The source file size turned out mistaken
#define ZR_ZMODE		15		// Tried to mix creating/opening a zip 
// The following come from bugs within the zip library itself
#define ZR_SEEK			16		// trying to seek in an unseekable file
#define ZR_NOCHANGE		17		// changed its mind on storage, but not allowed
#define ZR_FLATE		18		// An error in the de/inflation code
#define ZR_PASSWORD		19		// Password is incorrect
#define ZR_ABORT		20		// Zipping aborted
#endif

#ifdef __cplusplus
}
#endif

#endif // _LITEZIP_H
