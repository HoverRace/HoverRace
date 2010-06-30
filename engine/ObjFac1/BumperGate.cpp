// BumperGate.cpp
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

#include "BumperGate.h"
#include "ObjFac1Res.h"
#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"

using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

#define NB_STATE 10

const MR_Int32 cGateRayMin = 200;
const MR_Int32 cGateRayMax = 2500;

const MR_Int32 cGateHeightMin = 1500;
const MR_Int32 cGateHeightMax = 3000;

const MR_Int32 cGateWeight = MR_PhysicalCollision::eInfiniteWeight;

MR_Int32 BumperGate::ZMin() const
{
	return mPosition.mZ + 2;					  // the 2 reduce computing because the shape dont touch the floor
}

MR_Int32 BumperGate::ZMax() const
{
	return mPosition.mZ + cGateHeightMin + mCurrentFrame * (cGateHeightMax - cGateHeightMin) / (mLastState);
}

MR_Int32 BumperGate::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 BumperGate::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 BumperGate::RayLen() const
{
	return cGateRayMin + mCurrentFrame * (cGateRayMax - cGateRayMin) / (mLastState);
}

BumperGate::BumperGate(const MR_ObjectFromFactoryId & pId, ResourceLib* resourceLib)
	: MR_FreeElementBase(pId)
{
	mActor = resourceLib->GetActor(MR_BUMPERGATE);

	mTimeSinceLastCollision = +1000000;
	mLastState = mActor->GetFrameCount(0) - 1;

	mCurrentFrame = mLastState;
	mCurrentSequence = 0;

	mEffectList.push_back(&mCollisionEffect);

}

BumperGate::~BumperGate()
{
}

const MR_ContactEffectList *BumperGate::GetEffectList()
{

	mCollisionEffect.mWeight = cGateWeight;
	mCollisionEffect.mXSpeed = 0;
	mCollisionEffect.mYSpeed = 0;
	mCollisionEffect.mZSpeed = 0;

	return &mEffectList;
}

const MR_ShapeInterface *BumperGate::GetReceivingContactEffectShape()
{
	return this;
}

const MR_ShapeInterface *BumperGate::GetGivingContactEffectShape()
{
	// return this;
	// ASSERT( FALSE );
	return NULL;
}

// Simulation
int BumperGate::Simulate(MR_SimulationTime pDuration, Model::Level * pLevel, int pRoom)
{

	if(pDuration >= 0) {
		mTimeSinceLastCollision += pDuration;

		if(mTimeSinceLastCollision < 1500) {
			mCurrentFrame = (1500 - mTimeSinceLastCollision) * mLastState / 1500;
		}
		else if(mTimeSinceLastCollision < 9000) {
			mCurrentFrame = 0;
		}
		else if(mTimeSinceLastCollision < 13000) {
			mCurrentFrame = (mTimeSinceLastCollision - 9000) * mLastState / 4000;
		}
		else {
			mCurrentFrame = mLastState;
		}
	}

	return pRoom;
}

void BumperGate::ApplyEffect(const MR_ContactEffect * pEffect, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 /*pZMin */ , MR_Int32 /*pZMax */ , Model::Level * /*pLevel */ )
{
	MR_ContactEffect *lEffect = (MR_ContactEffect *) pEffect;
	const MR_PhysicalCollision *lPhysCollision = dynamic_cast < MR_PhysicalCollision * >(lEffect);

	if(lPhysCollision != NULL) {
		if(mCurrentFrame >= mLastState) {
			mTimeSinceLastCollision = 0;
		}
		else {
			mTimeSinceLastCollision = 1500 - 1500 * mCurrentFrame / mLastState;
		}
	}
}

}  // namespace ObjFac1
}  // namespace HoverRace
