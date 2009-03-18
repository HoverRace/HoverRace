/* StdAfx.h
	Precompiled header for mazecompiler. */

#ifdef _WIN32

#	define VC_EXTRALEAN							// Exclude rarely-used stuff from Windows headers

	// Minimum Windows version: XP
#	define WINVER 0x0501

#	include <afxwin.h>							// MFC core and standard components
#	include <afxext.h>							// MFC extensions
#	include <afxtempl.h>

#	include <typeinfo.h>

#	ifndef _AFX_NO_AFXCMN_SUPPORT
#		include <afxcmn.h>						// MFC support for Windows 95 Common Controls
#	endif										// _AFX_NO_AFXCMN_SUPPORT

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

#include <math.h>
#include <stdio.h>
