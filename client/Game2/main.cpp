// main.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2013 Michael Imamura.
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

#include "StdAfx.h"

#include <iostream>

#include <curl/curl.h>

#ifdef _WIN32
#	include <shellapi.h>
#endif

#include "../../engine/Exception.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"
#include "ClientApp.h"

#ifndef _WIN32
#	include "version.h"
#endif

using boost::format;
using boost::str;

using HoverRace::Client::ClientApp;
using HoverRace::Util::Config;
namespace Log = HoverRace::Util::Log;
using HoverRace::Util::OS;
namespace Str = HoverRace::Util::Str;

namespace {

OS::path_t initScript;
OS::path_t mediaPath;
bool debugMode = false;
bool safeMode = false;
bool allowMultipleInstances = false;
bool showVersion = false;

// Runtime config options.
bool silentMode = false;
bool verboseLog = false;
bool showFramerate = false;
bool noAccel = false;
bool skipStartupWarning = false;

/**
 * Display a message to the user.
 * On Windows, this will pop up a message box
 */
void ShowMessage(const std::string &s)
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
bool ProcessCmdLine(int argc, char **argv)
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
		else if (strcmp("--media-path", arg) == 0) {
			if (i < argc) {
#				ifdef _WIN32
					mediaPath = wargv[i++];
#				else
					mediaPath = argv[i++];
#				endif
			}
			else {
				ShowMessage("Expected: --media-path (path to media files)");
				return false;
			}
		}
		else if (strcmp("--no-accel", arg) == 0) {
			noAccel = true;
		}
		else if (strcmp("-s", arg) == 0) {
			safeMode = true;
		}
		else if (strcmp("--silent", arg) == 0) {
			silentMode = true;
		}
		else if (strcmp("--skip-startup-warning", arg) == 0) {
			skipStartupWarning = true;
		}
		else if (strcmp("-v", arg) == 0 || strcmp("--verbose", arg) == 0) {
			verboseLog = true;
		}
		else if (strcmp("-V", arg) == 0 || strcmp("--version", arg) == 0) {
			showVersion = true;
		}
	}

#	ifdef _WIN32
		LocalFree(wargv);
#	endif

	return true;
}

// Initialize (but not load) the config system.
Config *InitConfig(
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
	DWORD verInfoSize = GetFileVersionInfoSizeW(Str::PW(exePath), &dummyHandle);
	void *verInfo = malloc(verInfoSize);
	if (GetFileVersionInfoW(Str::PW(exePath), 0, verInfoSize, verInfo)) {
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

	if (verMajor == 0 && verMinor == 0 && verPatch == 0 && verBuild == 0) {
		MessageBoxW(NULL,
			L"Version resources missing!\r\n\r\n"
			L"Something went wrong when this snapshot was built.",
			PACKAGE_NAME_L, MB_OK | MB_ICONERROR);
		exit(1);
	}

	// Load the configuration, using the default OS-specific path.
	return Config::Init(verMajor, verMinor, verPatch, verBuild, prerelease, mediaPath);

#else
	
	//FIXME: Extract prerelease flag from ver resource at build time.
	return Config::Init(HR_APP_VERSION, HR_APP_VERSION_PRERELEASE, mediaPath);

#endif
}

#ifdef _WIN32
/**
 * Find the path of the executable itself.
 * @return The path, including the executable name.
 */
OS::path_t FindExePath()
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

int RunClient()
{
	ClientApp game;
	game.MainLoop();
	return EXIT_SUCCESS;
}

}  // anonymous namespace

// Entry point
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main(int argc, char** argv)
#endif
{
	// initialize return variables
	int lErrorCode = EXIT_SUCCESS;

	srand(static_cast<unsigned int>(time(nullptr)));

#ifdef _WIN32
	OS::path_t exePath = FindExePath();

	// Change the working directory to the app's directory.
	_wchdir(Str::PW(exePath.parent_path()));
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
	cfg->runtime.verboseLog = verboseLog;
	cfg->runtime.showFramerate = showFramerate;
	cfg->runtime.noAccel = noAccel;
	cfg->runtime.skipStartupWarning = skipStartupWarning;
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

	Log::Init();
	Log::Info("INFO level logging enabled.");
	Log::Debug("DEBUG level logging enabled.");

	try {
		lErrorCode = RunClient();
	}
	catch (HoverRace::Exception &ex) {
		lErrorCode = EXIT_FAILURE;
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

	return lErrorCode;
}
