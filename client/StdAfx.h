
/* StdAfx.h
	Precompiled header for client. */

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

	// Common warnings from system / library headers.
#	pragma warning(disable: 4503)  // decorated name length exceeded, name was truncated
#	pragma warning(disable: 4702)  // unreachable code
#	pragma warning(disable: 4714)  // function marked as __forceinline not inlined

#	include <assert.h>
#	define ASSERT(e) assert(e)
#	define TRACE __noop

#	include "../include/util/config-win32.h"

#else

#	include "../include/compat/unix.h"
#	include <hoverrace/hr-config.h>

#	include <string.h>
#	include <strings.h>

#endif

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#	pragma warning(push, 0)
#endif

#include "../../include/compat/luabind_cxx11.h"
#include "../../include/compat/boost_cxx11.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/thread.hpp>

#include "../../engine/Script/Lua.h"

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#define PACKAGE_NAME_WIDEN(x) L ## x
#define PACKAGE_NAME_APPLY(x) PACKAGE_NAME_WIDEN(x)
#define PACKAGE_NAME_L PACKAGE_NAME_APPLY(PACKAGE_NAME)

#ifdef _
#	undef _
#endif
#ifdef ENABLE_NLS
#	include <libintl.h>

	// The gettext custom versions of printf (and the like) sometimes cause
	// crashes on Win32.  We use Boost Format for translated strings anyway,
	// so we don't need these.
#	ifdef _WIN32
#		undef printf
#		undef vprintf
#		undef fprintf
#		undef vfprintf
#		undef snprintf
#		undef vsnprintf
#	endif

#	define _(x) gettext(x)

	// Our own little version of pgettext() so we don't need all of gettext.h.
#	define pgettext(p,x) pgettextImpl(p "\004" x, x)
	static inline const char *pgettextImpl(const char *full, const char *msg)
	{
		const char *retv = _(full);
		return (retv == full) ? msg : retv;
	}

#else

#	define _(x) (x)
#	define gettext(x) (x)
#	define dgettext(d,x) (x)
#	define dcgettext(d,x,c) (x)
#	define pgettext(p,x) (x)
#	define pgettextImpl(f,x) (x)

#endif  // ENABLE_NLS

#define HR_WEBSITE "http://www.hoverrace.com/"

// Mark unused parameters so we can keep both the compiler and Doxygen happy.
template<class... T> void HR_UNUSED(T&&...) { }