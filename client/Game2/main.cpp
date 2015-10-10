
// main.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2013-2015 Michael Imamura.
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

#include <iostream>

#include <curl/curl.h>

#ifdef _WIN32
#	include <shellapi.h>
#endif

#include "../../engine/Exception.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Locale.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"
#include "ClientApp.h"

#include <hoverrace/hr-version.h>

using boost::format;
using boost::str;

using HoverRace::Client::ClientApp;
using namespace HoverRace::Util;

namespace {

std::vector<OS::path_t> initScripts;
OS::path_t sysCfgPath;
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
std::string reqLocale;

/**
 * Display a simple error message to the user.
 *
 * On Windows, this will pop up a message box, elsewhere this will just
 * output to the console.
 *
 * @param s The message (ASCII-only, no Unicode).
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
	int i = 0;

#	ifdef _WIN32
		int wargc;
		wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
		ASSERT(argc == wargc);
#	endif

	// Pull the next argument as a path.
	// On Windows we get the wide string version of the argument to handle
	// Unicode in paths.
	const auto argPath = [&]() -> OS::path_t {
#		ifdef _WIN32
			return wargv[i++];
#		else
			return argv[i++];
#		endif
	};

	for (i = 1; i < argc; ) {
		const char *arg = argv[i++];

		if (strcmp("-D", arg) == 0) {
			debugMode = true;
		}
		else if (strcmp("--exec", arg) == 0) {
			if (i < argc) {
				initScripts.push_back(argPath());
			}
			else {
				ShowMessage("Expected: --exec (script filename)");
				return false;
			}
		}
		else if (strcmp("-L", arg) == 0) {
			if (i < argc) {
				reqLocale = argv[i++];
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
				mediaPath = argPath();
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
		else if (strcmp("--sys-cfg-path", arg) == 0) {
			if (i < argc) {
				sysCfgPath = argPath();
			}
			else {
				ShowMessage("Expected: --sys-cfg-path (path to system "
					"config files)");
				return false;
			}
		}
		else if (strcmp("-v", arg) == 0 || strcmp("--verbose", arg) == 0) {
			verboseLog = true;
		}
		else if (strcmp("-V", arg) == 0 || strcmp("--version", arg) == 0) {
			showVersion = true;
		}
		else {
			std::ostringstream oss;
			oss << "Unknown option: " << arg;
			ShowMessage(oss.str());
			return false;
		}
	}

#	ifdef _WIN32
		LocalFree(wargv);
#	endif

	return true;
}

// Initialize (but not load) the config system.
Config *InitConfig()
{
	return Config::Init(HR_APP_VERSION, HR_APP_VERSION_PRERELEASE,
		mediaPath, sysCfgPath);
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
	while (ClientApp().MainLoop() == ClientApp::ExitMode::SOFT_RESTART) {
		HR_LOG(info) << "Soft restart commenced.";
	}
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
	std::ios::sync_with_stdio(false);

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

	Config *cfg = InitConfig();
	cfg->runtime.silent = silentMode;
	cfg->runtime.verboseLog = verboseLog;
	cfg->runtime.showFramerate = showFramerate;
	cfg->runtime.noAccel = noAccel;
	cfg->runtime.skipStartupWarning = skipStartupWarning;
	cfg->runtime.initScripts = initScripts;

	Log::Init();
	Log::Info("INFO level logging enabled.");
	Log::Debug("DEBUG level logging enabled.");

	Locale(cfg->app.localePath, PACKAGE).RequestLocale(reqLocale);

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
