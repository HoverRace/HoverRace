// Missile.cpp
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

#include "Missile.h"
#include "ObjFac1Res.h"
#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"

using HoverRace::ObjFacTools::ResourceLib;
using namespace HoverRace::VideoServices;

namespace HoverRace {
namespace ObjFac1 {

namespace {

const MR_Int32 cMissileRay = 300;  ///< The missile have a diameter of 60cm.
const MR_Int32 cMissileWeight = 100;
const MR_Int32 cLifeTime = 7500;  // 7.5 sec
const MR_Int32 cStopTime = 1200;  // 1.2 sec
const MR_Int32 cIgnitionTime = 175;  // 0.175 sec

}  // namespace

#define TIME_SLICE                     5
#define MINIMUM_SPLITTABLE_TIME_SLICE  6
const double eSteadySpeed = 21.0 * 2222.0 / 1000.0;

MR_Int32 Missile::ZMin() const
{
	return mPosition.mZ - cMissileRay;
}

MR_Int32 Missile::ZMax() const
{
	return mPosition.mZ + cMissileRay;
}

MR_Int32 Missile::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 Missile::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 Missile::RayLen() const
{
	return cMissileRay;
}

Missile::Missile(const Util::ObjectFromFactoryId &pId,
	ResourceLib &resourceLib) :
	SUPER(pId),
	mHoverId(-1), mLived(0), mXSpeed(0), mYSpeed(0),
	mBounceSoundEvent(false),
	mBounceSound(resourceLib.GetShortSound(MR_SND_MISSILE_BOUNCE)->GetSound()),
	mMotorSound(resourceLib.
		GetContinuousSound(MR_SND_MISSILE_MOTOR)->GetSound())
{
	mEffectList.push_back(&mCollisionEffect);
	mEffectList.push_back(&mLostOfControlEffect);
	mActor = resourceLib.GetActor(MR_MISSILE);

	mLostOfControlEffect.mType = Model::LostOfControl::eMissile;
	mLostOfControlEffect.mElementId = -1;
	mLostOfControlEffect.mHoverId = mHoverId;
}

void Missile::SetOwnerId(int pHoverId)
{
	mHoverId = pHoverId;
	mLostOfControlEffect.mHoverId = mHoverId;
}

const Model::ContactEffectList *Missile::GetEffectList()
{

	if(mLived > cIgnitionTime) {
		mCollisionEffect.mWeight = cMissileWeight;
		mCollisionEffect.mXSpeed = static_cast<int>(mXSpeed * 256);
		mCollisionEffect.mYSpeed = static_cast<int>(mYSpeed * 256);
		mCollisionEffect.mZSpeed = 0;

		return &mEffectList;
	}
	else {
		return nullptr;
	}
}

// Simulation
int Missile::Simulate(MR_SimulationTime pDuration,
	Model::Level *pLevel, int pRoom)
{

	// Do the simulation
	while(pDuration > 0) {
		if(pDuration > TIME_SLICE) {
			pRoom = InternalSimulate(TIME_SLICE, pLevel, pRoom);
		}
		else {
			pRoom = InternalSimulate(pDuration, pLevel, pRoom);
		}
		pDuration -= TIME_SLICE;
	}

	// Determine the frame that must be displayed

	if(mLived > cLifeTime) {
		int lNbFrame = mActor->GetFrameCount(2);

		mCurrentSequence = 2;

		mCurrentFrame = lNbFrame * (mLived - cLifeTime) / cStopTime;

		if(mCurrentFrame >= lNbFrame) {
			mCurrentFrame = lNbFrame - 1;
		}

	}
	else if(mLived < (cIgnitionTime * 3)) {
		mCurrentSequence = 0;

		mCurrentFrame = mActor->GetFrameCount(mCurrentSequence) * mLived / (cIgnitionTime * 3);

	}
	else {
		mCurrentSequence = 1;

		mCurrentFrame = (mLived / 256) % 2;
	}

	// Determined if the object should be deleted
	if(mLived >= (cLifeTime + cStopTime)) {
		pRoom = Model::Level::eMustBeDeleted;
	}

	return pRoom;
}

int Missile::InternalSimulate(MR_SimulationTime pDuration,
	Model::Level *pLevel, int pRoom)
{

	mLived += pDuration;

	// MotorEffect
	double lSpeed = eSteadySpeed;

	if(mLived > cLifeTime) {
		lSpeed = (cStopTime - mLived - cLifeTime) * eSteadySpeed / cStopTime;

		if(lSpeed < 0) {
			lSpeed = 0;
		}
	}
	// Debug patch (stop the missile for observation
	/*
	   if( mLived > 1000 )
	   {
	   lSpeed = 0;
	   mLived = 1200;
	   }
	 */

	// Determine new dispacement

	Model::Cylinder lShape;

	lShape.mRayLen = 1;							  //cMissileRay;
	lShape.mZMin = mPosition.mZ - cMissileRay;
	lShape.mZMax = mPosition.mZ + cMissileRay;

	// Compute speed objectives
	MR_2DCoordinate lTranslation;

	mXSpeed = lSpeed * MR_Cos[mOrientation] / MR_TRIGO_FRACT;
	mYSpeed = lSpeed * MR_Sin[mOrientation] / MR_TRIGO_FRACT;

	lTranslation.mX = static_cast<int>(pDuration * mXSpeed);
	lTranslation.mY = static_cast<int>(pDuration * mYSpeed);

	// Verify if the move is valid
	Model::ObstacleCollisionReport lReport;

	BOOL lSuccessfullTry;
	MR_SimulationTime lDuration = pDuration;

	lShape.mAxis.mX = mPosition.mX + lTranslation.mX;
	lShape.mAxis.mY = mPosition.mY + lTranslation.mY;

	for (;;) {
		lSuccessfullTry = FALSE;

		lReport.GetContactWithObstacles(pLevel, &lShape, pRoom, this);

		if(lReport.IsInMaze()) {
			if(!lReport.HaveContact()) {
				mPosition.mX = lShape.mAxis.mX;
				mPosition.mY = lShape.mAxis.mY;
				pRoom = lReport.Room();

				lSuccessfullTry = TRUE;
			}
		}

		if(lDuration < MINIMUM_SPLITTABLE_TIME_SLICE) {
			break;
		}
		else {
			if(lSuccessfullTry) {
				if(lDuration == pDuration) {
					break;						  // success on first attempt
				}

				lDuration /= 2;

				lShape.mAxis.mX += lDuration * lTranslation.mX / pDuration;
				lShape.mAxis.mY += lDuration * lTranslation.mY / pDuration;
			}
			else {
				lDuration /= 2;

				lShape.mAxis.mX -= lDuration * lTranslation.mX / pDuration;
				lShape.mAxis.mY -= lDuration * lTranslation.mY / pDuration;
			}
		}

	}

	return pRoom;
}

void Missile::ApplyEffect(const Model::ContactEffect *pEffect,
	MR_SimulationTime, MR_SimulationTime,
	BOOL pValidDirection, MR_Angle pHorizontalDirection,
	MR_Int32, MR_Int32, Model::Level*)
{
	using namespace HoverRace::Model;

	const PhysicalCollision *lPhysCollision =
		dynamic_cast<const PhysicalCollision*>(pEffect);

	if (lPhysCollision && pValidDirection) {

		if(lPhysCollision->mWeight < PhysicalCollision::eInfiniteWeight) {
			if(mLived >= cIgnitionTime) {
				// Hitted a free object
				// Must died
				mLived = cLifeTime + cStopTime;
			}
		}
		else {
			// Hitted structure..make a perfect bounce
			MR_Angle lDiff = MR_NORMALIZE_ANGLE(pHorizontalDirection - mOrientation + MR_PI);

			if((lDiff < (MR_PI / 2)) || (lDiff > (MR_PI + MR_PI / 2))) {
				mOrientation = MR_NORMALIZE_ANGLE(pHorizontalDirection + lDiff);
				mBounceSoundEvent = true;
			}
		}
	}
}

// State broadcast

namespace {

struct MissileState
{
	MR_Int32 mPosX;							  // 4    4
	MR_Int32 mPosY;							  // 4    8
	MR_Int32 mPosZ;							  // 4   12

