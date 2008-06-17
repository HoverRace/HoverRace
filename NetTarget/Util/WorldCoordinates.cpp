// WorldCoordinates.cpp
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

// System includes
#include <math.h>

// Project Includes
#include "WorldCoordinates.h"



// Data intanciation
MR_Int16 MR_Sin[ MR_2PI ];
MR_Int16 MR_Cos[ MR_2PI ];



void MR_InitTrigoTables()
{
   double lThetaStep = 4.0*acos((double) 0) / MR_2PI;
   double lTrigoFract = MR_TRIGO_FRACT;
      
   for( int lCounter = 0; lCounter<MR_2PI; lCounter++ )
   {
      MR_Sin[ lCounter ] = MR_Int16(sin( lCounter*lThetaStep )*lTrigoFract);
      MR_Cos[ lCounter ] = MR_Int16(cos( lCounter*lThetaStep )*lTrigoFract);
   }  

}


void MR_2DCoordinate::Serialize( CArchive& pArchive )
{
   
   if( pArchive.IsStoring() )
   {
      pArchive << mX
               << mY;
   }
   else
   {
      pArchive >> mX
               >> mY;
   }
}

BOOL MR_2DCoordinate::operator ==( const MR_2DCoordinate& pCoordinate )const
{
   return (mX == pCoordinate.mX)&&(mY == pCoordinate.mY);
}

BOOL MR_2DCoordinate::operator !=( const MR_2DCoordinate& pCoordinate )const
{
   return (mX != pCoordinate.mX)||(mY != pCoordinate.mY);
}

void MR_3DCoordinate::Serialize( CArchive& pArchive )
{
   MR_2DCoordinate::Serialize( pArchive );

   if( pArchive.IsStoring() )
   {
      pArchive << mZ;
   }
   else
   {
      pArchive >> mZ;
   }
}