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

#pragma once

#include "Shapes.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Model {

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

class ContactSpec
{
	public:
		MR_Int32 mZMin;
		MR_Int32 mZMax;
};

class RoomContactSpec
{
	public:
		enum { eMaxWallContact = 6 };

		BOOL mTouchingRoom;

		MR_Int32 mDistanceFromFloor;			  // Can be negative
		MR_Int32 mDistanceFromCeiling;			  // Can be negative

		int mNbWallContact;						  // 0 mean no collision
		int mWallContact[eMaxWallContact];
};

BOOL MR_DllDeclare GetPolygonInclusion(const PolygonShape &pPolygon, const MR_2DCoordinate &pPosition);

// High level oontact function
BOOL MR_DllDeclare DetectActorContact(const ShapeInterface *pActor, const ShapeInterface *pObstacle, ContactSpec &pAnswer);
BOOL MR_DllDeclare DetectFeatureContact(const ShapeInterface *pActor, const PolygonShape *pFeature, ContactSpec &pAnswer);
void MR_DllDeclare DetectRoomContact(const ShapeInterface *pActor, const PolygonShape *pRoom, RoomContactSpec &pAnswer);

BOOL MR_DllDeclare GetActorForceLongitude(const ShapeInterface *pActor, const ShapeInterface *pObstacle, MR_Angle &pLongitude);
BOOL MR_DllDeclare GetFeatureForceLongitude(const ShapeInterface *pActor, const PolygonShape *pFeature, MR_Angle &pLongitude);
BOOL MR_DllDeclare GetWallForceLongitude(const ShapeInterface *pActor, const PolygonShape *pRoom, int pWallIndex, MR_Angle &pLongitude);

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
