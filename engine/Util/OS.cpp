
// OS.cpp
//
// Copyright (c) 2009, 2014 Michael Imamura.
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

#include <locale.h>
#include <stdlib.h>

#ifndef _WIN32
#	include <sys/time.h>
#	include <time.h>
#endif

#ifdef WITH_CHRONO_TIMESTAMP
#	include <chrono>
#endif
#include <iomanip>
#include <iostream>
#include <string>
#include <system_error>

#ifdef _WIN32
#	include <shellapi.h>
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include <SDL2/SDL.h>

#include "../Exception.h"
#include "Log.h"
#include "Str.h"

#include "OS.h"

namespace fs = boost::filesystem;
using boost::format;
using boost::str;

namespace HoverRace {
namespace Util {

namespace {

#ifdef WITH_CHRONO_TIMESTAMP
	using clock_t = std::chrono::high_resolution_clock;
	clock_t::time_point chronoStart;
#elif defined(_WIN32)
	LARGE_INTEGER qpcFreq = { 0 };
	LARGE_INTEGER qpcStart = { 0 };
#else
	timeval tvStart = { 0, 0 };
#endif

}  // namespace

/**
 * Global reference to the current locale.
 * Defaults to "C" until OS::SetLocale() is called.
 */
std::locale OS::locale("C");

/// The standard "C" locale for things that should be not be affected by locale.
const std::locale OS::stdLocale("C");

/**
 * Set an environment variable.
 * @param key The variable name (may not be NULL).
 * @param val The variable value (may not be NULL).
 * @throw Exception
 */
void OS::SetEnv(const char *key, const char *val)
{
	ASSERT(key != NULL);
	ASSERT(val != NULL);

#	ifdef _WIN32
		std::string combined(key);
		combined += '=';
		combined += val;

		// Windows has several environments; we need to update them all.
		if (_putenv(combined.c_str()) < 0) {
			throw Exception("_putenv failed.");
		}
		if (SetEnvironmentVariable(key, val) == 0) {
			throw Exception("SetEnvironmentVariable failed.");
		}

		// MSVC6-linked libraries have a separate environment.
		// If the MSVC6 runtime library hasn't been loaded, then we just
		// skip this step since nothing pulled it in as a dependency.
		typedef int (_cdecl *putenv_t)(const char *);
		static putenv_t privPutEnv = nullptr;
		if (!privPutEnv) {
			HMODULE hmod = GetModuleHandle("msvcrt");
			if (!hmod) return;
			privPutEnv = (putenv_t)GetProcAddress(hmod, "_putenv");
			if (!privPutEnv) return;
		}
		if (privPutEnv(combined.c_str()) < 0) {
			throw Exception("msvcrt _putenv failed.");
		}
#	else
		if (setenv(key, val, 1) < 0) {
			throw Exception("setenv failed: " + StrError(errno));
		}
#	endif
}

/**
 * Set the locale based on the current environment.
 */
void OS::SetLocale()
{
	// Common setting.
	setlocale(LC_ALL, "");

	try {
		locale = std::locale("");
	}
	catch (std::runtime_error&) {
		Log::Warn("Unsupported locale (falling back to default).");
		locale = std::locale("C");
	}

	// Update the current locale instance.
	std::locale::global(locale);
}

/**
 * Convert an error number to a string.
 * @param errnum The error number, usually @c errno.
 * @return The error as a string.
 */
std::string OS::StrError(int errnum)
{
	return std::error_code(errnum, std::generic_category()).message();
}

/**
 * Initialize the OS time source.
 * On Win32, this attempts to increase the precision to 1 ms.
 */
void OS::TimeInit()
{
#	ifdef WITH_CHRONO_TIMESTAMP
		chronoStart = clock_t::now();
#	elif defined(_WIN32)
		if (QueryPerformanceFrequency(&qpcFreq) == FALSE) {
			throw Exception("High-resolution timer not available.");
		}
		else {
			QueryPerformanceCounter(&qpcStart);
		}
#	else
		if (gettimeofday(&tvStart, nullptr) < 0) {
			throw Exception("High-resolution timer not available: " +
				StrError(errno));
		}
#	endif
}

/**
 * Retrieve a timestamp.
 * The value returned is OS-dependent and should only be used for relative
 * calculations.
 * @return A relative timestamp, in milliseconds.
 */
OS::timestamp_t OS::Time()
{
#	ifdef WITH_CHRONO_TIMESTAMP
		return static_cast<timestamp_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				clock_t::now() - chronoStart).count());
#	elif defined(_WIN32)
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);

		// Try to keep the values low, for sanity's sake when debugging.
		count.QuadPart -= qpcStart.QuadPart;

		count.QuadPart *= 1000;
		return static_cast<timestamp_t>(count.QuadPart / qpcFreq.QuadPart);
