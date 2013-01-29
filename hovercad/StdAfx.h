// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN         // Exclude rarely-used stuff from Windows headers

// Minimum Windows version: XP
#define WINVER 0x0501

#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

#include <afxwin.h>								  // MFC core and standard components
#include <afxext.h>								  // MFC extensions
#include <afxtempl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>								  // MFC support for Windows 95 Common Controls
#endif											  // _AFX_NO_AFXCMN_SUPPORT

#include "../include/config-win32.h"

#define BOOST_FILESYSTEM_VERSION 2

#include <locale.h>
#include <math.h>
#include <stdio.h>

#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#ifdef _
#	undef _
#endif
#ifdef ENABLE_NLS
#	include <libintl.h>

	// The gettext custom versions of printf (and the like) sometimes cause
	// crashes on Win32.  We use Boost Format for translated strings anyway,
	// so we don't need these.
#	undef printf
#	undef vprintf
#	undef fprintf
#	undef vfprintf
#	undef snprintf
#	undef vsnprintf

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
