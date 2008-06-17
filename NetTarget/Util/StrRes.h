// StrRes.h //  Set of functions that welp in the implementations of string-resources
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which 
// you have taken this file. If you can not find the license you can not use 
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions 
// and limitations under the License.
//

#include "MR_Types.h"

#ifndef STR_RES_H
#define STR_RES_H


#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


MR_DllDeclare CString     MR_LoadString( int pResource );
MR_DllDeclare const char* MR_LoadStringBuffered( int pResource );

#undef MR_DllDeclare

#endif
