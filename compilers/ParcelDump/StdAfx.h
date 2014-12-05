
/* StdAfx.h
	Precompiled header for ParcelDump. */

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

#	include <windows.h>
#	include <typeinfo.h>

#	pragma warning(disable: 4251)
#	pragma warning(disable: 4275)

	// Common warnings from system / library headers.
#	pragma warning(disable: 4503)  // decorated name length exceeded, name was truncated
#	pragma warning(disable: 4702)  // unreachable code
#	pragma warning(disable: 4714)  // function marked as __forceinline not inlined

#	include "../../include/config-win32.h"

#else

#	include "../../include/compat/unix.h"
#	include <hoverrace/hr-config.h>

#	include <string.h>
#	include <strings.h>

#endif

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <math.h>
#include <stdio.h>

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#	pragma warning(push, 0)
#endif

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#ifdef _WIN32
#	pragma warning(pop)
#endif

#ifndef _WIN32
	// Xlib.h must be included *after* boost/foreach.hpp as a workaround for
	// https://svn.boost.org/trac/boost/ticket/3000
#	include <X11/Xlib.h>
#endif

// Mark unused parameters so we can keep both the compiler and Doxygen happy.
template<class... T> void HR_UNUSED(T&&...) { }