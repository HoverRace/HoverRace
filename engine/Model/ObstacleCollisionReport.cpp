// FreeElementMovingInterface.cpp
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

#include "StdAfx.h"

#include "ObstacleCollisionReport.h"
#include "../Util/FastArray.h"

namespace HoverRace {
namespace Model {

// ObstacleCollisionReport

BOOL ObstacleCollisionReport::IsInMaze() const
{
	return mInMaze;
}

BOOL ObstacleCollisionReport::HaveContact() const
{
	ASSERT(mInMaze);

	return mHaveObstacleContact || (mClosestFloor < 0) || (mClosestCeiling < 0);
}

MR_Int32 ObstacleCollisionReport::StepHeight() const
{
	MR_Int32 lReturnValue = -mClosestFloor;

	ASSERT(mInMaze);

	if(mHaveObstacleContact) {
		lReturnValue = max(lReturnValue, mObstacleTop - mShapeBottom);
	}

	return lReturnValue;
}

MR_Int32 ObstacleCollisionReport::CeilingStepHeight() const
{
	MR_Int32 lReturnValue = -mClosestCeiling;

	ASSERT(mInMaze);

	if(mHaveObstacleContact) {
		lReturnValue = max(lReturnValue, mShapeTop - mObstacleBottom);
	}
	return lReturnValue;
}

MR_Int32 ObstacleCollisionReport::SpaceToFloor() const
{
	ASSERT(mInMaze);

	return mClosestFloor;
}

MR_Int32 ObstacleCollisionReport::SpaceToCeiling() const
{
	ASSERT(mInMaze);

	return mClosestCeiling;
}

MR_Int32 ObstacleCollisionReport::LargestHoleHeight() const
{
	return (mShapeTop - mShapeBottom) - (CeilingStepHeight() + StepHeight());
}

MR_Int32 ObstacleCollisionReport::LargestHoleStep() const
{
	return StepHeight();
}

BOOL ObstacleCollisionReport::AlmostCompleted() const
{
	return (mHaveObstacleContact && (mObstacleTop >= mShapeTop) && (mObstacleBottom <= mShapeBottom));
}

void ObstacleCollisionReport::GetContactWithObstacles(
	Level * pLevel, const ShapeInterface * pShape, int pRoom,
	FreeElement * pElement, BOOL pIgnoreActors)
{
	// Compute the touched rooms and the floor limits
	// Fort each room check collisions with the features
	// For each actor in the rooms, check the collisions with
	// there part list

	// I think that it is a really intensive calculation
	// Hope it will be fast enough X

	// First test the current room

	// First find aroom touching the shape

	mCurrentRoom = pLevel->FindRoomForPoint(MR_2DCoordinate(pShape->XPos(), pShape->YPos()), pRoom);

	if(mCurrentRoom == -1) {
		mInMaze = FALSE;
	}
	else {
		RoomContactSpec lSpec;
		pLevel->GetRoomContact(mCurrentRoom, pShape, lSpec);

		ASSERT(lSpec.mTouchingRoom);

		mInMaze = TRUE;
		mHaveObstacleContact = FALSE;

		mShapeTop = pShape->ZMax();
		mShapeBottom = pShape->ZMin();

		mClosestFloor = lSpec.mDistanceFromFloor;
		mClosestCeiling = lSpec.mDistanceFromCeiling;

		// Verify if the feature and actors of this room have contacts
		// with the shape

		GetContactWithFeaturesAndActors(pLevel, pShape, mCurrentRoom, pElement, pIgnoreActors);

		// For each of the touched walls,
		// Just verify immediate rooms for the moment
		for(int lCounter = 0; lCounter < lSpec.mNbWallContact; lCounter++) {
			int lNextRoom = pLevel->GetNeighbor(mCurrentRoom, lSpec.mWallContact[lCounter]);

			// If that wall have no neighbor, adjust the step height
			if(lNextRoom == -1) {
				mHaveObstacleContact = TRUE;
				mObstacleTop = mShapeTop;
				mObstacleBottom = mShapeBottom;
			}
			else {
				RoomContactSpec lNextSpec;

				pLevel->GetRoomContact(lNextRoom, pShape, lNextSpec);

				if(lNextSpec.mTouchingRoom) {
					mClosestFloor = min(mClosestFloor, lNextSpec.mDistanceFromFloor);
					mClosestCeiling = min(mClosestCeiling, lNextSpec.mDistanceFromCeiling);

					GetContactWithFeaturesAndActors(pLevel, pShape, lNextRoom, pElement, pIgnoreActors);

				}

			}

		}
	}
}

/**
 * Return the current room that this MR_ObstaceCollisionReport refers to.
 */
int ObstacleCollisionReport::Room() const
{
	return mCurrentRoom;
}

void ObstacleCollisionReport::GetContactWithFeaturesAndActors(
	Level * pLevel, const ShapeInterface * pShape, int pRoom,
	FreeElement * pElement, BOOL pIgnoreActors)
{

	int lCounter;
	ContactSpec lSpec;

	if(!AlmostCompleted()) {
		// First verify the features
		for(lCounter = 0; lCounter < pLevel->GetFeatureCount(pRoom); lCounter++) {
			if(pLevel->GetFeatureContact(pLevel->GetFeature(pRoom, lCounter), pShape, lSpec)) {
				if(mHaveObstacleContact) {
					mObstacleBottom = min(mObstacleBottom, lSpec.mZMin);
					mObstacleTop = max(mObstacleTop, lSpec.mZMax);
				}
				else {
					mHaveObstacleContact = TRUE;
					mObstacleBottom = lSpec.mZMin;
					mObstacleTop = lSpec.mZMax;
				}

				if(AlmostCompleted()) {
					break;
				}
			}
		}

		// Verify the actors
		if(!AlmostCompleted() && !pIgnoreActors) {
			MR_FreeElementHandle lObstacleHandle = pLevel->GetFirstFreeElement(pRoom);

			while(lObstacleHandle != NULL) {
				FreeElement *lObstacle = Level::GetFreeElement(lObstacleHandle);

				if(lObstacle != pElement) {

					const ShapeInterface *lObstacleShape = lObstacle->GetObstacleShape();

					if(lObstacleShape != NULL) {
						if(DetectActorContact(pShape, lObstacleShape, lSpec)) {
							if(mHaveObstacleContact) {
								mObstacleBottom = min(mObstacleBottom, lSpec.mZMin);
								mObstacleTop = max(mObstacleTop, lSpec.mZMax);
							}
							else {
								mHaveObstacleContact = TRUE;
								mObstacleBottom = lSpec.mZMin;
								mObstacleTop = lSpec.mZMax;
							}

							if(AlmostCompleted()) {
								break;
							}
						}
					}
				}
				lObstacleHandle = Level::GetNextFreeElement(lObstacleHandle);
			}
		}
	}
}

/*

void MR_FreeElementMovingInterface::AbsoluteMove( const MR_3DCoordinate& pNewPosition,
												  MR_Angle               pNewAngle     )
{
   // Find the section containing the new position
   int pNewSection = mLevel->FindRoomForPoint( pNewPosition, mSection );

   if( pNewSection != -1 )
   {
	  mLevel->MoveElement( mSection,
						   mElement,
						   pNewSection,
						   pNewPosition,
						   pNewAngle       );
   }
}

void MR_FreeElementMovingInterface::RelativeMove( const MR_3DCoordinate& pOffset, MR_Angle pRotation )
{
   // Compute new location and orientation
   MR_3DCoordinate lNewPosition;
   MR_Angle        lNewAngle;

   ComputePosition( pOffset,
					pRotation,
					lNewPosition,
					lNewAngle     );

   AbsoluteMove( lNewPosition, lNewAngle );

}

void MR_FreeElementMovingInterface::ComputePosition( const MR_3DCoordinate& pOffset,
													 MR_Angle               pRotation,
													 MR_3DCoordinate&       pNewPos,
													 MR_Angle&              pNewOrientation )const
{
   // Compute new location and orientation
   MR_3DCoordinate lNewPosition;
   MR_Angle        lNewAngle;
   MR_Angle        lNewAngleP90;
   FreeElement* lElement = mLevel->GetFreeElement( mLevel->GetFreeElementHandle( mSection, mElement ) );
   MR_3DCoordinate lOldPosition = lElement->mPosition;
   MR_Angle        lOldAngle    = lElement->mOrientation;

   lNewAngle    = MR_NORMALIZE_ANGLE( lOldAngle+pRotation );
   lNewAngleP90 = MR_NORMALIZE_ANGLE( lNewAngle+MR_PI/2 );

   lNewPosition.mZ = lOldPosition.mZ+pOffset.mZ;

   lNewPosition.mX = (pOffset.mX*MR_Cos[ lNewAngle    ]+pOffset.mY*MR_Cos[ lNewAngleP90 ])/MR_TRIGO_FRACT;
   lNewPosition.mY = (pOffset.mY*MR_Sin[ lNewAngleP90 ]+pOffset.mX*MR_Sin[ lNewAngle ])/MR_TRIGO_FRACT;

   lNewPosition.mX += lOldPosition.mX;
   lNewPosition.mY += lOldPosition.mY;

   pNewPos         = lNewPosition;
   pNewOrientation = lNewAngle;

}
*/

}  // namespace Model
}  // namespace HoverRace
