
// TestElement.h
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

//
// This element is used to test and build new FreeElements
//

#pragma once

#include "../ObjFac1/ObjFac1Res.h"
#include "../ObjFacTools/FreeElementBase.h"
#include "../Model/PhysicalCollision.h"

namespace HoverRace {
namespace ObjFac1 {

class TestElement : public ObjFacTools::FreeElementBase
{
	using SUPER = ObjFacTools::FreeElementBase;

	struct Cylinder : public Model::CylinderShape
	{
		MR_Int32 mRay;
		MR_3DCoordinate mPosition;

		MR_Int32 ZMin() const override { return mPosition.mZ; }
		MR_Int32 ZMax() const override { return mPosition.mZ + 1800; }
		MR_Int32 AxisX() const override { return mPosition.mX; }
		MR_Int32 AxisY() const override { return mPosition.mY; }
		MR_Int32 RayLen() const override { return mRay; }
	};

public:
	TestElement(ObjFacTools::ResourceLib &resourceLib, int pActorRes);
	~TestElement() { }

	int Simulate(MR_SimulationTime pDuration, Model::Track &track,
		int pRoom) override;

protected:

	// ContactEffectShapeInterface
	void ApplyEffect(const Model::ContactEffect *pEffect,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax, Model::Track &track) override;

	const Model::ContactEffectList *GetEffectList() override;

	const Model::ShapeInterface *GetObstacleShape() override;
	const Model::ShapeInterface *GetReceivingContactEffectShape() override;
	const Model::ShapeInterface *GetGivingContactEffectShape() override;

private:
	MR_SimulationTime mElapsedFrameTime;

	// Logic part (Always present.. who cares it is a test
	int mXSpeed;
	int mYSpeed;

	Cylinder mCollisionShape;
	Cylinder mContactShape;

	Model::PhysicalCollision mContactEffect;
	Model::ContactEffectList mContactEffectList;
};

}  // namespace ObjFac1
}  // namespace HoverRace
