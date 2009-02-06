
/* Str.h
	Header for Str namespace.
*/

#pragma once

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

#include "OS.h"

namespace HoverRace {
namespace Util {

namespace Str {

	MR_DllDeclare wchar_t *Utf8ToWide(const char *s);

	/** Utility class for easy conversion of UTF-8 to wide strings. */
	class MR_DllDeclare UW
	{
		wchar_t *cs;
		public:
			UW(const char *s=NULL) throw() : cs(Utf8ToWide(s)) { }
			~UW() throw() { OS::Free(cs); }
			operator const wchar_t*() const throw() { return cs; }
	};

}  // namespace Str

}  // namespace Util
}  // namespace HoverRace