#	else
		timeval count;
		gettimeofday(&count, nullptr);

		timestamp_t retv = static_cast<timestamp_t>(
			count.tv_sec - tvStart.tv_sec) * 1000;
		retv += static_cast<timestamp_t>(
			count.tv_usec - tvStart.tv_usec) / 1000;

		return retv;
#	endif
}

/**
 * Generate a string representing the current timestamp, suitable for filenames.
 * @return The string (in "YYYY-MM-DD HH.MM.SS TZ" format).
 */
std::string OS::FileTimeString()
{
	/* std::put_time is unavailable in libstdc++ 4.8.
	auto now = std::chrono::system_clock::to_time_t(
		std::chrono::system_clock::now());

	std::ostringstream oss;
	oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H.%M.%S %z");
	return oss.str();
	*/

	tm now;
	const time_t curTime = time(nullptr);
	memcpy(&now, localtime(&curTime), sizeof(now));
	char retv[128] = "";
	if (strftime(retv, sizeof(retv), "%Y-%m-%d %H.%M.%S %z", &now) == 0) {
		std::ostringstream oss;
		oss << "Filename timestamp exceeded limit: " << sizeof(retv);
		throw Exception(oss.str());
	}
	else {
		return std::string(retv);
	}
}

/**
 * Shutdown the OS time source.
 */
void OS::TimeShutdown()
{
	// Currently unnecessary.
}

/**
 * Open a URL.
 * @param url The URL (UTF-8 encoded).
 * @return @c true if successful, @c false otherwise.
 */
bool OS::OpenLink(const std::string &url)
{
	HR_LOG(info) << "Opening URL: " << url;

	if (!(boost::starts_with(url, "http://") ||
		boost::starts_with(url, "https://") ||
		boost::starts_with(url, "ftp://")))
	{
		HR_LOG(error) << "Blocked opening link (unsupported protocol): " << url;
		return false;
	}

#	ifdef _WIN32
		Str::UW urlw(url.c_str());
		return (int)ShellExecuteW(NULL, L"open", urlw, NULL, NULL, SW_SHOWNORMAL) > 32;
#	else
		pid_t pid = fork();
		if (pid == 0) {
			execlp("xdg-open", "xdg-open", url.c_str(), nullptr);
			perror("Failed to open link with xdg-open");
			_exit(1);
		}
		else if (pid < 0) {
			std::string exs = "Failed to fork to run xdg-open: ";
			exs += StrError(errno);
			throw std::runtime_error(exs.c_str());
		}
		//TODO: Wait for child process and return success/fail.
		return true;
#	endif
}

/**
 * Open a filesystem path.
 *
 * Only directories may be opened; they will be opened using the native file
 * navigator.
 *
 * @param path The path to open.
 * @return @c true if successful, @c false otherwise.
 */
bool OS::OpenPath(const path_t &path)
{
	if (!(fs::exists(path) && fs::is_directory(path))) return false;
	Log::Info("Opening path: %s", (const char*)Str::PU(path));
#	ifdef _WIN32
		std::wstring s((const wchar_t*)Str::PW(path));
		return (int)ShellExecuteW(NULL, L"open", s.c_str(), NULL, NULL, SW_SHOWNORMAL) > 32;
#	else
		std::string s((const char *)Str::PU(path));
		pid_t pid = fork();
		if (pid == 0) {
			execlp("xdg-open", "xdg-open", s.c_str(), nullptr);
			perror("Failed to open path with xdg-open");
			_exit(1);
		}
		else if (pid < 0) {
			std::string exs = "Failed to fork to run xdg-open: ";
			exs += StrError(errno);
			throw std::runtime_error(exs.c_str());
		}
		//TODO: Wait for child process and return success/fail.
		return true;
#	endif
}

/**
 * Open a file using the OS-specific path format.
 * @param path The file to open.
 * @param mode The mode (same as POSIX {@c fopen(3)}).
 * @return The file handle or @c NULL if an error occurred.
 */
FILE *OS::FOpen(const path_t &path, const char *mode)
{
#	ifdef WITH_WIDE_PATHS
#		ifdef _WIN32
			return _wfopen((const wchar_t*)Str::PW(path), Str::UW(mode));
#		else
			// Unimplemented.
			throw UnimplementedExn("OS::FOpen for non-Win32 wide paths");
#		endif
#	else
		return fopen((const char*)Str::PU(path), mode);
#	endif
}

}  // namespace Util
}  // namespace HoverRace
