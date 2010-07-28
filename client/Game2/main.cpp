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

#ifdef WITH_SDL
#	include "ClientApp.h"
#else
#	include "GameApp.h"
#endif

#ifdef _WIN32
#	include <direct.h>
#endif

#include <iostream>

#include <curl/curl.h>

#include "../../engine/Exception.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"

#ifndef _WIN32
#	include "version.h"
#endif

using boost::format;
using boost::str;

#ifdef WITH_SDL
using HoverRace::Client::ClientApp;
#else
using HoverRace::Client::GameApp;
#endif
using HoverRace::Util::Config;
using HoverRace::Util::OS;
namespace Str = HoverRace::Util::Str;

#if 0
#if defined(_WIN32) && defined(_DEBUG)
#	include <mfcleakfix.h>
	static int foo = use_ignore_mfc_leaks();
#endif
#endif

#ifdef _WIN32
	static HINSTANCE hinstance;
#endif

static OS::path_t initScript;
static bool debugMode = false;
static bool safeMode = false;
static bool allowMultipleInstances = false;
static bool showVersion = false;
static bool silentMode = false;
static bool experimentalMode =
#	ifdef _WIN32
		false;
#	else
		true;  // Always use experimental mode in non-Win32.
#	endif
static bool showFramerate = false;

/**
 * Display a message to the user.
 * On Windows, this will pop up a message box
 */
static void ShowMessage(const std::string &s)
{
#ifdef _WIN32
	MessageBox(NULL, s.c_str(), PACKAGE_NAME, MB_OK);
#else
	std::cout << s << std::endl;
#endif
}

/**
 * Process command-line options.
 * @param argc The arg count.
 * @param argv The original argument list.
 * @return @c true if successful.
 */
static bool ProcessCmdLine(int argc, char **argv)
{
#	ifdef _WIN32
		int wargc;
		wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
		ASSERT(argc == wargc);
#	endif

	for (int i = 1; i < argc; ) {
		const char *arg = argv[i++];

		if (strcmp("-D", arg) == 0) {
			debugMode = true;
		}
		else if (strcmp("--exec", arg) == 0) {
			if (i < argc) {
#				ifdef _WIN32
					initScript = wargv[i++];
#				else
					initScript = argv[i++];
#				endif
			}
			else {
				ShowMessage("Expected: --exec (script filename)");
				return false;
			}
		}
		else if (strcmp("-L", arg) == 0) {
			if (i < argc) {
				arg = argv[i++];
				// Set the language by updating the environment.
				// This may allow for more languages than setlocale() allows.
				OS::SetEnv("LC_ALL", arg);
			}
			else {
				ShowMessage("Expected: -L (language)");
				return false;
			}
		}
		else if (strcmp("--fps", arg) == 0) {
			showFramerate = true;
		}
		else if (strcmp("-m", arg) == 0) {
			allowMultipleInstances = true;
		}
		else if (strcmp("-s", arg) == 0) {
			safeMode = true;
		}
		else if (strcmp("--silent", arg) == 0) {
			silentMode = true;
		}
		else if (strcmp("-V", arg) == 0 || strcmp("--version", arg) == 0) {
			showVersion = true;
		}
#ifdef _WIN32
		else if (strcmp("--yes-i-totally-want-to-break-my-system", arg) == 0) {
			experimentalMode = true;
		}
#endif
	}

#	ifdef _WIN32
		LocalFree(wargv);
#	endif

	return true;
}

// Initialize (but not load) the config system.
static Config *InitConfig(
#ifdef _WIN32
	const OS::path_t &exePath
#endif
	)
{
#ifdef _WIN32
	// Load our own version info so we can pass it along to the config.
	long verMajor = 0;
	long verMinor = 0;
	long verPatch = 0;
	long verBuild = 0;
	bool prerelease = true;
	DWORD dummyHandle;
	DWORD verInfoSize = GetFileVersionInfoSizeW(exePath.file_string().c_str(), &dummyHandle);
	void *verInfo = malloc(verInfoSize);
	if (GetFileVersionInfoW(exePath.file_string().c_str(), 0, verInfoSize, verInfo)) {
		UINT outSize;
		void *outPtr;
		if (VerQueryValueW(verInfo, L"\\", (LPVOID*)&outPtr, &outSize)) {
			VS_FIXEDFILEINFO *fixedInfo = (VS_FIXEDFILEINFO*)outPtr;
			verMajor = (fixedInfo->dwProductVersionMS >> 16) & 0xffff;
			verMinor = fixedInfo->dwProductVersionMS & 0xffff;
			verPatch = (fixedInfo->dwProductVersionLS >> 16) & 0xffff;
			verBuild = fixedInfo->dwProductVersionLS & 0xffff;
			prerelease = (fixedInfo->dwFileFlagsMask & VS_FF_PRERELEASE & fixedInfo->dwFileFlags) > 0;
		}
	}
	free(verInfo);

	// Format the version.
	if (verMajor == 0 && verMinor == 0 && verPatch == 0 && verBuild == 0) {
		//FIXME: Oh bother, this means the .exe was compiled without
		//       version resources.  What do we do now?
	}

	// Load the configuration, using the default OS-specific path.
	return Config::Init(verMajor, verMinor, verPatch, verBuild, prerelease);

#else
	
	//FIXME: Extract prerelease flag from ver resource at build time.
	return Config::Init(HR_APP_VERSION, HR_APP_VERSION_PRERELEASE);

#endif
}

