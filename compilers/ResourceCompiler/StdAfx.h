
/* StdAfx.h
	Precompiled header for resourcecompiler. */

#ifdef _WIN32

#	define VC_EXTRALEAN							// Exclude rarely-used stuff from Windows headers

	// Minimum Windows version: XP
#	define WINVER 0x0501

#	define _CRT_SECURE_NO_DEPRECATE
#	define _SCL_SECURE_NO_DEPRECATE

#	include <afxwin.h>							// MFC core and standard components
#	include <afxext.h>							// MFC extensions
#	include <afxtempl.h>

#	include <typeinfo.h>

#	ifndef _AFX_NO_AFXCMN_SUPPORT
#		include <afxcmn.h>						// MFC support for Windows 95 Common Controls
#	endif										// _AFX_NO_AFXCMN_SUPPORT

#	include "../../include/config-win32.h"

#else

#	include "../../include/compat/unix.h"
#	include "../../config.h"

#	include <string.h>
#	include <strings.h>

#endif

#include <math.h>
#include <stdio.h>

#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#ifndef _WIN32
	// Xlib.h must be included *after* boost/foreach.hpp as a workaround for
	// https://svn.boost.org/trac/boost/ticket/3000
#	include <X11/Xlib.h>
#endif
