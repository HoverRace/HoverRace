// main.cpp              Fireball game entry point
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
#include "StdAfx.h"

#ifdef _WIN32
#include "GameApp.h"
#endif

#ifdef _WIN32
#	include <direct.h>
#endif

#include <iostream>

#include <curl/curl.h>

static bool safeMode = false;
static bool allowMultipleInstances = false;

/**
 * Process command-line options.
 * @param argc The arg count.
 * @param argv The original argument list.
 */
static void ProcessCmdLine(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];

		if (strcmp("-s", arg) == 0) {
			safeMode = true;
		}
		else if (strcmp("-m", arg) == 0) {
			allowMultipleInstances = true;
		}
	}
}

// Entry point
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE pPrevInstance, LPSTR /* pCmdLine */ , int pCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	// initialize return variables
	BOOL lReturnValue = TRUE;
	int lErrorCode = EXIT_SUCCESS;

#ifdef _WIN32
	char exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH - 1);

	// Change the working directory to the app's directory.
	char *appPath = strdup(exePath);
	char *appDiv = strrchr(appPath, '\\');
	*appDiv = '\0';
	chdir(appPath);
	free(appPath);
#endif

	// Process command-line options.
#ifdef _WIN32
	int argc = __argc;
	char** argv = __argv;
#endif
	ProcessCmdLine(argc, argv);

#ifdef ENABLE_NLS
	// Gettext initialization.
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	// Library initialization.
	curl_global_init(CURL_GLOBAL_ALL);

#ifdef _WIN32
	MR_GameApp lGame(pInstance, safeMode);

	// Allow only one instance of HoverRace; press CAPS_LOCK to bypass or
	// use the "-m" command-line option.
	if (!allowMultipleInstances) {
		GetAsyncKeyState(VK_CAPITAL);				  // Reset the function
		if(!GetAsyncKeyState(VK_CAPITAL))
			lReturnValue = lGame.IsFirstInstance();
	}

	if(lReturnValue && (pPrevInstance == NULL))
		lReturnValue = lGame.InitApplication();

	if(lReturnValue)
		lReturnValue = lGame.InitGame();

	// this is where the game actually takes control
	if(lReturnValue)
		lErrorCode = lGame.MainLoop();
#else
	std::cout << boost::format(_("HoverRace for Linux is under development!\n"
		"Please visit %s to learn how to\n"
		"contribute to this project.")) % "http://svn.igglybob.com/hoverrace/" <<
		std::endl;
#endif

	// Library cleanup.
	curl_global_cleanup();

	return lErrorCode;
}
