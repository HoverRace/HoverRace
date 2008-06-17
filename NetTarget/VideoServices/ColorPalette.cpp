// ColorPalette.cpp
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

#include <math.h>
#include "ColorPalette.h"


// Local functions

// Functions implementation

PALETTEENTRY* MR_GetColors( double pGamma, double pIntensity, double pIntensityBase )
{
   PALETTEENTRY* lReturnValue = new PALETTEENTRY[MR_BASIC_COLORS];

   int lColorIndex = 0;

   for( ; lColorIndex < MR_BasicPaletteSize; lColorIndex++ )
   {
      double lRed;
      double lGreen;
      double lBlue;

      // Compute the gamma correction
      lRed   = pIntensityBase + pIntensity*pow( MR_BasicPalette[ lColorIndex ][0],  pGamma );
      lGreen = pIntensityBase + pIntensity*pow( MR_BasicPalette[ lColorIndex ][1],  pGamma );
      lBlue  = pIntensityBase + pIntensity*pow( MR_BasicPalette[ lColorIndex ][2],  pGamma );

      // Return in the int domain
      lRed   *= 256;
      lGreen *= 256;
      lBlue  *= 256;

      if( lRed >= 256 )
      {
         lRed = 255;
      }

      if( lGreen >= 256 )
      {
         lGreen = 255;
      }

      if( lBlue >= 256 )
      {
         lBlue = 255;
      }

      lReturnValue[ lColorIndex ].peRed   = (MR_UInt8)lRed;
      lReturnValue[ lColorIndex ].peGreen = (MR_UInt8)lGreen;
      lReturnValue[ lColorIndex ].peBlue  = (MR_UInt8)lBlue;
   
      lReturnValue[ lColorIndex ].peFlags = 0; //PC_EXPLICIT;

   }

   for( ; lColorIndex < MR_BASIC_COLORS; lColorIndex++ )
   {

      lReturnValue[ lColorIndex ].peRed   = 255;
      lReturnValue[ lColorIndex ].peGreen = 255;
      lReturnValue[ lColorIndex ].peBlue  = lColorIndex-15;
   
      lReturnValue[ lColorIndex ].peFlags = 0; //PC_EXPLICIT;

      lColorIndex++;
   }

   return lReturnValue;
}

const PALETTEENTRY& MR_ConvertColor( MR_UInt8 pRed, MR_UInt8 pGreen, MR_UInt8 pBlue,
                                     double pGamma, double pIntensity, double pIntensityBase )
{
   static PALETTEENTRY lReturnValue;

   double lRed;
   double lGreen;
   double lBlue;

   // Compute the gamma correction
   lRed   = pIntensityBase + pIntensity*pow( pRed/256.0,   pGamma );
   lGreen = pIntensityBase + pIntensity*pow( pGreen/256.0, pGamma );
   lBlue  = pIntensityBase + pIntensity*pow( pBlue/256.0,  pGamma );

   lRed   *= 256;
   lGreen *= 256;
   lBlue  *= 256;

   if( lRed >= 256 )
   {
      lRed = 255;
   }

   if( lGreen >= 256 )
   {
      lGreen = 255;
   }

   if( lBlue >= 256 )
   {
      lBlue = 255;
   }


   lReturnValue.peRed   = (MR_UInt8)lRed;
   lReturnValue.peGreen = (MR_UInt8)lGreen;
   lReturnValue.peBlue  = (MR_UInt8)lBlue;

   lReturnValue.peFlags = 0; //PC_EXPLICIT;

   return lReturnValue;
}
