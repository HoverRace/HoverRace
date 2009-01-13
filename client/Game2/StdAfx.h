
/* StdAfx.h
	Precompiled header for client. */

#ifdef _WIN32

#	define VC_EXTRALEAN							  // Exclude rarely-used stuff from Windows headers

	// Minimum Windows version: XP
#	define WINVER 0x0501

#	include <afxwin.h>								// MFC core and standard components
#	include <afxext.h>								// MFC extensions
#	include <afxtempl.h>

#	include <typeinfo.h>

#	ifndef _AFX_NO_AFXCMN_SUPPORT
#		include <afxcmn.h>							// MFC support for Windows 95 Common Controls
#	endif											// _AFX_NO_AFXCMN_SUPPORT

#	include "../../include/config-win32.h"

#else

#	include "../../include/compat/unix.h"
#	include "../../config.h"

#	include <strings.h>

#endif

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <exception>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>

#include <libintl.h>

#ifdef _
#	undef _
#endif
#ifdef WITH_GETTEXT
#	define _(x) gettext(x)
#else
#	define _(x) (x)
#	define gettext(x) (x)
#	define dgettext(d,x) (x)
#	define dcgettext(d,x,c) (x)
#endif
