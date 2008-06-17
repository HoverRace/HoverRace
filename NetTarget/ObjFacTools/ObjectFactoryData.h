// ObjsFactoryData
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


#ifndef MR_OBJ_FAC_DATA_H
#define MR_OBJ_FAC_DATA_H

#include "ResourceLib.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif



class MR_ObjectFactoryData
{
   public:
      HMODULE         mModule;      // Current module id
      MR_ResourceLib  mResourceLib; // resource lib

      MR_DllDeclare  MR_ObjectFactoryData( HMODULE pModule, const char* pResourceFile );
      MR_DllDeclare ~MR_ObjectFactoryData();
};


extern MR_ObjectFactoryData* gObjectFactoryData;   // Must be implemented in each module


#undef MR_DllDeclare

#endif



