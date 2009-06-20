
// OS.cpp
// Operating system support utilities.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <locale.h>
#include <stdlib.h>

#ifndef _WIN32
#	include <sys/time.h>
#	include <time.h>
#endif

#include <iostream>
#include <string>

#ifdef _WIN32
#	include <ddraw.h>
#	include <mmsystem.h>
#endif

#include <boost/format.hpp>

#include "Str.h"

#include "OS.h"

using boost::format;
using boost::str;

using namespace HoverRace::Util;

/// Lookup table for converting from hex.
int OS::nibbles[256] = {
	 0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 00
	 0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 10
	 0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 20
	 0,  1,  2,  3,  4,  5,  6,  7,   8,  9,  0,  0,  0,  0,  0,  0, // 30
	 0, 10, 11, 12, 13, 14, 15,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 40
	 0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 50
	 0, 10, 11, 12, 13, 14, 15,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 60
	 0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0, // 70
	};

/**
 * Global reference to the current locale.
 * Defaults to "C" until OS::SetLocale() is called.
 */
std::locale OS::locale("C");

/// The standard "C" locale for things that should be not be affected by locale.
const std::locale OS::stdLocale("C");

static inline bool isHex(const char &c)
{
	return 
		(c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f');
}

/**
 * Parse the resolution from a string.
 * @param s The string (e.g. "1024x768").
 */
OS::Resolution::Resolution(const std::string &s)
{
	const char *cs = s.c_str();
	w = atoi(cs);
	size_t div = s.find('x');
	h = (div == std::string::npos) ? 0 : atoi(cs + div + 1);
}

/**
 * Render the resolution as a string.
 * @return The string (ASCII).
 */
std::string OS::Resolution::AsString() const
{
	return str(format("%dx%d", stdLocale) % w % h);
}

/**
 * Set an environment variable.
 * @param key The variable name (may not be NULL).
 * @param val The variable value (may not be NULL).
 */
void OS::SetEnv(const char *key, const char *val)
{
	ASSERT(key != NULL);
	ASSERT(val != NULL);

	std::string combined(key);
	combined += '=';
	combined += val;

#	ifdef _WIN32
		// Windows has several environments; we need to update them all.
		_putenv(combined.c_str());
		SetEnvironmentVariable(key, val);

		// MSVC6-linked libraries have a separate environment.
		typedef int (_cdecl *putenv_t)(const char *);
		static putenv_t privPutEnv = NULL;
		if (privPutEnv == NULL) {
			HMODULE hmod = GetModuleHandle("msvcrt");
			if (hmod == NULL) return;
			privPutEnv = (putenv_t)GetProcAddress(hmod, "_putenv");
			if (privPutEnv == NULL) return;
		}
		privPutEnv(combined.c_str());
#	else
#		ifdef HAVE_SETENV
			setenv(key, val, 1);
#		else
			putenv(strdup(combined.c_str()));
#		endif
#	endif
}

#ifdef _WIN32
static const char *locLang = NULL;
static const char *locRegion = NULL;
//static const char *locCharset = NULL;
//static const char *locModifier = NULL;
static bool locMatched = false;

// Callback for EnumSystemLocales(), used by OS::SetLocale().
// See EnumLocalesProc in MSDN.
static BOOL CALLBACK SetLocaleProc(LPTSTR locale)
{
	char *s;
	LCID lcid = strtoul(locale, &s, 16);
	if (*s != '\0') return TRUE;

	// Check for matching language name.
	char curLang[16];
	if (GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, curLang, 16) == 0) return TRUE;
	if (strcmp(curLang, locLang) != 0) return TRUE;

	// Check for matching region.
	if (locRegion == NULL) {
		if (SUBLANGID(LANGIDFROMLCID(lcid)) != SUBLANG_DEFAULT) return TRUE;
	}
	else {
		char curRegion[16];
		if (GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, curRegion, 16) == 0) return TRUE;
		if (strcmp(curRegion, locRegion) != 0) return TRUE;
	}

	// If we made it this far, then we have a match!
	// Note: This does nothing on Vista and later.
	if (SetThreadLocale(lcid) == 0) {
		// Found a matching LCID but couldn't set it.
		/*
		DWORD err = GetLastError();
		LPVOID errMsg;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errMsg,
			0, NULL );
		MessageBox(NULL, (const char*)errMsg, "AIEEE", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		LocalFree(errMsg);
		*/
		ASSERT(FALSE);
	}

	// Now set the global locale using the Win32-formatted name.
	char fullLang[64];
	if (GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, fullLang, 64) == 0) return TRUE;
	std::string fullLocale = fullLang;
	char fullRegion[64];
	if (GetLocaleInfo(lcid, LOCALE_SENGCOUNTRY, fullRegion, 64) != 0) {
		fullLocale.append("_").append(fullRegion);
	}
	OutputDebugString("Full locale name: ");
	OutputDebugString(fullLocale.c_str());
	OutputDebugString("\n");
	try {
		OS::locale = std::locale(fullLocale.c_str());
	}
	catch (std::runtime_error&) {
		ASSERT(FALSE);
		return TRUE;
	}

	locMatched = true;
	return FALSE;
}
#endif

