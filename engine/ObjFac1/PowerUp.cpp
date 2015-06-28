// PowerUp.cpp
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

#include "PowerUp.h"
#include "ObjFac1Res.h"
#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"

using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

namespace {

const MR_Int32 cPowerUpRay = 550;
const MR_Int32 cPowerUpHalfHeight = 550;

}  // namespace


PowerUp::PowerUp(const Util::ObjectFromFactoryId &pId,
	ResourceLib &resourceLib) :
	SUPER(pId)
{
	mEffectList.push_back(&mPowerUpEffect);
	mActor = resourceLib.GetActor(MR_PWRUP);

	mOrientation = 0;
	mPowerUpEffect.mElementPermId = -1;
}

MR_Int32 PowerUp::ZMin() const
{
	return mPosition.mZ - cPowerUpHalfHeight;
}

MR_Int32 PowerUp::ZMax() const
{
	return mPosition.mZ + cPowerUpHalfHeight;
}

MR_Int32 PowerUp::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 PowerUp::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 PowerUp::RayLen() const
{
	return cPowerUpRay;
}

BOOL PowerUp::AssignPermNumber(int pNumber)
{
	mPowerUpEffect.mElementPermId = pNumber;
	return TRUE;
}

const Model::ContactEffectList *PowerUp::GetEffectList()
{
	return &mEffectList;
}

const Model::ShapeInterface *PowerUp::GetReceivingContactEffectShape()
{
	return this;
}

const Model::ShapeInterface *PowerUp::GetGivingContactEffectShape()
{
	return nullptr;
}

// Simulation
int PowerUp::Simulate(MR_SimulationTime pDuration, Model::Track&, int pRoom)
{
	// Just rotate on ourself
	if(pDuration != 0) {
		mOrientation = MR_NORMALIZE_ANGLE(mOrientation + pDuration);
	}

	return pRoom;
}

// State broadcast

struct MR_PowerUpState
{
	MR_Int32 mPosX;							  // 4    4
	MR_Int32 mPosY;							  // 4    8
	MR_Int32 mPosZ;							  // 4   12
};

Model::ElementNetState PowerUp::GetNetState() const
{
	static MR_PowerUpState lsState;				  // Static is ok because the variable will be used immediatly

	Model::ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8*)&lsState;

	lsState.mPosX = mPosition.mX;
	lsState.mPosY = mPosition.mY;
	lsState.mPosZ = mPosition.mZ;

	return lReturnValue;
}

void PowerUp::SetNetState(int, const MR_UInt8 *pData)
{
	const MR_PowerUpState *lState = (const MR_PowerUpState*)pData;

	mPosition.mX = lState->mPosX;
	mPosition.mY = lState->mPosY;
	mPosition.mZ = lState->mPosZ;

	mOrientation = 0;

}

}  // namespace ObjFac1
}  // namespace HoverRace
