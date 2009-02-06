
/* OS.cpp
	Platform-specific utilities for HoverRace.
*/

#include "StdAfx.h"

#include <locale.h>
#include <stdlib.h>

#include <string>

#include "OS.h"

using namespace HoverRace::Util;

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
			}
		}

		free(lang);
#	endif
}

/**
 * Free a memory buffer created by a function from this class.
 * @param buf The buffer to free (may be NULL).
 */
void OS::Free(void *buf)
{
	if (buf != NULL) free(buf);
}
