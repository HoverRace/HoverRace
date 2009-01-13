
/* StdAfx.h
	Precompiled header for engine. */

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
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

// Don't use the min/max macros; use std::min and std::max from the STL.
#ifdef min
#	undef min
#endif
#ifdef max
#	undef max
#endif
using std::min;
using std::max;
