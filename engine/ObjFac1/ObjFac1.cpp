
// ObjFac1.cpp
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

#include "DefaultSurface.h"
#include "WoodSurface.h"
#include "BallElement.h"
#include "TestElement.h"
#include "GenericRenderer.h"
#include "FuelSource.h"
#include "DoubleSpeedSource.h"
#include "FinishLine.h"
#include "HoverRender.h"
#include "Missile.h"
#include "BumperGate.h"
#include "PowerUp.h"
#include "Mine.h"
#include "ObjFac1Res.h"
#include "../ObjFacTools/SpriteHandle.h"
#include "../Util/Config.h"

#include "ObjFac1.h"

using namespace HoverRace::Util;
using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

ObjFac1::ObjFac1() :
	resourceLib(new ResourceLib(Config::GetInstance()->GetMediaPath("ObjFac1.dat")))
{
}

ObjFac1::~ObjFac1()
{
}

std::shared_ptr<ObjectFromFactory> ObjFac1::GetObject(int pClassId)
{
	if (pClassId < 1 || pClassId > 65535) {
		return nullptr;
	}

	ObjectFromFactoryId lId = { 1, static_cast<MR_UInt16>(pClassId) };

	switch (pClassId) {
		case 1:
			return std::make_shared<DefaultSurface>(lId, *resourceLib);

		case 2:
			return std::make_shared<WoodSurface>(lId, *resourceLib);

		case 10:
			return std::make_shared<TestElement>(lId, *resourceLib, MR_DEMO_FIGHTER);

		case 12:
			return std::make_shared<BallElement>(lId, *resourceLib);

		case 13:
			return std::make_shared<TestElement>(lId, *resourceLib, MR_ELECTRO_CAR);

		case 50:
			return std::make_shared<BitmapSurface>(lId, nullptr);

		case 51:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_STD_FLOOR));

		case 52:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_STD_RIGHT_WALL), 4000);

		case 53:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_STD_LEFT_WALL), 4000);

		case 54:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_RED_RIGHT_WALL_OFF), resourceLib->GetBitmap(MR_RED_RIGHT_WALL), 200, 4, 4000);

		case 55:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_RED_LEFT_WALL_OFF), resourceLib->GetBitmap(MR_RED_LEFT_WALL), -200, 4, 4000);

		case 56:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_GREEN_RIGHT_WALL_OFF), resourceLib->GetBitmap(MR_GREEN_RIGHT_WALL), 200, 4, 4000);

		case 57:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_GREEN_LEFT_WALL_OFF), resourceLib->GetBitmap(MR_GREEN_LEFT_WALL), -200, 4, 4000);

		case 58:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_STEP_WALL));

		case 59:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_PASS_RIGHT_WALL));

		case 60:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_PASS_LEFT_WALL));

		case 61:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_DO_NOT_ENTER_WALL1));

		case 62:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_DO_NOT_ENTER_WALL2));

		case 63:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_BLUE_BUBBLE_FLOOR));

		case 64:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_SPEED_ZONE));

		case 65:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_FUEL_ZONE));

		case 66:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_YELLOW_STEP));

		case 67:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_CHECKER));

		case 68:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_PIT_WORD));

		case 69:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_FINISH_WORD));

		case 70:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_YELLOW_NEON), resourceLib->GetBitmap(MR_YELLOW_NEON_FLASH), 50, 20);

		case 71:
			return std::make_shared<BitmapSurface>(lId, resourceLib->GetBitmap(MR_YELLOW_NEON), resourceLib->GetBitmap(MR_YELLOW_NEON_FLASH), -50, 20);

		case 72:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_STD_WALL), 4000);

		case 73:
			return std::make_shared<VStretchBitmapSurface>(lId, resourceLib->GetBitmap(MR_STD_WALL_TOP), 6000);

		case 100:
			return std::make_shared<HoverRender>(lId, *resourceLib);

		case 150:
			return std::make_shared<Missile>(lId, *resourceLib);

		case 151:
			return std::make_shared<Mine>(lId, *resourceLib);

		case 152:
			return std::make_shared<PowerUp>(lId, *resourceLib);

		case 170:
			return std::make_shared<BumperGate>(lId, *resourceLib);

		case 200:
			return std::make_shared<FuelSource>(lId);

		case 201:
			return std::make_shared<DoubleSpeedSource>(lId);

		case 202:
			return std::make_shared<FinishLine>(lId, Model::CheckPoint::eFinishLine);

		case 203:
			return std::make_shared<FinishLine>(lId, Model::CheckPoint::eCheck1);

		case 204:
			return std::make_shared<FinishLine>(lId, Model::CheckPoint::eCheck2);

		case 1000:
			return std::make_shared<SpriteHandle>(lId, resourceLib->GetSprite(MR_FONT1));

		case 1100:
			return std::make_shared<SpriteHandle>(lId, resourceLib->GetSprite(MR_MISSILE_STAT));

		case 1101:
			return std::make_shared<SpriteHandle>(lId, resourceLib->GetSprite(MR_HOVER_ICONS));

		case 1102:
			return std::make_shared<SpriteHandle>(lId, resourceLib->GetSprite(MR_MINE_STAT));

		case 1103:
			return std::make_shared<SpriteHandle>(lId, resourceLib->GetSprite(MR_PWRUP_STAT));
	}

	return std::shared_ptr<ObjectFromFactory>();
}

} // namespace ObjFac1
} // namespace HoverRace
