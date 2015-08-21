
// Mine.h
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

namespace HoverRace {
namespace ObjFac1 {

class Mine :
	public ObjFacTools::FreeElementBase,
	protected Model::CylinderShape
{
	using SUPER = ObjFacTools::FreeElementBase;

public:
	Mine(ObjFacTools::ResourceLib &resourceLib);
	~Mine();

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

	int Simulate(MR_SimulationTime pTimeSlice,
		Model::Track &track, int pRoom) override;

	void Render(VideoServices::Viewport3D *pDest,
		MR_SimulationTime pTime) override;

	// Network state
	Model::ElementNetState GetNetState() const override;
	void SetNetState(int pDataLen, const MR_UInt8 *pData) override;

	bool AssignPermNumber(int pNumber) override;

private:
	bool mOnGround;
	Model::LostOfControl mEffect;
	Model::ContactEffectList mEffectList;
};

}  // namespace ObjFac1
}  // namespace HoverRace