/**
 * Set the locale based on the current environment.
 */
void OS::SetLocale()
{
	// Common setting.
	setlocale(LC_ALL, "");

#	ifdef _WIN32
		// For Win32, each thread has its own locale setting.
		// We need to map the ISO-abbreviated locale name from the env
		// to a Win32 LCID by enumerating all of the supported locales
		// and finding the one that matches.
		
		char *lang = getenv("LC_ALL");
		if (lang == NULL || *lang == '\0') {
			lang = getenv("LC_MESSAGES");
			if (lang == NULL || *lang == '\0') {
				lang = getenv("LANG");
				if (lang == NULL || *lang == '\0') {
					SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
					return;
				}
			}
		}
		lang = strdup(lang);

		if (strcmp(lang, "C") == 0) {
			// The "C" locale on Win32 roughly means to use system default.
			SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
		}
		else {
			// POSIX locale string format:
			//   lang[_region][.charset][@modifier]
			// lang corresponds to the LOCALE_SISO639LANGNAME and
			// region corresponds to the LOCALE_SISO3166CTRYNAME.
			// We currently ignore the charset and modifier.
			locLang = lang;
			locRegion = NULL;
			//locCharset = NULL;
			//locModifier = NULL;
			for (char *s = lang; *s != '\0'; ++s) {
				switch (*s) {
					case '_':
						*s = '\0';
						locRegion = s + 1;
						break;
					case '.':
						*s = '\0';
						//locCharset = s + 1;
						break;
					case '@':
						*s = '\0';
						//locModifier = s + 1;
						break;
				}
			}

			locMatched = false;
			EnumSystemLocales(SetLocaleProc, LCID_SUPPORTED);
			if (!locMatched) {
				//TODO: Log that we didn't find any matching language.
				ASSERT(FALSE);
				SetThreadLocale(LOCALE_SYSTEM_DEFAULT);
				locale = std::locale("C");
			}
		}

		free(lang);
#	else
		try {
			locale = std::locale("");
		}
		catch (std::runtime_error&) {
			std::cerr << "Unsupported locale (falling back to default)." <<
				std::endl;
			locale = std::locale("C");
		}
#	endif

	// Update the current locale instance.
	std::locale::global(locale);
}

#ifdef _WIN32
typedef std::map<std::string, GUID> monGuids_t;

// Callback for DirectDrawEnumerateEx(), used by OS::GetMonitors().
static BOOL CALLBACK GetMonitorsProc(GUID *guid, LPSTR /*desc*/,
                                     LPSTR name, LPVOID ctx, HMONITOR /*mon*/)
{
	// If guid is NULL, then it's the entry for "default".
	if (guid != NULL && name != NULL) {
		((monGuids_t*)ctx)->insert(monGuids_t::value_type(name, *guid));
	}
	return TRUE;
}
#endif

// Predicate to check the "primary" field.
namespace {
	struct isNotPrimary
	{
		bool operator() (const OS::Monitor &mon) { return !mon.primary; }
	};
}

/**
 * Retrieve the list of monitors.
 * @return A shared pointer of monitor info (never @c NULL, never empty).
 */
