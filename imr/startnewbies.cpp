// startnewbiess.cpp
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
// CGI script that restart the newbies room

#include <stdio.h>
#include <string.h>

int main(int pArgc, char **pArgs)
{

	if(fork() == 0) {
		// Child process
		// Close pipes
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);

		// Change user
		setuid(geteuid());
		setgid(getegid());

		// Change directory
		char *lPathEnd = strrchr(pArgs[0], '/');

		if(lPathEnd != NULL) {
			*lPathEnd = 0;
			chdir(pArgs[0]);
		}

		chdir("../irserver");

		// exec
		execl("ir3.exe", "ir3.exe", "12360", "log/new.log", NULL);
	}
	else {
		// Parent process
		printf("Content-type: text/html%c%c", 10, 10);
		printf("<html><body>Trying to restart the room..<br><br><br>..retry if not working</body></html>\n\n");

	}
	return 0;
}
