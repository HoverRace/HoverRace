// PhysicalCollision.cpp
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

#include "PhysicalCollision.h"

// class MR_InertialMoment

void MR_InertialMoment::ComputeCollision( const MR_InertialMoment*                   pObstacle,
                                          MR_Angle                                   pHorizontalDirection )
{

   // Compute the speed component in the direction of the Impact direction

   MR_Int32 lDirectionSpeed    = (mXSpeed*MR_Cos[ pHorizontalDirection ]/MR_TRIGO_FRACT) + (mYSpeed*MR_Sin[ pHorizontalDirection ]/MR_TRIGO_FRACT);
   MR_Int32 lObjDirectionSpeed = (pObstacle->mXSpeed*MR_Cos[ pHorizontalDirection ]/MR_TRIGO_FRACT) + (pObstacle->mYSpeed*MR_Sin[ pHorizontalDirection ]/MR_TRIGO_FRACT);

   if( lObjDirectionSpeed > lDirectionSpeed )
   {
       double lKeepRatio     = (mWeight-pObstacle->mWeight)/(mWeight+pObstacle->mWeight);
       double lReceivedRatio = (2*mWeight)*pObstacle->mWeight/((mWeight+pObstacle->mWeight)*mWeight);

       lDirectionSpeed    = MR_Int32(lDirectionSpeed*lKeepRatio);
       lObjDirectionSpeed = MR_Int32(lObjDirectionSpeed*lReceivedRatio);

       MR_Int32 lXBounceSpeed = (MR_Int32)lDirectionSpeed*MR_Cos[ pHorizontalDirection ]/MR_TRIGO_FRACT + (MR_Int32)lObjDirectionSpeed*MR_Cos[ pHorizontalDirection ]/MR_TRIGO_FRACT;
       MR_Int32 lYBounceSpeed = (MR_Int32)lDirectionSpeed*MR_Sin[ pHorizontalDirection ]/MR_TRIGO_FRACT + (MR_Int32)lObjDirectionSpeed*MR_Sin[ pHorizontalDirection ]/MR_TRIGO_FRACT;

       MR_Angle lPerpendicularAngle  = MR_NORMALIZE_ANGLE( pHorizontalDirection + MR_PI/2 );
       MR_Int32 lPerpendicularSpeed  = (mXSpeed*MR_Cos[ lPerpendicularAngle ]/MR_TRIGO_FRACT) + (mYSpeed*MR_Sin[ lPerpendicularAngle ]/MR_TRIGO_FRACT);

       MR_Int32 lXPerpendicularSpeed = lPerpendicularSpeed*MR_Cos[ lPerpendicularAngle ]/MR_TRIGO_FRACT;
       MR_Int32 lYPerpendicularSpeed = lPerpendicularSpeed*MR_Sin[ lPerpendicularAngle ]/MR_TRIGO_FRACT;

       mXSpeed = lXPerpendicularSpeed+ lXBounceSpeed;
       mYSpeed = lYPerpendicularSpeed+ lYBounceSpeed;
   }

}