#ifdef _WIN32
/**
 * Find the path of the executable itself.
 * @return The path, including the executable name.
 */
static OS::path_t FindExePath()
{
	DWORD curSize = MAX_PATH;
	wchar_t *exePath = new wchar_t[curSize];
	for (;;) {
		DWORD ret = GetModuleFileNameW(NULL, exePath, curSize);
		if (ret == 0) {
			ShowMessage("Internal Error: GetModuleFileName() failed.");
			exit(EXIT_FAILURE);
		}
		else if (ret >= curSize) {
			curSize *= 2;
			delete[] exePath;
			exePath = new wchar_t[curSize];
		}
		else {
			break;
		}
	} 
	OS::path_t retv(exePath);
	delete[] exePath;
	return retv;
}
#endif

static int RunClient()
{
#	ifdef WITH_SDL
		ClientApp game;
		game.MainLoop();
		return EXIT_SUCCESS;

#	elif _WIN32
		BOOL lReturnValue = TRUE;

		GameApp lGame(hinstance, safeMode);

		// Allow only one instance of HoverRace; press CAPS_LOCK to bypass or
		// use the "-m" command-line option.
		if (!allowMultipleInstances) {
			GetAsyncKeyState(VK_CAPITAL);				  // Reset the function
			if(!GetAsyncKeyState(VK_CAPITAL))
				lReturnValue = lGame.IsFirstInstance();
		}

		if(lReturnValue)
			lReturnValue = lGame.InitApplication();

		if(lReturnValue)
			lReturnValue = lGame.InitGame();

		// this is where the game actually takes control
		if(lReturnValue) {
			return lGame.MainLoop();
		}
		else {
			return EXIT_FAILURE;
		}

#	endif
}

// Entry point
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE, LPSTR, int pCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	// initialize return variables
	int lErrorCode = EXIT_SUCCESS;

#ifdef _WIN32
	hinstance = pInstance;

	OS::path_t exePath = FindExePath();

	// Change the working directory to the app's directory.
	_wchdir(exePath.parent_path().file_string().c_str());

	CoInitialize(NULL);
#endif

	// Process command-line options.
#ifdef _WIN32
	int argc = __argc;
	char** argv = __argv;
#endif
	if (!ProcessCmdLine(argc, argv))
		return EXIT_FAILURE;

	Config *cfg = InitConfig(
#ifdef _WIN32
		exePath
#endif
		);
	cfg->runtime.silent = silentMode;
	cfg->runtime.aieeee = experimentalMode;
	cfg->runtime.showFramerate = showFramerate;
	cfg->runtime.initScript = initScript;

#ifdef ENABLE_NLS
	// Gettext initialization.
	OS::SetLocale();
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	bindtextdomain(PACKAGE, debugMode ? "share/locale" : LOCALEDIR);
	textdomain(PACKAGE);
#endif

	// If --version was passed, output the version and exit.
	if (showVersion) {
		// We intentionally do not localize this string since scripts
		// may want to parse it (for whatever reason).
		ShowMessage(PACKAGE_NAME " version " + cfg->GetVersion());
		return EXIT_SUCCESS;
	}

	// Library initialization.
	curl_global_init(CURL_GLOBAL_ALL);

	OS::TimeInit();

	try {
		lErrorCode = RunClient();
	}
	catch (HoverRace::Exception &ex) {
		//TODO: Managed error handler.
#		ifdef _WIN32
			std::ostringstream oss;
			oss <<
				_("You found a bug!") << "\r\n\r\n" <<
				ex.what();
			MessageBoxW(NULL, Str::UW(oss.str()), PACKAGE_NAME_L, MB_OK | MB_ICONERROR);
#		else
			std::cerr <<
				"***\n\n" <<
				_("You found a bug!") << "\n\n" <<
				ex.what() << "\n\n" 
				"***\n" <<
				std::endl;
#		endif
	}

	OS::TimeShutdown();

	// Library cleanup.
	curl_global_cleanup();

	Config::Shutdown();

#ifdef _WIN32
	CoUninitialize();
#endif

	return lErrorCode;
}