	MR_Angle mOrientation;					  // 2   14
	MR_Int8 mHoverId;						  // 1   16
};

}  // namespace

Model::ElementNetState Missile::GetNetState() const
{
	static MissileState lsState;				  // Static is ok because the variable will be used immediatly

	Model::ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8 *) & lsState;

	lsState.mPosX = mPosition.mX;
	lsState.mPosY = mPosition.mY;
	lsState.mPosZ = mPosition.mZ;

	lsState.mOrientation = mOrientation;

	lsState.mHoverId = static_cast<MR_Int8>(mHoverId);

	return lReturnValue;
}

void Missile::SetNetState(int pDataLen, const MR_UInt8 *pData)
{
	const MissileState *lState = (const MissileState *) pData;

	mPosition.mX = lState->mPosX;
	mPosition.mY = lState->mPosY;
	mPosition.mZ = lState->mPosZ;

	mOrientation = lState->mOrientation;

	if (static_cast<unsigned>(pDataLen) >= sizeof(MissileState)) {
		mHoverId = lState->mHoverId;
		mLostOfControlEffect.mHoverId = mHoverId;
	}

}

void Missile::PlayExternalSounds(int pDB, int pPan)
{
	if(mBounceSoundEvent) {
		mBounceSoundEvent = false;

		SoundServer::Play(mBounceSound, pDB, 1.0, pPan);
	}

	SoundServer::Play(mMotorSound, 1, pDB, 1.0, pPan);
}

}  // namespace ObjFac1
}  // namespace HoverRace
