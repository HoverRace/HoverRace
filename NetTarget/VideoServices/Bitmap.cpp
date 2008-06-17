// Bitmap.cpp
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

#include "stdafx.h"

#include "Bitmap.h"





int MR_Bitmap::GetBestBitmapFor( int pXRes, int pYRes )const
{
   int lReturnValue = -1;

   pXRes = pXRes*2;
   pYRes = pYRes*2;

   int lCount = GetNbSubBitmap();

   for( int lCounter = 0; lCounter < lCount; lCounter++ )
   {
      if( ( GetXRes( lCounter ) < pXRes )&&
          ( GetYRes( lCounter ) < pYRes ) )
      {
         lReturnValue = lCounter;
         break;
      }
   }

   return lReturnValue;
}

int MR_Bitmap::GetBestBitmapForYRes( int pYRes )const
{
   int lReturnValue = -1;

   pYRes = pYRes;

   int lCount = GetNbSubBitmap();

   for( int lCounter = 0; lCounter < lCount; lCounter++ )
   {
      if( GetYRes( lCounter ) <= pYRes )
      {
         lReturnValue = lCounter;
         break;
      }
   }
   return lReturnValue;

}

int MR_Bitmap::GetBestBitmapForXRes( int pXRes )const
{
   int lReturnValue = -1;

   pXRes = pXRes;

   int lCount = GetNbSubBitmap();

   for( int lCounter = 0; lCounter < lCount; lCounter++ )
   {
      if( GetXRes( lCounter ) <= pXRes )
      {
         lReturnValue = lCounter;
         break;
      }
   }
   return lReturnValue;

}

int MR_Bitmap::GetBestBitmapForPitch_4096( int pPitch_4096 )const
{
   int lReturnValue = -1;


   int lCount = GetNbSubBitmap();

   for( int lCounter = 0; lCounter < lCount; lCounter++ )
   {
      if( pPitch_4096 <= (2*4096)  )
      {
         lReturnValue = lCounter;
         break;
      }
      pPitch_4096 /=2;
   }
   return lReturnValue;

}

