
/* Common OS-specific headers. */

#pragma once

#ifdef _WIN32

	// Exclude rarely-used stuff from Windows headers
#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX

	// Minimum Windows version: 7
#	define WINVER _WIN32_WINNT_WIN7

#	define _CRT_SECURE_NO_DEPRECATE
#	define _SCL_SECURE_NO_DEPRECATE

#	define _USE_MATH_DEFINES

#	include <windows.h>
#	include <typeinfo.h>

#	pragma warning(disable: 4251)
#	pragma warning(disable: 4275)
#	pragma warning(disable: 4458)

	// Common warnings from system / library headers.
#	pragma warning(disable: 4503)  // decorated name length exceeded, name was truncated
#	pragma warning(disable: 4702)  // unreachable code
#	pragma warning(disable: 4714)  // function marked as __forceinline not inlined

	// Bugged warning introduced in VS2015 Update 1.
	// http://stackoverflow.com/a/34027257
#	pragma warning(disable: 4592)  // symbol will be dynamically initialized (implementation limitation)

#	include <assert.h>
#	define ASSERT(e) assert(e)
#	define TRACE __noop

#	include "config-win32.h"
#	include <hoverrace/hr-config.h>

#else

#	include "../compat/unix.h"
#	include <hoverrace/hr-config.h>

#	include <string.h>
#	include <strings.h>

#endif
