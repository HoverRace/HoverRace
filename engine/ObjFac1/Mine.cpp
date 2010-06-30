// Mine.cpp
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

#include "StdAfx.h"

#include "Mine.h"
#include "ObjFac1Res.h"
#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"

using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

const MR_Int32 cMineRay = 400;
const MR_Int32 cMineHeight = 140;

MR_Int32 Mine::ZMin() const
{
	return mPosition.mZ;
}

MR_Int32 Mine::ZMax() const
{
	return mPosition.mZ + cMineHeight;
}

MR_Int32 Mine::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 Mine::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 Mine::RayLen() const
{
	return cMineRay;
}

Mine::Mine(const MR_ObjectFromFactoryId & pId, ResourceLib* resourceLib) :
	MR_FreeElementBase(pId)
{
	mEffectList.push_back(&mEffect);
	mActor = resourceLib->GetActor(MR_MINE);

	mOnGround = FALSE;
	mOrientation = 0;

	mEffect.mType = MR_LostOfControl::eMine;
	mEffect.mElementId = -1;
	mEffect.mHoverId = -1;
}

Mine::~Mine()
{
}

BOOL Mine::AssignPermNumber(int pNumber)
{
	mEffect.mElementId = pNumber;
	return TRUE;
}

const MR_ContactEffectList *Mine::GetEffectList()
{
	if(mOnGround) {
		return &mEffectList;
	}
	else {
		return NULL;
	}
}

const MR_ShapeInterface *Mine::GetReceivingContactEffectShape()
{
	return this;
}

const MR_ShapeInterface *Mine::GetGivingContactEffectShape()
{
	// return this;
	return NULL;
}

// Simulation
int Mine::Simulate(MR_SimulationTime pDuration, MR_Level * pLevel, int pRoom)
{
	if(pRoom == -1) {
		mOnGround = FALSE;
	}
	else if(!mOnGround && (pDuration > 0)) {
		// FreeFall computation

		Model::ObstacleCollisionReport lReport;

		mPosition.mZ -= static_cast<int>(pDuration * 0.6);

		lReport.GetContactWithObstacles(pLevel, this, pRoom, this);

		if(!lReport.IsInMaze()) {
			ASSERT(FALSE);
			mOnGround = TRUE;
		}
		else {
			if(lReport.HaveContact()) {
				int lStepHeight = lReport.StepHeight();
				ASSERT(lStepHeight >= 0);
				ASSERT(lStepHeight <= 1000);

				mOnGround = TRUE;
				mPosition.mZ += lStepHeight;
			}
		}
	}
	return pRoom;
}

void Mine::Render(MR_3DViewPort * pDest, MR_SimulationTime pTime)
{
	mCurrentFrame = (pTime >> 9) & 1;
	MR_FreeElementBase::Render(pDest, pTime);
}

// State broadcast

class MineState
{
	public:
		MR_Int32 mPosX;							  // 4    4
		MR_Int32 mPosY;							  // 4    8
		MR_Int32 mPosZ;							  // 4   12

};

MR_ElementNetState Mine::GetNetState() const
{
	static MineState lsState;				  // Static is ok because the variable will be used immediatly

	MR_ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8 *) & lsState;

	lsState.mPosX = mPosition.mX;
	lsState.mPosY = mPosition.mY;
	lsState.mPosZ = mPosition.mZ;

	return lReturnValue;
}

void Mine::SetNetState(int /*pDataLen */ , const MR_UInt8 * pData)
{

	const MineState *lState = (const MineState *) pData;

	mPosition.mX = lState->mPosX;
	mPosition.mY = lState->mPosY;
	mPosition.mZ = lState->mPosZ;

	mOrientation = 0;

}

}  // namespace ObjFac1
}  // namespace HoverRace
