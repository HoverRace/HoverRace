
// BumperGate.h
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

class BumperGate :
	public ObjFacTools::FreeElementBase,
	protected Model::CylinderShape
{
	using SUPER = ObjFacTools::FreeElementBase;

public:
	BumperGate(ObjFacTools::ResourceLib &resourceLib);
	~BumperGate() { }

protected:
	// Shape interface
	MR_Int32 ZMin() const override;
	MR_Int32 ZMax() const override;
	MR_Int32 AxisX() const override;
	MR_Int32 AxisY() const override;
	MR_Int32 RayLen() const override;

protected:
	// ContactEffectShapeInterface
	const Model::ContactEffectList *GetEffectList() override;
	const Model::ShapeInterface *GetGivingContactEffectShape() override;
	const Model::ShapeInterface *GetReceivingContactEffectShape() override;

	int Simulate(MR_SimulationTime pTimeSlice, Model::Track &track,
		int pRoom) override;

	void ApplyEffect(const Model::ContactEffect *pEffect,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax, Model::Track &track) override;

private:
	MR_SimulationTime mTimeSinceLastCollision;
	int mLastState;
	Model::PhysicalCollision mCollisionEffect;
	Model::ContactEffectList mEffectList;

};

}  // namespace ObjFac1
}  // namespace HoverRace
