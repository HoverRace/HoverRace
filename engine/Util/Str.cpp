
// Str.h
// String support functions.
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

#include <utf8/utf8.h>

#include "Log.h"

#include "Str.h"

#ifdef _WIN32
#	pragma warning(disable: 4996)
#endif

namespace HoverRace {
namespace Util {

/**
 * Convert a UTF-8 string to a wide string.
 * The caller must use OS::Free() on the result.
 * @param s The UTF-8-encoded string (may not be NULL).
 * @return A wide string (must be freed by the caller) (never NULL).
 */
wchar_t *Str::Utf8ToWide(const char *s)
{
	ASSERT(s != NULL);

	if (*s == '\0') {
		wchar_t *retv = (wchar_t*)malloc(sizeof(wchar_t));
		*retv = L'\0';
		return retv;
	}

	size_t len = strlen(s);
	std::wstring ws;
	ws.reserve(len);

	try {
		if (sizeof(wchar_t) == 2) {
			utf8::utf8to16(s, s + len, std::back_inserter(ws));
		}
		else {
			utf8::utf8to32(s, s + len, std::back_inserter(ws));
		}
	}
	catch (utf8::exception &e) {
		Log::Warn("UTF-8 to wide string failed: %s", e.what());
		return wcsdup(L"<invalid UTF-8 string>");
	}

	return wcsdup(ws.c_str());
}

/**
 * Convert a wide string to a UTF-8 string.
 * The caller must use OS::Free() on the result.
 * @param ws The wide string (may not be NULL).
 * @return A UTF-8-encoded string (must be freed by the caller) (never NULL).
 */
char *Str::WideToUtf8(const wchar_t *ws)
{
	ASSERT(ws != NULL);

	if (*ws == L'\0') {
		char *retv = (char*)malloc(1);
		*retv = '\0';
		return retv;
	}

	size_t len = wcslen(ws);
	std::string s;
	s.reserve(len * 3 + 1);  // Initial guess.

	try {
		if (sizeof(wchar_t) == 2) {
			utf8::utf16to8(ws, ws + len, std::back_inserter(s));
		}
		else {
			utf8::utf32to8(ws, ws + len, std::back_inserter(s));
		}
	}
	catch (utf8::exception &e) {
		Log::Warn("Wide string to UTF-8 failed: %s", e.what());
		return strdup("<invalid wide string>");
	}

	return strdup(s.c_str());
}

}  // namespace Util
}  // namespace HoverRace
