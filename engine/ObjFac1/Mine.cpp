
// Mine.cpp
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
#include "../Model/Track.h"
#include "../ObjFacTools/ResourceLib.h"
#include "../Parcel/ResBundle.h"
#include "../Util/Config.h"
#include "ObjFac1Res.h"

#include "Mine.h"

using namespace HoverRace::Util;
using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

namespace {

const MR_Int32 cMineRay = 400;
const MR_Int32 cMineHeight = 140;

}  // namespace

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

Mine::Mine() :
	SUPER({ 1, 151 }),
	mOnGround(false)
{
	auto &resLib = Config::GetInstance()->GetResBundle().GetResourceLib();

	mEffectList.push_back(&mEffect);
	mActor = resLib.GetActor(MR_MINE);

	mOrientation = 0;

	mEffect.mType = Model::LostOfControl::eMine;
	mEffect.mElementId = -1;
	mEffect.mHoverId = -1;
}

Mine::~Mine()
{
}

bool Mine::AssignPermNumber(int pNumber)
{
	mEffect.mElementId = pNumber;
	return true;
}

const Model::ContactEffectList *Mine::GetEffectList()
{
	if(mOnGround) {
		return &mEffectList;
	}
	else {
		return NULL;
	}
}

const Model::ShapeInterface *Mine::GetReceivingContactEffectShape()
{
	return this;
}

const Model::ShapeInterface *Mine::GetGivingContactEffectShape()
{
	// return this;
	return NULL;
}

// Simulation
int Mine::Simulate(MR_SimulationTime pDuration, Model::Track &track, int pRoom)
{
	if(pRoom == -1) {
		mOnGround = false;
	}
	else if(!mOnGround && (pDuration > 0)) {
		// FreeFall computation

		Model::ObstacleCollisionReport lReport;

		mPosition.mZ -= static_cast<int>(pDuration * 0.6);

		lReport.GetContactWithObstacles(track.GetLevel(), this, pRoom, this);

		if(!lReport.IsInMaze()) {
			ASSERT(FALSE);
			mOnGround = true;
		}
		else {
			if(lReport.HaveContact()) {
				int lStepHeight = lReport.StepHeight();
				ASSERT(lStepHeight >= 0);
				ASSERT(lStepHeight <= 1000);

				mOnGround = true;
				mPosition.mZ += lStepHeight;
			}
		}
	}
	return pRoom;
}

void Mine::Render(VideoServices::Viewport3D *pDest, MR_SimulationTime pTime)
{
	mCurrentFrame = (pTime >> 9) & 1;
	SUPER::Render(pDest, pTime);
}

// State broadcast

struct MineState
{
	MR_Int32 mPosX;							  // 4    4
	MR_Int32 mPosY;							  // 4    8
	MR_Int32 mPosZ;							  // 4   12
};

Model::ElementNetState Mine::GetNetState() const
{
	static MineState lsState;  // Static is ok because the variable will be used immediatly

	Model::ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8 *) & lsState;

	lsState.mPosX = mPosition.mX;
	lsState.mPosY = mPosition.mY;
	lsState.mPosZ = mPosition.mZ;

	return lReturnValue;
}

void Mine::SetNetState(int, const MR_UInt8 *pData)
{
	const MineState *lState = (const MineState *) pData;

	mPosition.mX = lState->mPosX;
	mPosition.mY = lState->mPosY;
	mPosition.mZ = lState->mPosZ;

	mOrientation = 0;
}

}  // namespace ObjFac1
}  // namespace HoverRace
