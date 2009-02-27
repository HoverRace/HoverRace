
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

#include "Str.h"

using namespace HoverRace::Util;

/**
 * Convert a UTF-8 string to a wide string.
 * The caller must use OS::Free() on the result.
 * @param s The UTF-8-encoded string (may not be NULL).
 * @return A wide string (must be freed by the caller) (never NULL).
 */
wchar_t *Str::Utf8ToWide(const char *s)
{
	ASSERT(s != NULL);

#	ifdef _WIN32
		size_t sz = strlen(s) + 1;
		wchar_t *retv = (wchar_t*)malloc(sz * sizeof(wchar_t));
		if (sz == 1) {
			*retv = L'\0';
			return retv;
		}
		
		int ct = MultiByteToWideChar(CP_UTF8, 0, s, -1, retv, sz);
		while (ct == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				sz *= 2;
				retv = (wchar_t*)realloc(retv, sz * sizeof(wchar_t));
				ct = MultiByteToWideChar(CP_UTF8, 0, s, -1, retv, sz);
			}
			else {
				ASSERT(FALSE);
				*retv = L'\0';
				break;
			}
		}

		return retv;
#	else
		//TODO: Use iconv.
		throw std::exception();
#	endif
}

