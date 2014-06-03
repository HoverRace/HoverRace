
/* StdAfx.h
	Precompiled header for engine. */

#pragma once

#ifdef _WIN32

	// Exclude rarely-used stuff from Windows headers
#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN

// Minimum Windows version: XP
#	define WINVER 0x0501

#	define _CRT_SECURE_NO_DEPRECATE
#	define _SCL_SECURE_NO_DEPRECATE

#	define _USE_MATH_DEFINES

#	include <windows.h>
#	include <typeinfo.h>

#	pragma warning(disable: 4251)
#	pragma warning(disable: 4275)

#	include <assert.h>
#	define ASSERT(e) assert(e)

#	include "../include/config-win32.h"

#else

#	include "../include/compat/unix.h"
#	include <hoverrace/hr-config.h>

#	include <strings.h>

#endif

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Commonly-used STL headers.
#include <algorithm>
#include <array>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Prefer Boost::Filesystem v3 on Boost 1.44+.
#include <boost/version.hpp>
#if BOOST_VERSION >= 104400
#	define BOOST_FILESYSTEM_VERSION 3
#	define BOOST_FILESYSTEM_NO_DEPRECATED
#else
#	define BOOST_FILESYSTEM_VERSION 2
#endif

#include "../include/compat/luabind_cxx11.h"
#include "../include/compat/boost_cxx11.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/signals2.hpp>

#include "../../engine/Script/Lua.h"

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

// Don't use the min/max macros; use std::min and std::max from the STL.
#ifdef min
#	undef min
#endif
#ifdef max
#	undef max
#endif
using std::min;
using std::max;

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