boost::shared_ptr<OS::monitors_t> OS::GetMonitors()
{
	boost::shared_ptr<monitors_t> retv(new monitors_t());
#	ifdef _WIN32
		// Use DirectDraw enumeration to map device names to GUIDs.
		monGuids_t monGuids;
		DirectDrawEnumerateEx(GetMonitorsProc, (void*)&monGuids,
			DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES);

		bool foundPrimary = false;
		for (int i = 0; ; ++i) {
			DISPLAY_DEVICE devInfo;
			memset(&devInfo, 0, sizeof(devInfo));
			devInfo.cb = sizeof(devInfo);
			if (!EnumDisplayDevices(NULL, i, &devInfo, 0)) break;

			// Ignore mirroring pseudo-devices.
			if (devInfo.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

			const GUID *guid;
			if (!monGuids.empty()) {
				// Add the device if it was included in the DirectDraw
				// enumeration.
				monGuids_t::iterator monEnt = monGuids.find(devInfo.DeviceName);
				if (monEnt == monGuids.end()) continue;
				guid = &(monEnt->second);
			}
			else {
				// If the DirectDraw enumeration only enumerated a single NULL
				// device (or no device at all), then assume there's only one
				// monitor.
				guid = NULL;
			}
			
			DISPLAY_DEVICE monInfo;
			memset(&monInfo, 0, sizeof(monInfo));
			monInfo.cb = sizeof(monInfo);
			if (EnumDisplayDevices(devInfo.DeviceName, 0, &monInfo, 0)) {
				retv->push_back(Monitor());
				Monitor &monitor = retv->back();
				monitor.primary = (devInfo.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) > 0;
				foundPrimary = foundPrimary || monitor.primary;
				monitor.name = str(format("%d. %s") % (i + 1) % monInfo.DeviceString);
				monitor.id = (guid == NULL) ?
					"{00000000-0000-0000-0000-000000000000}" :
					GuidToString(*guid);
				/*
				OutputDebugString(monitor.id.c_str());
				OutputDebugString("\n");
				*/
				
				// Retrieve the supported resolutions.
				for (int j = 0; ; ++j) {
					DEVMODE modeInfo;
					memset(&modeInfo, 0, sizeof(modeInfo));
					modeInfo.dmSize = sizeof(modeInfo);
					if (!EnumDisplaySettings(devInfo.DeviceName, j, &modeInfo)) break;
					monitor.resolutions.insert(Resolution(modeInfo.dmPelsWidth, modeInfo.dmPelsHeight));
				}

				// In the unlikely event that there were no resolutions
				// to enumerate, fill in some defaults.
				if (monitor.resolutions.empty()) {
					monitor.resolutions.insert(Resolution(640, 480));
					monitor.resolutions.insert(Resolution(800, 600));
					monitor.resolutions.insert(Resolution(1024, 768));
				}
			}
		}

		// No monitors found?  Fill out a default list and hope for the best!
		if (retv->empty()) {
			retv->push_back(Monitor());
			Monitor &monitor = retv->back();
			monitor.primary = true;
			monitor.name = "1. Plug and Play Monitor";
			monitor.id = "{00000000-0000-0000-0000-000000000000}";
			monitor.resolutions.insert(Resolution(640, 480));
			monitor.resolutions.insert(Resolution(800, 600));
			monitor.resolutions.insert(Resolution(1024, 768));
		}
		else {
			// No monitor was marked as primary?  Mark the first one.
			if (!foundPrimary) {
				retv->front().primary = true;
			}

			// If the DirectDraw enumeration only enumerated a single NULL
			// device (or no device at all), then remove all monitors other
			// than the primary (since we won't be able to pass a GUID to
			// DirectDraw for the others).
			if (monGuids.empty() && retv->size() > 1) {
				retv->erase(
					std::remove_if(retv->begin(), retv->end(), isNotPrimary()),
					retv->end());
			}
		}

		return retv;
#	else
		//TODO
		throw std::exception();
#	endif
}

#ifdef _WIN32
std::string OS::GuidToString(const GUID &guid)
{
	return str(format("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", stdLocale) %
		guid.Data1 % guid.Data2 % guid.Data3 %
		(int)guid.Data4[0] % (int)guid.Data4[1] %
		(int)guid.Data4[2] % (int)guid.Data4[3] % (int)guid.Data4[4] % (int)guid.Data4[5] % (int)guid.Data4[6] % (int)guid.Data4[7]);
}

//           1         2         3
// 01234567890123456789012345678901234567
// {3F2504E0-4F89-11D3-9A0C-0305E82C3301}

void OS::StringToGuid(const std::string &s, GUID &guid)
{
	memset(&guid, 0, sizeof(guid));
	if (s.length() < 38) return;
	const char *w = s.c_str();

	// Validation.
	if (*w++ != '{') return;
	for (int i = 1;  i <= 8;  ++i) if (!isHex(*w++)) return;
	if (*w++ != '-') return;
	for (int i = 10; i <= 13; ++i) if (!isHex(*w++)) return;
	if (*w++ != '-') return;
	for (int i = 15; i <= 18; ++i) if (!isHex(*w++)) return;
	if (*w++ != '-') return;
	for (int i = 20; i <= 23; ++i) if (!isHex(*w++)) return;
	if (*w++ != '-') return;
	for (int i = 25; i <= 36; ++i) if (!isHex(*w++)) return;
	if (*w++ != '}') return;

	w = s.c_str();
	
	guid.Data1 =
		(nibbles[w[ 1]] << 28) + (nibbles[w[ 2]] << 24) +
		(nibbles[w[ 3]] << 20) + (nibbles[w[ 4]] << 16) +
		(nibbles[w[ 5]] << 12) + (nibbles[w[ 6]] <<  8) +
		(nibbles[w[ 7]] <<  4) +  nibbles[w[ 8]];
	guid.Data2 =
		(nibbles[w[10]] << 12) + (nibbles[w[11]] <<  8) +
		(nibbles[w[12]] <<  4) +  nibbles[w[13]];
	guid.Data3 =
		(nibbles[w[15]] << 12) + (nibbles[w[16]] <<  8) +
		(nibbles[w[17]] <<  4) +  nibbles[w[18]];
	guid.Data4[0] = (nibbles[w[20]] << 4) + nibbles[w[21]];
	guid.Data4[1] = (nibbles[w[22]] << 4) + nibbles[w[23]];
	for (int i = 2, j = 25; i < 8; ++i, j += 2) {
		guid.Data4[i] = (nibbles[w[j]] << 4) + nibbles[w[j + 1]];
	}
}
#endif

/**
 * Initialize the OS time source.
 * On Win32, this attempts to increase the precision to 1 ms.
 */
void OS::TimeInit()
{
#	ifdef _WIN32
		timeBeginPeriod(1);
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
#	ifdef _WIN32
		return timeGetTime();
#	else
		timeval tv;
		gettimeofday(&tv, NULL);
		return (((timestamp_t)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#	endif
}

/**
 * Calculate the difference between two timestamps.
 * This properly handles wraparound in timestamps.
 * @param a The later timestamp.
 * @param b The earlier timestamp.
 * @return @p a - @p b
 */
OS::timestamp_t OS::TimeDiff(timestamp_t a, timestamp_t b)
{
	if (a >= b) {
		return a - b;
	}
	else {
#		ifdef _WIN32
			return 0xfffffffful - (b - a) + 1;
#		else
			return 0xffffffffffffffffull - (b - a) + 1;
#		endif
	}
}

/**
 * Shutdown the OS time source.
 */
void OS::TimeShutdown()
{
#	ifdef _WIN32
		timeEndPeriod(1);
#	endif
}

/**
 * Free a memory buffer created by a function from this class.
 * @param buf The buffer to free (may be NULL).
 */
void OS::Free(void *buf)
{
	free(buf);
}

/**
 * Open a URL.
 * @param url The URL (UTF-8 encoded).
 * @return @c true if successful, @c false otherwise.
 */
bool OS::OpenLink(const std::string &url)
{
	//FIXME: Need to make sure we're only handling "http:", "https:", and "ftp:".
#	ifdef _WIN32
		Str::UW urlw(url.c_str());
		OutputDebugStringW(L"Opening URL: ");
		OutputDebugStringW(urlw);
		OutputDebugStringW(L"\n");
		return (int)ShellExecuteW(NULL, L"open", urlw, NULL, NULL, SW_SHOWNORMAL) > 32;
#	endif
}
