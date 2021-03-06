
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

#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"
#include "../ObjFacTools/ResourceLib.h"
#include "ObjFac1Res.h"

#include "BumperGate.h"

using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

namespace {

const MR_Int32 cGateRayMin = 200;
const MR_Int32 cGateRayMax = 2500;

const MR_Int32 cGateHeightMin = 1500;
const MR_Int32 cGateHeightMax = 3000;

const MR_Int32 cGateWeight = Model::PhysicalCollision::eInfiniteWeight;

}  // namespace


BumperGate::BumperGate(ResourceLib &resourceLib) :
	SUPER({ 1, 170 })
{
	mActor = resourceLib.GetActor(MR_BUMPERGATE);

	mTimeSinceLastCollision = +1000000;
	mLastState = mActor->GetFrameCount(0) - 1;

	mCurrentFrame = mLastState;
	mCurrentSequence = 0;

	mEffectList.push_back(&mCollisionEffect);
}

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

const Model::ContactEffectList *BumperGate::GetEffectList()
{
	mCollisionEffect.mWeight = cGateWeight;
	mCollisionEffect.mXSpeed = 0;
	mCollisionEffect.mYSpeed = 0;
	mCollisionEffect.mZSpeed = 0;

	return &mEffectList;
}

const Model::ShapeInterface *BumperGate::GetReceivingContactEffectShape()
{
	return this;
}

const Model::ShapeInterface *BumperGate::GetGivingContactEffectShape()
{
	return nullptr;
}

// Simulation
int BumperGate::Simulate(MR_SimulationTime pDuration, Model::Track&, int pRoom)
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

void BumperGate::ApplyEffect(const Model::ContactEffect *pEffect,
	MR_SimulationTime, MR_SimulationTime,
	BOOL, MR_Angle,
	MR_Int32, MR_Int32, Model::Track&)
{
	using namespace Model;

	const PhysicalCollision *lPhysCollision =
		dynamic_cast<const PhysicalCollision*>(pEffect);

	if (lPhysCollision) {
		if (mCurrentFrame >= mLastState) {
			mTimeSinceLastCollision = 0;
		}
		else {
			mTimeSinceLastCollision = 1500 - 1500 * mCurrentFrame / mLastState;
		}
	}
}

}  // namespace ObjFac1
}  // namespace HoverRace
