// orderno.cpp

//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//
//

// HoverRace order parser Unix or Win32.
//
// This is a CGI application
//
//   URL?=file,majorid,minorid
//

// #define _FAST_CGI_

#ifdef _FAST_CGI_
#include "/usr/local/fcgi/include/fcgi_stdio.h"
#else
// #include <stdio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

// Required section to compile on Windows
#ifdef WIN32

#include <direct.h>
#include <io.h>

#else

#include <unistd.h>
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Implementation

int main(int pArgc, const char **pArgs)
{

	// Send the header required by the server
	printf("Content-type: text/html%c%c", 10, 10);
	// printf("Content-type: text/html%c%c", 10, 10);

	const char *lQueryPtr = getenv("QUERY_STRING");

	char lFileName[30];
	int lMajor = 0;
	int lMinor = 0;

	lFileName[0] = 0;

	if(lQueryPtr != NULL) {
		sscanf(lQueryPtr, "=%30[^,],%d,%d", lFileName, &lMajor, &lMinor);
	}
#ifndef _FAST_CGI_
	// Change the local directory
	char *lPathEnd = strrchr(pArgs[0], '/');

	if(lPathEnd != NULL) {
		*lPathEnd = 0;
		chdir(pArgs[0]);
	}
#endif

	if(strlen(lFileName) <= 0) {
		printf("Wrong parameters please contact webmaster@grokksoft.com\n\n");
	}
	else {
		FILE *lFile = fopen(lFileName, "r");

		if(lFile == NULL) {
			printf("Bad file name please contact webmaster@grokksoft.com\n\n");

		}
		else {
			char lMajorStr[15];
			char lMinorStr[15];

			char lLineBuffer[1024];

			sprintf(lMajorStr, "%06d", lMajor);
			sprintf(lMinorStr, "%06d", lMinor);

			while(fgets(lLineBuffer, 1024, lFile) != NULL) {
				// parse the string
				char *lPtr = lLineBuffer;

				while(*lPtr != 0) {
					if(*lPtr == '%') {
						if(!strncmp(lPtr, "%MAJOR", 6)) {
							memcpy(lPtr, lMajorStr, 6);
						}
						else if(!strncmp(lPtr, "%MINOR", 6)) {
							memcpy(lPtr, lMinorStr, 6);
						}
					}
					lPtr++;
				}

				printf("%s", lLineBuffer);
			}
			printf("\n\n");
			fclose(lFile);
		}

	}

	return 0;
}
