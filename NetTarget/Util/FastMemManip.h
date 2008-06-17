// FastMemManip.h
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

inline void MR_Mem32Set( MR_UInt32* pDest, MR_UInt32 pValue, int pCount )
{
   for( int lCounter = 0; lCounter < pCount; lCounter++ )
   {
      pDest[ lCounter ] = pValue;
   }
}

inline void MR_SmallMem16Set( MR_UInt16* pDest, MR_UInt16 pValue, int pCount )
{
   for( int lCounter = 0; lCounter < pCount; lCounter++ )
   {
      pDest[ lCounter ] = pValue;
   }
}

inline void MR_LargeMem16Set( MR_UInt16* pDest, MR_UInt16 pValue, int pCount )
{
   if( pCount > 0 )
   {
      MR_UInt32  lValue = pValue|( pValue<<16);
      MR_UInt32* lDest  = (MR_UInt32*)(((int)pDest)&~1);


      pDest[ pCount-1 ] = pValue;

      if( (int)pDest&1 )
      {
         *pDest = pValue; 
         pCount--;
      }


      for( int lCounter = 0; lCounter < pCount/2; lCounter++ )
      {
         lDest[ lCounter ] = lValue;
      }
   }
}
