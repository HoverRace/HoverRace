
// GameSession.cpp
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

#include "GameSession.h"
#include "ObstacleCollisionReport.h"
#include "../Model/Level.h"
#include "../Model/Track.h"
#include "../Util/FastArray.h"

#define MR_SIMULATION_SLICE             15
#define MR_MINIMUM_SIMULATION_SLICE     10

using namespace HoverRace::Parcel;

namespace HoverRace {
namespace Model {

GameSession::GameSession(bool pAllowRendering) :
	mAllowRendering(pAllowRendering),
	mCurrentLevelNumber(-1),
	mSimulationTime(-3000),  // 3 sec countdown
	mLastSimulateCallTime(Util::OS::Time())
{
}

GameSession::~GameSession()
{
	Clean();
}

bool GameSession::LoadLevel(const Model::GameOptions &gameOpts)
{
	track->Load(mAllowRendering, gameOpts);
	return true;
}

void GameSession::Clean()
{
	mCurrentLevelNumber = -1;
}

bool GameSession::LoadNew(const char *pTitle, std::shared_ptr<Track> track,
	const Model::GameOptions &gameOpts)
{
	bool lReturnValue = false;

	Clean();
	if (track) {
		mTitle = pTitle;
		this->track = track;
		lReturnValue = LoadLevel(gameOpts);

		if(!lReturnValue)
			Clean();
	}

	return lReturnValue;
}

void GameSession::SetSimulationTime(MR_SimulationTime pTime)
{
	mSimulationTime = pTime;
	mLastSimulateCallTime = Util::OS::Time();
}

MR_SimulationTime GameSession::GetSimulationTime() const
{
	return mSimulationTime;
}

void GameSession::Simulate()
{
	Util::OS::timestamp_t lSimulateCallTime = Util::OS::Time();
	MR_SimulationTime lTimeToSimulate;

	// Determine the duration of the simulation step
	lTimeToSimulate = static_cast<MR_SimulationTime>(
		lSimulateCallTime - mLastSimulateCallTime);

	if(lTimeToSimulate < 0)
		lTimeToSimulate = 0;

	/*
	   if( (lTimeToSimulate < 0 )||(lTimeToSimulate>500) )
	   {
	   // There is someting wrong with the calculated duration
	   // This is probably because the game have been paused
	   // (or the refresh rate is extrenemly slow, less than 2 frames sec)

	   // Use a relativly small time step
	   lTimeToSimulate = 20; // 2/100 of second
	   }
	 */

	while(lTimeToSimulate >= MR_SIMULATION_SLICE) {
		SimulateFreeElems(mSimulationTime < 0 ? 0 : MR_SIMULATION_SLICE);
		lTimeToSimulate -= MR_SIMULATION_SLICE;
		mSimulationTime += MR_SIMULATION_SLICE;
	}

	if(lTimeToSimulate >= MR_MINIMUM_SIMULATION_SLICE) {
		SimulateFreeElems(mSimulationTime < 0 ? 0 : lTimeToSimulate);
		mSimulationTime += lTimeToSimulate;
		lTimeToSimulate = 0;
	}

	SimulateSurfaceElems(static_cast<MR_SimulationTime>(
		lSimulateCallTime - lTimeToSimulate - mLastSimulateCallTime));

	mLastSimulateCallTime = lSimulateCallTime - lTimeToSimulate;
}

void GameSession::SimulateLateElement(MR_FreeElementHandle pElement,
	MR_SimulationTime pDuration, int pRoom)
{
	MR_SimulationTime lTimeToSimulate = pDuration;

	if(lTimeToSimulate < 0) {
		return;
	}

	if(mSimulationTime <= lTimeToSimulate) {
		// can't backtrack below 0
		return;
	}
	// clock back
	MR_SimulationTime lOriginalTime = mSimulationTime;
	mSimulationTime -= lTimeToSimulate;

	while((pRoom >= 0) && (lTimeToSimulate >= MR_SIMULATION_SLICE)) {
		pRoom = SimulateOneFreeElem(MR_SIMULATION_SLICE, pElement, pRoom);
		lTimeToSimulate -= MR_SIMULATION_SLICE;
		mSimulationTime += MR_SIMULATION_SLICE;
	}

	if((pRoom >= 0) && (lTimeToSimulate >= MR_MINIMUM_SIMULATION_SLICE)) {
		pRoom = SimulateOneFreeElem(lTimeToSimulate, pElement, pRoom);
		SimulateFreeElems(mSimulationTime < 0 ? 0 : lTimeToSimulate);
	}
	// return to good time
	mSimulationTime = lOriginalTime;
}

void GameSession::SimulateSurfaceElems(MR_SimulationTime /*pTimeToSimulate */ )
{
	// Give the control to each surface so they can update there state

	// TODO but it's a big job that will slow the simulation (find a better way)

}

int GameSession::SimulateOneFreeElem(MR_SimulationTime pTimeToSimulate,
	MR_FreeElementHandle pElementHandle, int pRoom)
{
	Level *mCurrentLevel = track->GetLevel();

	BOOL lDeleteElem = FALSE;
	FreeElement *lElement = mCurrentLevel->GetFreeElement(pElementHandle);

	// Ask the element to simulate its movement
	int lNewRoom = lElement->Simulate(pTimeToSimulate, *track, pRoom);
	int lReturnValue = lNewRoom;

	if(lNewRoom == Level::eMustBeDeleted) {
		lDeleteElem = TRUE;
		lNewRoom = pRoom;
	}

	if(pRoom != lNewRoom)
		mCurrentLevel->MoveElement(pElementHandle, lNewRoom);

	// Compute interaction of the element with the environment
	const ShapeInterface *lContactShape = lElement->GetGivingContactEffectShape();

	if(lContactShape != NULL) {
		// Compute contact with structural elements
		MR_FixedFastArray < int, 20 > lVisitedRooms;
		RoomContactSpec lSpec;

		// Do the contact treatement for that room
		mCurrentLevel->GetRoomContact(lNewRoom, lContactShape, lSpec);

		ComputeShapeContactEffects(lNewRoom, lElement, lSpec, &lVisitedRooms, 1, pTimeToSimulate);
	}

	if(lDeleteElem)
		mCurrentLevel->DeleteElement(pElementHandle);
	return lReturnValue;
}

void GameSession::SimulateFreeElems(MR_SimulationTime pTimeToSimulate)
{
	Level *mCurrentLevel = track->GetLevel();

	// Do the simulation
	int lRoomIndex;

	// Interaction objects collection

	// Simulate element by element
	for(lRoomIndex = -1; lRoomIndex < mCurrentLevel->GetRoomCount(); lRoomIndex++) {
		// Simulate FreeElements
		MR_FreeElementHandle lElementHandle = mCurrentLevel->GetFirstFreeElement(lRoomIndex);

		while(lElementHandle != NULL) {
			MR_FreeElementHandle lNext = Level::GetNextFreeElement(lElementHandle);
			SimulateOneFreeElem(pTimeToSimulate, lElementHandle, lRoomIndex);
			lElementHandle = lNext;
		}
	}
	mCurrentLevel->FlushPermElementPosCache();
}

void GameSession::ComputeShapeContactEffects(int pCurrentRoom,
	FreeElement *pActor, const RoomContactSpec &pLastSpec,
	MR_FastArrayBase<int> *pVisitedRooms, int pMaxDepth,
	MR_SimulationTime pDuration)
{
	Level *mCurrentLevel = track->GetLevel();

	int lCounter;
	ContactSpec lSpec;

	const ShapeInterface *lActorShape = pActor->GetGivingContactEffectShape();

	BOOL lValidDirection = FALSE;
	MR_Angle lDirectionAngle = 0;

	pVisitedRooms->Add(pCurrentRoom);

	// Compute contact with features

	// Not correctly done. If a collision is detected, only the floor of the
	// feature is assumed to be touched (directio is ok but wrong elementis selected
	int lNbFeatures = mCurrentLevel->GetFeatureCount(pCurrentRoom);

	for(lCounter = 0; lCounter < lNbFeatures; lCounter++) {
		int lFeatureId = mCurrentLevel->GetFeature(pCurrentRoom, lCounter);

		if(mCurrentLevel->GetFeatureContact(lFeatureId, lActorShape, lSpec)) {
			SurfaceElement *lFloor = mCurrentLevel->GetFeatureTopElement(lFeatureId);

			// Ok Compute the directiion of the collision
			if(lSpec.mZMax <= lActorShape->ZMin()) {
				lValidDirection = FALSE;
			}
			else if(lSpec.mZMin >= lActorShape->ZMax()) {
				lValidDirection = FALSE;
			}
			else {
				lValidDirection = mCurrentLevel->GetFeatureContactOrientation(lFeatureId, lActorShape, lDirectionAngle);
			}

			// const ContactEffectList* lActorEffectList    = pActor->GetEffectList();
			const ContactEffectList *lObstacleEffectList = lFloor->GetEffectList();

			// Apply feature effects to the actor
			pActor->ApplyEffects(lObstacleEffectList,
				mSimulationTime, pDuration,
				lValidDirection, lDirectionAngle,
				lSpec.mZMin, lSpec.mZMax, *track);

			// Apply actor effects to the feature
			// if( lValidDirection )
			//{
			//   lDirectionAngle = MR_NORMALIZE_ANGLE( lDirectionAngle+MR_PI );
			//   lFloor->ApplyEffects( lActorEffectList, mSimulationTime, pDuration, lValidDirection, lDirectionAngle );
			//}
		}
	}

	// Compute interaction with room actors
	MR_FreeElementHandle lObstacleHandle = mCurrentLevel->GetFirstFreeElement(pCurrentRoom);

	while(lObstacleHandle != NULL) {
		FreeElement *lObstacleElem = Level::GetFreeElement(lObstacleHandle);

		if(lObstacleElem != pActor) {

			if(DetectActorContact(lActorShape, lObstacleElem->GetReceivingContactEffectShape(), lSpec)) {
				// Ok Compute the directiion of the collision
				if(lSpec.mZMax <= lActorShape->ZMin()) {
					lValidDirection = FALSE;
				}
				else if(lSpec.mZMin >= lActorShape->ZMax()) {
					lValidDirection = FALSE;
				}
				else {
					lValidDirection = GetActorForceLongitude(lActorShape, lObstacleElem->GetReceivingContactEffectShape(), lDirectionAngle);
				}

				const ContactEffectList *lActorEffectList = pActor->GetEffectList();
				const ContactEffectList *lObstacleEffectList = lObstacleElem->GetEffectList();

				// Apply feature effects to the actor
				pActor->ApplyEffects(lObstacleEffectList,
					mSimulationTime, pDuration,
					lValidDirection, lDirectionAngle,
					lSpec.mZMin, lSpec.mZMax, *track);

				// Apply actor effects to the feature
				lDirectionAngle = MR_NORMALIZE_ANGLE(lDirectionAngle + MR_PI);
				lObstacleElem->ApplyEffects(lActorEffectList,
					mSimulationTime, pDuration,
					lValidDirection, lDirectionAngle,
					lSpec.mZMin, lSpec.mZMax, *track);

			}
		}
		lObstacleHandle = Level::GetNextFreeElement(lObstacleHandle);
	}

	// Compute interaction with touched walls
	// at the same time compute interaction with other rooms

	for(lCounter = 0; lCounter < pLastSpec.mNbWallContact; lCounter++) {
		// Verify that the wall is a wall on all its height

		MR_Int32 lContactTop = lActorShape->ZMax();
		MR_Int32 lContactBottom = lActorShape->ZMin();

		int lNeighbor = mCurrentLevel->GetNeighbor(pCurrentRoom,
			pLastSpec.mWallContact[lCounter]);

		if(lNeighbor != -1) {
			if(!pVisitedRooms->Contains(lNeighbor)) {
				RoomContactSpec cspec;

				// Recursively call this function

				mCurrentLevel->GetRoomContact(lNeighbor, lActorShape, cspec);

				if((cspec.mDistanceFromFloor < 0) || (cspec.mDistanceFromCeiling < 0)) {
					lNeighbor = -1;
				}
				else {
					ComputeShapeContactEffects(lNeighbor, pActor, cspec,
						pVisitedRooms, pMaxDepth - 1, pDuration);
				}
			}
		}

		if(lNeighbor == -1) {
			// Apply the effect of the wall
			if(mCurrentLevel->GetRoomWallContactOrientation(pCurrentRoom, pLastSpec.mWallContact[lCounter], lActorShape, lDirectionAngle)) {
				SurfaceElement *lWall = mCurrentLevel->GetRoomWallElement(pCurrentRoom, static_cast<size_t>(pLastSpec.mWallContact[static_cast<size_t>(lCounter)]));
				// const ContactEffectList* lActorEffectList    = pActor->GetEffectList();
				const ContactEffectList *lWallEffectList = lWall->GetEffectList();

				// Apply feature effects to the actor
				pActor->ApplyEffects(lWallEffectList,
					mSimulationTime, pDuration,
					TRUE, lDirectionAngle,
					lContactBottom, lContactTop, *track);

				// Apply actor effects to the feature
				// lDirectionAngle = MR_NORMALIZE_ANGLE( lDirectionAngle+MR_PI );
				// lWall->ApplyEffects( lActorEffectList, mSimulationTime, pDuration, TRUE, lDirectionAngle );
			}
		}
	}

}

Level *GameSession::GetCurrentLevel() const
{
	return track->GetLevel();
}

const char *GameSession::GetTitle() const
{
	return mTitle.c_str();
}

}  // namespace Model
}  // namespace HoverRace
