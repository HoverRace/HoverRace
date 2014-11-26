/* StdAfx.h
	Precompiled header for updater. */

#ifdef _WIN32

	// Exclude rarely-used stuff from Windows headers
#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN

	// Minimum Windows version: XP
#	define WINVER 0x0501

	// Defined in project since some resources don't use StdAfx.h.
//#	define _CRT_SECURE_NO_DEPRECATE
//#	define _SCL_SECURE_NO_DEPRECATE

//#	include <afxwin.h>							// MFC core and standard components
//#	include <afxext.h>							// MFC extensions
//#	include <afxtempl.h>

//#	include <typeinfo.h>

//#	ifndef _AFX_NO_AFXCMN_SUPPORT
//#		include <afxcmn.h>						// MFC support for Windows 95 Common Controls
//#	endif										// _AFX_NO_AFXCMN_SUPPORT

#endif

#define BOOST_FILESYSTEM_DEPRECATED

