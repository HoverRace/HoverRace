/* StdAfx.h
	Precompiled header for mazecompiler. */

#pragma once

#ifdef _WIN32

	// Exclude rarely-used stuff from Windows headers
#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN

	// Minimum Windows version: XP
#	define WINVER _WIN32_WINNT_VISTA

#	define _CRT_SECURE_NO_DEPRECATE
#	define _SCL_SECURE_NO_DEPRECATE

#	include <windows.h>
#	include <typeinfo.h>

#	pragma warning(disable: 4251)
#	pragma warning(disable: 4275)

#	include <assert.h>
#	define ASSERT(e) assert(e)
#	define TRACE __noop

#	include "../../include/config-win32.h"

#else

#	include "../../include/compat/unix.h"
#	include <hoverrace/hr-config.h>

#	include <string.h>
#	include <strings.h>

#endif

// Prefer Boost::Filesystem v3 on Boost 1.44+.
#include <boost/version.hpp>
#if BOOST_VERSION >= 104400
#	define BOOST_FILESYSTEM_VERSION 3
#	define BOOST_FILESYSTEM_NO_DEPRECATED
#else
#	define BOOST_FILESYSTEM_VERSION 2
#endif

#include <locale.h>
#include <math.h>
#include <stdio.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#ifndef _WIN32
	// Xlib.h must be included *after* boost/foreach.hpp as a workaround for
	// https://svn.boost.org/trac/boost/ticket/3000
#	include <X11/Xlib.h>
#endif

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

// Mark unused parameters so we can keep both the compiler and Doxygen happy.
#define HR_UNUSED(x) do { (void)(x); } while (0);
