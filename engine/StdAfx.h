
/* StdAfx.h
	Precompiled header for engine. */

#ifdef _WIN32

#	define VC_EXTRALEAN							  // Exclude rarely-used stuff from Windows headers

// Minimum Windows version: XP
#	define WINVER 0x0501

#	define _CRT_SECURE_NO_DEPRECATE
#	define _SCL_SECURE_NO_DEPRECATE

#	include <windows.h>
#	include <typeinfo.h>

#	pragma warning(disable: 4251)
#	pragma warning(disable: 4275)

#	include <assert.h>
#	define ASSERT(e) assert(e)

#	include "../include/config-win32.h"

#else

#	include "../include/compat/unix.h"
#	include "../config.h"

#	include <strings.h>

#endif

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Commonly-used STL headers.
#include <algorithm>
#include <exception>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// For newer versions of boost where filesystem has a v3.
#define BOOST_FILESYSTEM_VERSION 2

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#ifndef _WIN32
	// Xlib.h must be included *after* boost/foreach.hpp as a workaround for
	// https://svn.boost.org/trac/boost/ticket/3000
#	include <X11/Xlib.h>
#endif

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
