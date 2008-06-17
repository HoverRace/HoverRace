// ColorTools.h
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

#ifndef MR_COLOR_TOOLS_H
#define MR_COLOR_TOOLS_H

#include "../Util/MR_Types.h"

#ifdef MR_COLOR_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

namespace MR_ColorTools
{

   MR_DllDeclare void      Init();
   MR_DllDeclare MR_UInt8  GetNearest( double pRed, double pGreen, double pBlue );
   MR_DllDeclare void      GetComponents( MR_UInt8 pColor, double& pRed, double& pGreen, double& pBlue );
   MR_DllDeclare void      GetIntComponents( MR_UInt8 pColor, int& pRed, int& pGreen, int& pBlue );
   MR_DllDeclare int       GetNbColors();

};



#undef MR_DllDeclare

#endif



