// cgiref.cpp
//
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
// This program return the url passed as a parameter and log tho origin of
// the caller in a log file

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define LOG_FILE "log/cgiref"

static const char *chk(const char *pSrc);

int main(int pArgc, char **pArgs)
{
	char lLineBuffer[2048];

	if(pArgc < 2) {
		printf("Content-type: text/plain%c%c", 10, 10);
		printf("Error");
	}
	else {
		const char *lLocation = strchr(pArgs[1], '^');

		if(lLocation != NULL) {
			lLocation++;
		}
		else {
			lLocation = pArgs[1];
		}

		// Return the location string
		printf("Location: %s%c%c", lLocation, 10, 10);

		// Retrieve time
		char lTimeBuffer[256];
		time_t lTimer;

		lTimer = time(NULL);

		struct tm *lTime = localtime(&lTimer);

		strcpy(lTimeBuffer, "error");
		strftime(lTimeBuffer, sizeof(lTimeBuffer), "%y-%m-%d_%H:%M:%S", lTime);

		// Log the request
		char *lPathEnd = strrchr(pArgs[0], '/');

		if(lPathEnd != NULL) {
			*lPathEnd = 0;
			chdir(pArgs[0]);
		}

		sprintf(lLineBuffer, "* %s %s %s %s %s\n", lTimeBuffer, pArgs[1], chk(getenv("REMOTE_ADDR")), chk(getenv("REMOTE_HOST")), chk(getenv("REMOTE_USER"))
			);

		// output the string
		FILE *lFile = fopen(LOG_FILE, "a");

		if(lFile != NULL) {
			fprintf(lFile, lLineBuffer);
			fclose(lFile);
		}

	}

	return 0;
}

const char *chk(const char *pSrc)
{
	if(pSrc == NULL) {
		return "null";
	}
	else {
		return pSrc;
	}
}

void Append(char *pDest, const char *pStr, int pEoL)
{
	if(pStr == NULL) {
		pStr = "null";
	}

	strcat(pDest, pStr);

	if(pEoL) {
		strcat(pDest, "\n");
	}
	else {
		strcat(pDest, " ");
	}
}
