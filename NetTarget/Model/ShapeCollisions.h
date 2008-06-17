// ShapeCollisions.h
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


#ifndef SHAPE_COLLISIONS_H
#define SHAPE_COLLISIONS_H

#include "Shapes.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

// Collisions vs Movement
// 
// Collisions and movement are relatively linked together, that is why
// the movement algorithm will be described here.
//
// Actors can enter in collision with 3 Kind of thing:
//   Other actors (Each actor component is seperatly evaluated)
//   Feature      (The feature is evaluated as a whole)
//   Room limits  (Each wall, ceiling and floor are individually evaluated)
//
// It also exist two kind of contact(or collision) (Big simplification)
//   Horizontal
//      On the horizontal plan (with an horizontal orientation)
//   Vertical
//      On the vertical plan with a direction, either up or down
//
// To be revisted.. I have an idea
//


// Collisions description

class MR_ContactSpec
{
   public:
      MR_Int32 mZMin;
      MR_Int32 mZMax;
};


class MR_RoomContactSpec
{
   public:
      enum{ eMaxWallContact = 6 };

      BOOL mTouchingRoom;

      MR_Int32 mDistanceFromFloor;   // Can be negative
      MR_Int32 mDistanceFromCeiling; // Can be negative

      int  mNbWallContact; // 0 mean no collision
      int  mWallContact[ eMaxWallContact ];
};


BOOL MR_DllDeclare MR_GetPolygonInclusion( const MR_PolygonShape&     pPolygon,
                                           const MR_2DCoordinate&     pPosition );
                                          

// High level oontact function
BOOL MR_DllDeclare MR_DetectActorContact(    const MR_ShapeInterface* pActor, const MR_ShapeInterface*   pObstacle, MR_ContactSpec&     pAnswer );
BOOL MR_DllDeclare MR_DetectFeatureContact(  const MR_ShapeInterface* pActor, const MR_PolygonShape*     pFeature,  MR_ContactSpec&     pAnswer );
void MR_DllDeclare MR_DetectRoomContact(     const MR_ShapeInterface* pActor, const MR_PolygonShape*     pRoom,     MR_RoomContactSpec& pAnswer );

BOOL MR_DllDeclare MR_GetActorForceLongitude(   const MR_ShapeInterface* pActor, const MR_ShapeInterface* pObstacle,             MR_Angle& pLongitude );
BOOL MR_DllDeclare MR_GetFeatureForceLongitude( const MR_ShapeInterface* pActor, const MR_PolygonShape*   pFeature,              MR_Angle& pLongitude );
BOOL MR_DllDeclare MR_GetWallForceLongitude(    const MR_ShapeInterface* pActor, const MR_PolygonShape*   pRoom, int pWallIndex, MR_Angle& pLongitude );



#undef MR_DllDeclare 

#endif
