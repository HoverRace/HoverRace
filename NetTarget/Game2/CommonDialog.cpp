// CommonDialog.cpp
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


#include "stdafx.h"
#include "CommonDialog.h"

CString GetFileName( HWND pParent, BOOL pFileMustExist )
{
   CString      lReturnValue;
   OPENFILENAME lParam;
   char         lBuffer[300];

   lBuffer[0] = 0;

   memset( &lParam, 0, sizeof( lParam ) );
   lParam.lStructSize = sizeof( lParam );
   lParam.hwndOwner   = pParent;
   lParam.lpstrFile   = lBuffer;
   lParam.nMaxFile    = sizeof( lBuffer );


   lParam.Flags       = OFN_LONGNAMES|OFN_NOCHANGEDIR;
   if( pFileMustExist )
   {
      lParam.Flags |= OFN_FILEMUSTEXIST;
   }

   if( GetOpenFileName( &lParam ) )
   {
      lReturnValue = lParam.lpstrFile;
   }
   return lReturnValue;
}
