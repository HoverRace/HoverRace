// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// #define VC_EXTRALEAN         // Exclude rarely-used stuff from Windows headers

// Minimum Windows version: XP
#define WINVER 0x0501

#include <afxwin.h>								  // MFC core and standard components
#include <afxext.h>								  // MFC extensions
#include <afxtempl.h>

#include <typeinfo.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>								  // MFC support for Windows 95 Common Controls
#endif											  // _AFX_NO_AFXCMN_SUPPORT

#include <algorithm>
#include <exception>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <steam/steam_api.h>
#include <steam/isteamuser.h>
#include <steam/isteamuserstats.h>
#include <steam/steamencryptedappticket.h>
#include <steam/isteamnetworking.h>