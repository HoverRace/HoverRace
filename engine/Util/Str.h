
// Str.h
// Header for string support functions.
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

#pragma once

#include "OS.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Util {

namespace Str {

	MR_DllDeclare wchar_t *Utf8ToWide(const char *s);
	MR_DllDeclare char *WideToUtf8(const wchar_t *ws);

	/** Utility class for easy conversion of UTF-8 to wide strings. */
	class MR_DllDeclare UW
	{
		wchar_t *cs;
		public:
			UW(const char *s=NULL) throw() : cs(Utf8ToWide(s)) { }
			~UW() throw() { OS::Free(cs); }
			operator const wchar_t*() const throw() { return cs; }
			operator const std::wstring() const { return cs; }
	};

	/** Utility class for easy conversion of wide strings to UTF-8. */
	class MR_DllDeclare WU
	{
		char *cs;
		public:
			WU(const wchar_t *ws=NULL) throw() : cs(WideToUtf8(ws)) { }
			~WU() throw() { OS::Free(cs); }
			operator const char*() const throw() { return cs; }
			operator const std::string() const { return cs; }
	};

#	ifdef WITH_WIDE_PATHS
		typedef UW UP;
		typedef WU PU;
#	else
		MR_DllDeclare const char *UP(const char *s) { return s; }
		MR_DllDeclare const wchar_t *PU(const wchar_t *s) { return s; }
#	endif

}  // namespace Str

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
