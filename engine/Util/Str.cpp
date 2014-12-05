
// Str.cpp
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

#include "../StdAfx.h"

#include <iomanip>

#include <utf8/utf8.h>

#include "Log.h"

#include "Str.h"

#ifdef _WIN32
#	pragma warning(disable: 4996)
#endif

namespace HoverRace {
namespace Util {

namespace {

// Helpers to pick the appropriate conversion function for the size of wchar_t.

template<size_t SZ>
struct Widen { };

template<>
struct Widen<2>
{
	template<class... Args>
	Widen(Args&&... args) { utf8::utf8to16(std::forward<Args>(args)...); }
};

template<>
struct Widen<4>
{
	template<class... Args>
	Widen(Args&&... args) { utf8::utf8to32(std::forward<Args>(args)...); }
};

template<size_t SZ>
struct Narrow { };

template<>
struct Narrow<2>
{
	template<class... Args>
	Narrow(Args&&... args) { utf8::utf16to8(std::forward<Args>(args)...); }
};

template<>
struct Narrow<4>
{
	template<class... Args>
	Narrow(Args&&... args) { utf8::utf32to8(std::forward<Args>(args)...); }
};

}  // namespace

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
		Widen<sizeof(wchar_t)>(s, s + len, std::back_inserter(ws));
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
		Narrow<sizeof(wchar_t)>(ws, ws + len, std::back_inserter(s));
	}
	catch (utf8::exception &e) {
		Log::Warn("Wide string to UTF-8 failed: %s", e.what());
		return strdup("<invalid wide string>");
	}

	return strdup(s.c_str());
}

std::ostream &Str::Lua::StreamOut(std::ostream &os) const
{
	std::ios::fmtflags flags(os.flags());

	os << std::setfill('0') << std::dec;

	// Lua strings are simple character arrays without any specific
	// support for Unicode, so our UTF-8 strings are escaped as a
	// series of bytes.

	for (auto iter = s.cbegin(), iend = s.cend(); iter != iend; ++iter) {
		auto ch = static_cast<unsigned char>(*iter);

		switch (ch) {
			case '\a': os << "\\a"; break;
			case '\b': os << "\\b"; break;
			case '\f': os << "\\f"; break;
			case '\n': os << "\\n"; break;
			case '\r': os << "\\r"; break;
			case '\t': os << "\\t"; break;
			case '\v': os << "\\v"; break;
			case '\\': os << "\\\\"; break;
			case '"': os << "\\\""; break;
			case '\'': os << "\\'"; break;
			default:
				if (ch < 32 || ch >= 127) {
					os << '\\' << std::setw(3) <<
						static_cast<unsigned int>(ch);
				}
				else {
					os << ch;
				}
		}
	}

	os.flags(flags);

	return os;
}

}  // namespace Util
}  // namespace HoverRace
