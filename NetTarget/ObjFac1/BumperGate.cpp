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

#include "stdafx.h"

#include "BumperGate.h"
#include "ObjFac1Res.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../Model/ConcreteShape.h"
#include "../Model/FreeElementMovingHelper.h"
#include "../MainCharacter/MainCharacter.h"

#define NB_STATE 10

const MR_Int32 cGateRayMin = 200;
const MR_Int32 cGateRayMax = 2500;

const MR_Int32 cGateHeightMin = 1500;
const MR_Int32 cGateHeightMax = 3000;

const MR_Int32 cGateWeight = MR_PhysicalCollision::eInfiniteWeight;

int MR_BumperGate::GetHoverSlot(int pHoverId) const
{
	if((pHoverId >= 0) && (pHoverId < MR_MAX_LOCAL_PLAYER)) {
		return pHoverId;
	}
	return eStateCount - 1;
}

void MR_BumperGate::UpdateFrameForSlot(int pSlot)
{
	if(mTimeSinceLastCollision[pSlot] < 1500) {
		mFrameByHover[pSlot] = (1500 - mTimeSinceLastCollision[pSlot]) * mLastState / 1500;
	}
	else if(mTimeSinceLastCollision[pSlot] < 9000) {
		mFrameByHover[pSlot] = 0;
	}
	else if(mTimeSinceLastCollision[pSlot] < 13000) {
		mFrameByHover[pSlot] = (mTimeSinceLastCollision[pSlot] - 9000) * mLastState / 4000;
	}
	else {
		mFrameByHover[pSlot] = mLastState;
	}
}

void MR_BumperGate::SelectHoverSlot(int pHoverId)
{
	mActiveHoverSlot = GetHoverSlot(pHoverId);
	mCurrentFrame = mFrameByHover[mActiveHoverSlot];
}

MR_Int32 MR_BumperGate::ZMin() const
{
	return mPosition.mZ + 2;					  // the 2 reduce computing because the shape dont touch the floor
}

MR_Int32 MR_BumperGate::ZMax() const
{
	return mPosition.mZ + cGateHeightMin + mCurrentFrame * (cGateHeightMax - cGateHeightMin) / (mLastState);
}

MR_Int32 MR_BumperGate::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 MR_BumperGate::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 MR_BumperGate::RayLen() const
{
	return cGateRayMin + mCurrentFrame * (cGateRayMax - cGateRayMin) / (mLastState);
} MR_BumperGate::MR_BumperGate(const MR_ObjectFromFactoryId & pId)
:MR_FreeElementBase(pId)
{
	mActor = gObjectFactoryData->mResourceLib.GetActor(MR_BUMPERGATE);
	mLastState = mActor->GetFrameCount(0) - 1;
	mActiveHoverSlot = eStateCount - 1;
	mCurrentSequence = 0;
	for(int lCounter = 0; lCounter < eStateCount; lCounter++) {
		mTimeSinceLastCollision[lCounter] = +1000000;
		mFrameByHover[lCounter] = mLastState;
	}
	mCurrentFrame = mFrameByHover[mActiveHoverSlot];

	mEffectList.AddTail(&mCollisionEffect);

}

MR_BumperGate::~MR_BumperGate()
{
}

const MR_ContactEffectList *MR_BumperGate::GetEffectList()
{

	mCollisionEffect.mWeight = cGateWeight;
	mCollisionEffect.mXSpeed = 0;
	mCollisionEffect.mYSpeed = 0;
	mCollisionEffect.mZSpeed = 0;
	mCollisionEffect.mHoverId = -1;

	return &mEffectList;
}

const MR_ShapeInterface *MR_BumperGate::GetReceivingContactEffectShape()
{
	return this;
}

const MR_ShapeInterface *MR_BumperGate::GetGivingContactEffectShape()
{
	// return this;
	// ASSERT( FALSE );
	return NULL;
}

void MR_BumperGate::Render(MR_3DViewPort * pDest, MR_SimulationTime pTime)
{
	SelectHoverSlot(pDest->GetViewingHoverId());
	MR_FreeElementBase::Render(pDest, pTime);
}

// Simulation
int MR_BumperGate::Simulate(MR_SimulationTime pDuration, MR_Level * pLevel, int pRoom)
{

	if(pDuration >= 0) {
		for(int lCounter = 0; lCounter < eStateCount; lCounter++) {
			mTimeSinceLastCollision[lCounter] += pDuration;
			UpdateFrameForSlot(lCounter);
		}
	}

	mCurrentFrame = mFrameByHover[mActiveHoverSlot];

	return pRoom;
}

void MR_BumperGate::ApplyEffect(const MR_ContactEffect * pEffect, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 /*pZMin */ , MR_Int32 /*pZMax */ , MR_Level * /*pLevel */ )
{
	MR_ContactEffect *lEffect = (MR_ContactEffect *) pEffect;
	const MR_PhysicalCollision *lPhysCollision = dynamic_cast < MR_PhysicalCollision * >(lEffect);

	if(lPhysCollision != NULL) {
		const int lSourceHoverId = mActiveHoverSlot;

		if(mFrameByHover[lSourceHoverId] >= mLastState) {
			mTimeSinceLastCollision[lSourceHoverId] = 0;
		}
		else {
			mTimeSinceLastCollision[lSourceHoverId] =
				1500 - 1500 * mFrameByHover[lSourceHoverId] / mLastState;
		}
		UpdateFrameForSlot(lSourceHoverId);

		if(MR_MainCharacter::IsHoverColumnInteractionTracked(lSourceHoverId) &&
			MR_MainCharacter::IsHoverColumnInteractionEnabled(lSourceHoverId))
		{
			for(int lHoverId = 0; lHoverId < MR_MAX_LOCAL_PLAYER; lHoverId++) {
				if((lHoverId == lSourceHoverId) ||
					!MR_MainCharacter::IsHoverColumnInteractionTracked(lHoverId) ||
					!MR_MainCharacter::IsHoverColumnInteractionEnabled(lHoverId))
				{
					continue;
				}

				mTimeSinceLastCollision[lHoverId] =
					mTimeSinceLastCollision[lSourceHoverId];
				UpdateFrameForSlot(lHoverId);
			}
		}

		mCurrentFrame = mFrameByHover[mActiveHoverSlot];
	}
}

void MR_BumperGate::SetContactHoverId(int pHoverId)
{
	SelectHoverSlot(pHoverId);
}
