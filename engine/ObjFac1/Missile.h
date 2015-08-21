
// Missile.h
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

#pragma once

#include "../ObjFacTools/FreeElementBase.h"
#include "../Model/RaceEffects.h"
#include "../Model/PhysicalCollision.h"

namespace HoverRace {
namespace ObjFac1 {

class Missile :
	public ObjFacTools::FreeElementBase,
	protected Model::CylinderShape
{
	using SUPER = ObjFacTools::FreeElementBase;

public:
	Missile(ObjFacTools::ResourceLib &resourceLib);
	~Missile() { }

protected:
	// Shape interface
	MR_Int32 ZMin() const override;
	MR_Int32 ZMax() const override;
	MR_Int32 AxisX() const override;
	MR_Int32 AxisY() const override;
	MR_Int32 RayLen() const override;

protected:
	// Init interface
	void SetOwnerId(int pOwner) override;

	// ContactEffectShapeInterface
	const Model::ContactEffectList *GetEffectList() override;
	const Model::ShapeInterface *GetGivingContactEffectShape() override { return this; }
	const Model::ShapeInterface *GetReceivingContactEffectShape() override { return this; }

	int Simulate(MR_SimulationTime pTimeSlice,
		Model::Track &track, int pRoom) override;
	int InternalSimulate(MR_SimulationTime pDuration,
		Model::Track &track, int pRoom);

	void ApplyEffect(const Model::ContactEffect *pEffect,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax,
		Model::Track &pLevel) override;

	// Network state
	Model::ElementNetState GetNetState() const override;
	void SetNetState(int pDataLen, const MR_UInt8 *pData) override;

	// Sounds
	void PlayExternalSounds(int pDB, int pPan) override;

private:
	int mHoverId;
	MR_SimulationTime mLived;
	Model::PhysicalCollision mCollisionEffect;
	Model::LostOfControl mLostOfControlEffect;
	Model::ContactEffectList mEffectList;

	double mXSpeed;
	double mYSpeed;

	bool mBounceSoundEvent;
	VideoServices::ShortSound *mBounceSound;
	VideoServices::ContinuousSound *mMotorSound;
};

}  // namespace ObjFac1
}  // namespace HoverRace
