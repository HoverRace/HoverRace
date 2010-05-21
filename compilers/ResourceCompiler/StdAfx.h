
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

#endif

#include <math.h>
#include <stdio.h>

#include <map>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
