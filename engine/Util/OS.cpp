
/* OS.cpp
	Platform-specific utilities for HoverRace.
*/

#include "StdAfx.h"

#include "stdlib.h"

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

	// Common between UNIX and Windows.
	putenv(combined.c_str());

	// Windows has several environments; we need to update them all.
#	ifdef _WIN32
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
#	endif
}
