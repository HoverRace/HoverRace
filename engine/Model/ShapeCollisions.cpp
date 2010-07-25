// ShapeContacts.cpp
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

#include <math.h>

#include "ShapeCollisions.h"

namespace HoverRace {
namespace Model {

// Local prototypes
typedef BOOL(*MR_ActorActorContactFunc) (const ShapeInterface * pActor0, const ShapeInterface * pActor1, ContactSpec & pAnswer);
typedef BOOL(*MR_ActorActorLongitudeFunc) (const ShapeInterface * pActor0, const ShapeInterface * pActor1, MR_Angle & pLongitude);

static BOOL MR_CylinderCylinderContact(const CylinderShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_LineLineContact(const LineSegmentShape * pActor0, const LineSegmentShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_PolygonPolygonContact(const PolygonShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_CylinderLineContact(const CylinderShape * pActor0, const LineSegmentShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_CylinderPolygonContact(const CylinderShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_LinePolygonContact(const LineSegmentShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_LineCylinderContact(const LineSegmentShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_PolygonCylinderContact(const PolygonShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer);
static BOOL MR_PolygonLineContact(const PolygonShape * pActor0, const LineSegmentShape * pActor1, ContactSpec & pAnswer);

static void MR_CylinderRoomContact(const CylinderShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer);
static void MR_LineRoomContact(const LineSegmentShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer);
static void MR_PolygonRoomContact(const PolygonShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer);

static BOOL MR_TestLevelShape(const ShapeInterface * pActor0, const ShapeInterface * pActor1, ContactSpec & pAnswer);
static BOOL MR_TestBoundingBox(const ShapeInterface * pActor0, const ShapeInterface * pActor1);
static BOOL MR_IsOnLeft(const MR_2DCoordinate & pPointToCheck, const MR_2DCoordinate & pVectorOrigin, const MR_2DCoordinate & pVectorDest);
static BOOL MR_Is1Outside0(const PolygonShape * pActor0, const PolygonShape * pActor1);
static void MR_AddContactWall(int pWallIndex, RoomContactSpec & pAnswer);
static BOOL MR_AreLineCrossing(MR_Int32 pAX0, MR_Int32 pAY0, MR_Int32 pAX1, MR_Int32 pAY1, MR_Int32 pBX0, MR_Int32 pBY0, MR_Int32 pBX1, MR_Int32 pBY1);

static const MR_ActorActorContactFunc MR_ActorActorContactMatrix[3][3] =
{
	{
		(MR_ActorActorContactFunc) MR_CylinderCylinderContact,
		(MR_ActorActorContactFunc) MR_CylinderLineContact,
		(MR_ActorActorContactFunc) MR_CylinderPolygonContact
	},
	{
		(MR_ActorActorContactFunc) MR_LineCylinderContact,
		(MR_ActorActorContactFunc) MR_LineLineContact,
		(MR_ActorActorContactFunc) MR_LinePolygonContact
	},
	{
		(MR_ActorActorContactFunc) MR_PolygonCylinderContact,
		(MR_ActorActorContactFunc) MR_PolygonLineContact,
		(MR_ActorActorContactFunc) MR_PolygonPolygonContact
	}
};

BOOL GetPolygonInclusion(const PolygonShape &pPolygon, const MR_2DCoordinate &pPosition)
{
	BOOL lAnswer = TRUE;

	// Verify that the point is inside the bounding box of the polygon
	if((pPosition.mX < pPolygon.XMin()) || (pPosition.mX > pPolygon.XMax()) || (pPosition.mY < pPolygon.YMin()) || (pPosition.mY > pPolygon.YMax())) {
		lAnswer = FALSE;
	}
	else {
		// Verify that the given point is on the
		// right side of each line segment
		//
		// The point C is at the right side of segment
		// AB if the scalar product of( AB rotated by 90deg x AC ) is positive

		for(int lA = 0; lA < pPolygon.VertexCount(); lA++) {
			int lB = (lA + 1) % pPolygon.VertexCount();

			MR_Int32 lDXAB = pPolygon.X(lB) - pPolygon.X(lA);
			MR_Int32 lDYAB = pPolygon.Y(lB) - pPolygon.Y(lA);

			MR_Int32 lDXAC = pPosition.mX - pPolygon.X(lA);
			MR_Int32 lDYAC = pPosition.mY - pPolygon.Y(lA);

			MR_Int64 lScalarProduct = Int32x32To64(lDYAB, lDXAC) - Int32x32To64(lDXAB, lDYAC);

			if(lScalarProduct < 0) {
				lAnswer = FALSE;
				break;							  // There is no reason to continue
			}
		}
	}

	return lAnswer;
}

BOOL DetectActorContact(const ShapeInterface * pActor, const ShapeInterface * pObstacle, ContactSpec & pAnswer)
{
	return MR_ActorActorContactMatrix[pActor->ShapeType()][pObstacle->ShapeType()](pActor, pObstacle, pAnswer);
}

BOOL DetectFeatureContact(const ShapeInterface * pActor, const PolygonShape * pFeature, ContactSpec & pAnswer)
{
	return DetectActorContact(pActor, pFeature, pAnswer);
}

void DetectRoomContact(const ShapeInterface * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer)
{

	// Initialize basic stuff
	pAnswer.mTouchingRoom = FALSE;
	pAnswer.mNbWallContact = 0;

	// Compute the vertical variables
	pAnswer.mDistanceFromFloor = pActor->ZMin() - pRoom->ZMin();
	pAnswer.mDistanceFromCeiling = pRoom->ZMax() - pActor->ZMax();

	// Verify if the bounding boxes are matching

	if(MR_TestBoundingBox(pActor, pRoom)) {
		switch (pActor->ShapeType()) {
			case ShapeInterface::eCylinder:
				MR_CylinderRoomContact((const CylinderShape *) pActor, pRoom, pAnswer);
				break;

			case ShapeInterface::eLineSegment:
				MR_LineRoomContact((const LineSegmentShape *) pActor, pRoom, pAnswer);
				break;

			case ShapeInterface::ePolygon:
				MR_PolygonRoomContact((const PolygonShape *) pActor, pRoom, pAnswer);
				break;

			default:
				ASSERT(FALSE);					  // Shape type not supported
		}
	}
}

BOOL GetActorForceLongitude(const ShapeInterface * pActor, const ShapeInterface * pObstacle, MR_Angle & pLongitude)
{
	// Big approximation:Assume that the shape is not a long shape
	int lX0 = pActor->XPos();
	int lY0 = pActor->YPos();
	int lX1 = pObstacle->XPos();
	int lY1 = pObstacle->YPos();

	pLongitude = RAD_2_MR_ANGLE(atan2((double) lY0 - lY1, (double) lX0 - lX1));
	return TRUE;
}

BOOL GetFeatureForceLongitude(const ShapeInterface * pActor, const PolygonShape * pFeature, MR_Angle & pLongitude)
{

	MR_Int32 lXMassCenter = pActor->XPos();
	MR_Int32 lYMassCenter = pActor->YPos();

	// To determine the horizontal force ray, keep the greatest distance of the center of mass
	// if that distance is positive, take the orientation of the wall
	// associated with greatest distance for the calculationof the force field

	BOOL lReturnValue;

	int lGreatestDistance = -1;
	int lForceWall = -1;
	int lNbExtern = 0;

	int lVertexCount = pFeature->VertexCount();

	for(int lCounter = 0; lCounter < lVertexCount; lCounter++) {
		int lP1 = (lCounter + 1) % lVertexCount;

		MR_Int32 lLeftDistance = -(pFeature->Y(lP1) - pFeature->Y(lCounter)) * (lXMassCenter - pFeature->X(lCounter))
			+ (pFeature->X(lP1) - pFeature->X(lCounter)) * (lYMassCenter - pFeature->Y(lCounter));

		if(lLeftDistance > 0) {
			lLeftDistance /= pFeature->SideLen(lCounter);

			if(lLeftDistance > lGreatestDistance) {
				lGreatestDistance = lLeftDistance;
				lForceWall = lCounter;
			}
			lNbExtern++;
		}
	}

	if(lForceWall == -1) {
		lReturnValue = FALSE;
	}
	else {
		lReturnValue = TRUE;

		if(lNbExtern > 1) {
			// Compute vertor based on center of mass of the feature
			// TODO (Wrong method)
			int lP1 = (lForceWall + 1) % lVertexCount;

			pLongitude = RAD_2_MR_ANGLE(atan2((double) pFeature->X(lP1) - pFeature->X(lForceWall), (double) pFeature->Y(lForceWall) - pFeature->Y(lP1)));

		}
		else {
			// Compute vertor based on the hitted wall
			int lP1 = (lForceWall + 1) % lVertexCount;

			pLongitude = RAD_2_MR_ANGLE(atan2((double) pFeature->X(lP1) - pFeature->X(lForceWall), (double) pFeature->Y(lForceWall) - pFeature->Y(lP1)));
		}
	}

	return lReturnValue;
}

BOOL GetWallForceLongitude(const ShapeInterface * /*pActor */ , const PolygonShape * pRoom, int pWallIndex, MR_Angle & pLongitude)
{
	// return a vector perpendicular to the selected wall
	int lP1 = (pWallIndex + 1) % pRoom->VertexCount();

	pLongitude = RAD_2_MR_ANGLE(atan2((double) -(pRoom->X(lP1) - pRoom->X(pWallIndex)), (double) pRoom->Y(lP1) - pRoom->Y(pWallIndex)));

	return TRUE;
}

// Local functions implementation

BOOL MR_CylinderCylinderContact(const CylinderShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer)
{

	// First verify the level of the objects
	BOOL lReturnValue = MR_TestLevelShape(pActor0, pActor1, pAnswer);

	if(lReturnValue) {
		// Verify that the objects are in a relativly close range
		int lRaySum = pActor0->RayLen() + pActor1->RayLen();

		int lXDist, lXDistAbs;
		int lYDist, lYDistAbs;
		lXDist = lXDistAbs = pActor1->AxisX() - pActor0->AxisX();
		lYDist = lYDistAbs = pActor1->AxisY() - pActor0->AxisY();

		if(lXDistAbs < 0) {
			lXDistAbs = -lXDistAbs;
		}

		if(lXDistAbs > lRaySum) {
			lReturnValue = FALSE;
		}
		else {
			if(lYDistAbs < 0) {
				lYDistAbs = -lYDistAbs;
			}

			if(lYDistAbs > lRaySum) {
				lReturnValue = FALSE;
			}
			else {
				if((lXDistAbs * lXDistAbs + lYDistAbs * lYDistAbs) > (lRaySum * lRaySum)) {
					lReturnValue = FALSE;
				}
				else {
					// OK we have a contact
				}
			}
		}
	}
	return lReturnValue;
}

BOOL MR_LineLineContact(const LineSegmentShape * /*pActor0 */ , const LineSegmentShape * /*pActor1 */ , ContactSpec & /*pAnswer */ )
{
	// Line segments can not enter in collision
	return FALSE;
}

BOOL MR_PolygonPolygonContact(const PolygonShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer)
{

	// First verify the level of the objects
	BOOL lReturnValue = MR_TestLevelShape(pActor0, pActor1, pAnswer);

	if(lReturnValue) {
		lReturnValue = MR_TestBoundingBox(pActor0, pActor1);
	}

	if(lReturnValue) {
		// Ok we must now verity that the polygon is touching the other polygon
		// If for one of the polygons, it exist a side for witch all the vertex of the other polygon
		// are on its left, the polygons are not touching each other

		// This is an intensive calculation (Hope that they are small polygons)
		lReturnValue = !(MR_Is1Outside0(pActor0, pActor1) && MR_Is1Outside0(pActor0, pActor1));

	}

	return lReturnValue;
}

BOOL MR_CylinderLineContact(const CylinderShape * pActor0, const LineSegmentShape * pActor1, ContactSpec & pAnswer)
{
	// First check the 3D bounding box
	BOOL lReturnValue = MR_TestLevelShape(pActor0, pActor1, pAnswer);

	if(lReturnValue) {
		lReturnValue = MR_TestBoundingBox(pActor0, pActor1);
	}
	// Now we only make a 2D check. we assume that the
	// line segment is almost horizontal (That mean that it is now a wall not a line segment
	if(lReturnValue) {
		// Just compute the distance between the line and the cylinder. this will be a very good aproximation
		MR_Int32 lDistance = (-(pActor1->Y1() - pActor1->Y0()) * (pActor0->AxisX() - pActor1->X0())
			+ (pActor1->X1() - pActor1->X0()) * (pActor0->AxisY() - pActor1->Y0())) / pActor1->HorizontalLen();

		if(lDistance < 0) {
			lDistance = -lDistance;
		}

		if(lDistance > pActor0->RayLen()) {
			lReturnValue = FALSE;
		}
		else {
			// Ok we have a contact
		}
	}
	return lReturnValue;
}

BOOL MR_CylinderPolygonContact(const CylinderShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer)
{
	// First check the 3D bounding box
	BOOL lReturnValue = MR_TestLevelShape(pActor0, pActor1, pAnswer);

	if(lReturnValue) {
		lReturnValue = MR_TestBoundingBox(pActor0, pActor1);
	}

	if(lReturnValue) {
		// For each side of the polygon, verify that
		// the left perpendicular distance of the center of
		// the cylinder is < than the cylinder ray

		int lVertexCount = pActor1->VertexCount();

		for(int lCounter = 0; lReturnValue && (lCounter < lVertexCount); lCounter++) {
			int lP1 = (lCounter + 1) % lVertexCount;

			MR_Int32 lLeftDistance = -(pActor1->Y(lP1) - pActor1->Y(lCounter)) * (pActor0->AxisX() - pActor1->X(lCounter))
				+ (pActor1->X(lP1) - pActor1->X(lCounter)) * (pActor0->AxisY() - pActor1->Y(lCounter));

			if(lLeftDistance > 0) {
				lLeftDistance /= pActor1->SideLen(lCounter);

				if(lLeftDistance > pActor0->RayLen()) {
					lReturnValue = FALSE;
				}
			}
		}
	}

	return lReturnValue;
}

BOOL MR_LinePolygonContact(const LineSegmentShape * pActor0, const PolygonShape * pActor1, ContactSpec & pAnswer)
{
	// First check the 3D bounding box
	BOOL lReturnValue = MR_TestLevelShape(pActor0, pActor1, pAnswer);

	if(lReturnValue) {
		lReturnValue = MR_TestBoundingBox(pActor0, pActor1);
	}
	// Now we only make a 2D check. we assume that the
	// line segment is almost horizontal (That mean that it is now a wall not a line segment
	if(lReturnValue) {
		// The real test should be made in two phase
		// Phase 1
		//   Verify that there is vertex of the polygon on both
		//   side of the line segment
		// Phase 2
		//   Verify that it do not exist a side of the polygon for witch the
		//   two points of the line segment are on it's left
		//
		// Usually both condition have to be true to declare that the line segment
		// intersect with the polygon.
		// Because this is very intensive calculation we will only check the phase 1

		int lVertexCount = pActor1->VertexCount();

		int lX0 = pActor0->X0();
		int lY0 = pActor0->Y0();

		int lXLen = pActor0->X1() - lX0;
		int lYLen = pActor0->Y1() - lY0;

		lReturnValue = FALSE;

		BOOL lRight = FALSE;
		BOOL lLeft = FALSE;

		for(int lCounter = 0; lReturnValue && (lCounter < lVertexCount); lCounter++) {
			MR_Int32 lDistance = -lYLen * (pActor1->X(lCounter) - lX0)
				+ lXLen * (pActor1->Y(lCounter) - lY0);

			if(lDistance < 0) {
				lRight = TRUE;

				if(lLeft) {
					lReturnValue = TRUE;
				}
			}
			else {
				lLeft = TRUE;

				if(lRight) {
					lReturnValue = TRUE;
				}
			}
		}
	}
	return lReturnValue;
}

BOOL MR_LineCylinderContact(const LineSegmentShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer)
{
	BOOL lReturnValue = MR_CylinderLineContact(pActor1, pActor0, pAnswer);

	return lReturnValue;
}

BOOL MR_PolygonCylinderContact(const PolygonShape * pActor0, const CylinderShape * pActor1, ContactSpec & pAnswer)
{
	BOOL lReturnValue = MR_CylinderPolygonContact(pActor1, pActor0, pAnswer);

	return lReturnValue;
}

BOOL MR_PolygonLineContact(const PolygonShape * pActor0, const LineSegmentShape * pActor1, ContactSpec & pAnswer)
{
	BOOL lReturnValue = MR_LinePolygonContact(pActor1, pActor0, pAnswer);

	return lReturnValue;
}

void MR_CylinderRoomContact(const CylinderShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer)
{
	// For each side of the polygon, verify that
	// the left perpendicular distance of the center of
	// the cylinder is < than the cylinder ray

	int lVertexCount = pRoom->VertexCount();

	pAnswer.mTouchingRoom = TRUE;

	for(int lCounter = 0; pAnswer.mTouchingRoom && (lCounter < lVertexCount); lCounter++) {
		int lP1 = (lCounter + 1) % lVertexCount;

		MR_Int32 lLeftDistance = -(pRoom->Y(lP1) - pRoom->Y(lCounter)) * (pActor->AxisX() - pRoom->X(lCounter))
			+ (pRoom->X(lP1) - pRoom->X(lCounter)) * (pActor->AxisY() - pRoom->Y(lCounter));

		lLeftDistance /= pRoom->SideLen(lCounter);

		if(lLeftDistance > 0) {
			if(lLeftDistance > pActor->RayLen()) {
				pAnswer.mTouchingRoom = FALSE;
			}
		}

		if(pAnswer.mTouchingRoom) {
			if(lLeftDistance > -pActor->RayLen()) {
				// This side is potentially crossing the selectedside
				/*
				   MR_Int32 lOldLenDistance =  (pRoom->X(lP1)-pRoom->X(lCounter))*(pActor->AxisX()-pRoom->X(lCounter))
				   +(pRoom->Y(lP1)-pRoom->Y(lCounter))*(pActor->AxisY()-pRoom->Y(lCounter));

				   lOldLenDistance /= pRoom->SideLen( lCounter );
				 */

				MR_Int32 lLenDistance = ((pRoom->X(lP1) - pRoom->X(lCounter)) / 2) * ((pActor->AxisX() - pRoom->X(lCounter)) / 2)
					+ ((pRoom->Y(lP1) - pRoom->Y(lCounter)) / 2) * ((pActor->AxisY() - pRoom->Y(lCounter)) / 2);

				lLenDistance /= pRoom->SideLen(lCounter) / 4;

				/*
				   MR_Int32 lDiff = lLenDistance - lOldLenDistance;

				   ASSERT( (lDiff<20)&&(lDiff>-20) );
				 */

				if(lLenDistance < 0) {
					if(lLenDistance >= -pActor->RayLen()) {
						if((pRoom->X(lCounter) - pActor->AxisX()) * (pRoom->X(lCounter) - pActor->AxisX())
						+ (pRoom->Y(lCounter) - pActor->AxisY()) * (pRoom->Y(lCounter) - pActor->AxisY()) <= pActor->RayLen() * pActor->RayLen()) {
							MR_AddContactWall(lCounter, pAnswer);
						}
					}
				}
				else if(lLenDistance > pRoom->SideLen(lCounter)) {
					if(lLenDistance <= pActor->RayLen() + pRoom->SideLen(lCounter)) {
						if((pRoom->X(lP1) - pActor->AxisX()) * (pRoom->X(lP1) - pActor->AxisX())
						+ (pRoom->Y(lP1) - pActor->AxisY()) * (pRoom->Y(lP1) - pActor->AxisY()) <= pActor->RayLen() * pActor->RayLen()) {
							MR_AddContactWall(lCounter, pAnswer);
						}
					}
				}
				else {
					MR_AddContactWall(lCounter, pAnswer);
				}

			}
		}
	}
}

void MR_LineRoomContact(const LineSegmentShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer)
{
	// Count the crossed sides
	int lVertexCount = pRoom->VertexCount();

	for(int lCounter = 0; pAnswer.mTouchingRoom && (lCounter < lVertexCount); lCounter++) {
		int lP1 = (lCounter + 1) % lVertexCount;

		if(MR_AreLineCrossing(pRoom->X(lCounter), pRoom->Y(lCounter), pRoom->X(lP1), pRoom->Y(lP1), pActor->X0(), pActor->Y0(), pActor->X1(), pActor->Y1())) {
			MR_AddContactWall(lCounter, pAnswer);
		}
	}

	// if no side is crossed verify if one of the points
	// is in room shape

	if(pAnswer.mNbWallContact > 0) {
		pAnswer.mTouchingRoom = TRUE;
	}
	else {
		if(GetPolygonInclusion(*pRoom, MR_2DCoordinate(pActor->X0(), pActor->X1()))) {
			pAnswer.mTouchingRoom = TRUE;
		}
	}
}

void MR_PolygonRoomContact(const PolygonShape * pActor, const PolygonShape * pRoom, RoomContactSpec & pAnswer)
{
	// For each wall of the Room, verify if it is crossed by a wall of the
	// polygon to check

	int lRoomSides = pRoom->VertexCount();
	int lActorSides = pActor->VertexCount();

	for(int lP0Room = 0; lP0Room < lRoomSides; lP0Room++) {
		int lP1Room = (lP0Room + 1) % lRoomSides;

		// Verify if the bounding box of the actor cross the selected side
		if((pActor->XMin() <= max(pRoom->X(lP0Room), pRoom->X(lP1Room)))
		&& (pActor->XMax() >= min(pRoom->X(lP0Room), pRoom->X(lP1Room)))) {
			if((pActor->YMin() <= max(pRoom->Y(lP0Room), pRoom->Y(lP1Room)))
			&& (pActor->YMax() >= min(pRoom->Y(lP0Room), pRoom->Y(lP1Room)))) {
				for(int lP0Actor = 0; lP0Actor < lActorSides; lP0Actor++) {
					int lP1Actor = (lP0Actor + 1) % lActorSides;

					if(MR_AreLineCrossing(pRoom->X(lP0Room), pRoom->Y(lP0Room), pRoom->X(lP1Room), pRoom->Y(lP1Room), pActor->X(lP0Actor), pActor->Y(lP0Actor), pActor->X(lP1Actor), pActor->Y(lP1Actor))) {
						MR_AddContactWall(lP0Room, pAnswer);
						break;
					}

				}
			}
		}
	}

	// if no side is crossed verify if one of the points
	// is in room shape

	if(pAnswer.mNbWallContact > 0) {
		pAnswer.mTouchingRoom = TRUE;
	}
	else {
		if(GetPolygonInclusion(*pRoom, MR_2DCoordinate(pActor->X(0), pActor->Y(0)))) {
			pAnswer.mTouchingRoom = TRUE;
		}
	}
}

BOOL MR_TestLevelShape(const ShapeInterface * pActor0, const ShapeInterface * pActor1, ContactSpec & pAnswer)
{
	int lZMin0 = pActor0->ZMin();
	int lZMin1 = pActor1->ZMin();
	int lZMax0 = pActor0->ZMax();
	int lZMax1 = pActor1->ZMax();

	pAnswer.mZMin = max(lZMin0, lZMin1);
	pAnswer.mZMax = min(lZMax0, lZMax1);

	return pAnswer.mZMin < pAnswer.mZMax;
}

BOOL MR_TestBoundingBox(const ShapeInterface * pActor0, const ShapeInterface * pActor1)
{
	BOOL lReturnValue = TRUE;

	int lXMin = max(pActor0->XMin(), pActor1->XMin());
	int lXMax = min(pActor0->XMax(), pActor1->XMax());

	if(lXMax < lXMin) {
		lReturnValue = FALSE;
	}
	else {
		int lYMin = max(pActor0->YMin(), pActor1->YMin());
		int lYMax = min(pActor0->YMax(), pActor1->YMax());

		if(lXMax < lXMin) {
			lReturnValue = FALSE;
		}
	}
	return lReturnValue;
}

BOOL MR_IsOnLeft(const MR_2DCoordinate & pPointToCheck, const MR_2DCoordinate & pVectorOrigin, const MR_2DCoordinate & pVectorDest)
{
	MR_Int32 lLeftScalarResult = -(pVectorDest.mY - pVectorOrigin.mY) * (pPointToCheck.mX - pVectorOrigin.mX)
		+ (pVectorDest.mX - pVectorOrigin.mX) * (pPointToCheck.mY - pVectorOrigin.mY);

	return lLeftScalarResult > 0;
}

BOOL MR_Is1Outside0(const PolygonShape * pActor0, const PolygonShape * pActor1)
{
	BOOL lReturnValue = FALSE;

	int lVertex0Count = pActor0->VertexCount();
	int lVertex1Count = pActor0->VertexCount();

	for(int lVertex1 = 0; !lReturnValue && (lVertex1 < lVertex1Count); lVertex1++) {
		lReturnValue = TRUE;

		for(int lVertex0 = 0; lReturnValue && (lVertex0 < lVertex0Count); lVertex0++) {
			lReturnValue = MR_IsOnLeft(MR_2DCoordinate(pActor0->X(lVertex0), pActor1->Y(lVertex0)), MR_2DCoordinate(pActor0->X(lVertex1), pActor1->Y(lVertex1)), MR_2DCoordinate(pActor0->X((lVertex1 + 1) % lVertex1Count), pActor1->X((lVertex1 + 1) % lVertex1Count)));
		}
	}

	return lReturnValue;
}

void MR_AddContactWall(int pWallIndex, RoomContactSpec & pAnswer)
{
	if(pAnswer.mNbWallContact < RoomContactSpec::eMaxWallContact) {
		pAnswer.mWallContact[pAnswer.mNbWallContact++] = pWallIndex;
	}
	else {
		ASSERT(FALSE);							  // Room is too detailled
	}
}

BOOL MR_AreLineCrossing(MR_Int32 pAX0, MR_Int32 pAY0, MR_Int32 pAX1, MR_Int32 pAY1, MR_Int32 pBX0, MR_Int32 pBY0, MR_Int32 pBX1, MR_Int32 pBY1)
{
	BOOL lReturnValue = FALSE;

	// Go by elimination
	if((min(pBX0, pBX1) <= max(pAX0, pAX1))
	&& (max(pBX0, pBX1) >= min(pAX0, pAX1))) {
		if((min(pBY0, pBY1) <= max(pAY0, pAY1))
		&& (max(pBY0, pBY1) >= min(pAX0, pAY1))) {
			// Now verify that the two points of B are on each side of A
			int lDist0 = (pAY1 - pAY0 * pBX0 - pAX0)
				- (pAX1 - pAX0 * pBY0 - pAY0);

			int lDist1 = (pAY1 - pAY0 * pBX1 - pAX0)
				- (pAX1 - pAX0 * pBY1 - pAY0);

			if(((lDist0 < 0) && (lDist1 >= 0)) || ((lDist0 >= 0) && (lDist1 < 0))) {
				int lDist2 = (pBY1 - pBY0 * pAX0 - pBX0)
					- (pBX1 - pBX0 * pAY0 - pBY0);

				int lDist3 = (pBY1 - pBY0 * pAX1 - pBX0)
					- (pBX1 - pBX0 * pAY1 - pBY0);

				if(((lDist2 < 0) && (lDist3 >= 0)) || ((lDist2 >= 0) && (lDist3 < 0))) {
					lReturnValue = TRUE;
				}
			}
		}
	}
	return lReturnValue;
}

}  // namespace Model
}  // namespace HoverRace
