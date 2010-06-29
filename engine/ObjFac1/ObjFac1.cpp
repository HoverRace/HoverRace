// ObjFac1/main.cpp
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

#include "stdafx.h"

#include "DefaultSurface.h"
#include "WoodSurface.h"
#include "BallElement.h"
//#include "BabeElement.h"
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

using namespace HoverRace::ObjFac1;
using HoverRace::ObjFacTools::ResourceLib;
using HoverRace::Util::Config;

ObjFac1::ObjFac1()
{
	Config *cfg = Config::GetInstance();
	resourceLib = new ResourceLib(cfg->GetMediaPath("ObjFac1.dat"));
}

ObjFac1::~ObjFac1()
{
	delete resourceLib;
}

MR_ObjectFromFactory *ObjFac1::GetObject(int pClassId)
{
	MR_ObjectFromFactory *lReturnValue = NULL;
	MR_ObjectFromFactoryId lId = { 1, pClassId };

	switch (pClassId) {
		case 1:
			lReturnValue = new MR_DefaultSurface(lId, resourceLib);
			break;

		case 2:
			lReturnValue = new MR_WoodSurface(lId, resourceLib);
			break;

		case 3:
			//         lReturnValue = new MR_BitmapSurface( lId, resourceLib->GetBitmap( MR_FIRE ) );
			break;

		case 4:
			//         lReturnValue = new MR_BitmapSurface( lId, resourceLib->GetBitmap( MR_BIG_BRICK ) );
			break;

		case 5:
			//         lReturnValue = new MR_BitmapSurface( lId, resourceLib->GetBitmap( MR_RED_BRICK ) );
			break;

		case 10:
			lReturnValue = new MR_TestElement(lId, resourceLib, MR_DEMO_FIGHTER);
			break;

		case 11:
			//         lReturnValue = new MR_BabeElement( lId );
			break;

		case 12:
			lReturnValue = new MR_BallElement(lId, resourceLib);
			break;

		case 13:
			lReturnValue = new MR_TestElement(lId, resourceLib, MR_ELECTRO_CAR);
			break;

		case 50:
			lReturnValue = new MR_BitmapSurface(lId, NULL);
			break;

		case 51:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_STD_FLOOR));
			break;

		case 52:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_STD_RIGHT_WALL), 4000);
			break;

		case 53:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_STD_LEFT_WALL), 4000);
			break;

		case 54:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_RED_RIGHT_WALL_OFF), resourceLib->GetBitmap(MR_RED_RIGHT_WALL), 200, 4, 4000);
			break;

		case 55:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_RED_LEFT_WALL_OFF), resourceLib->GetBitmap(MR_RED_LEFT_WALL), -200, 4, 4000);
			break;

		case 56:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_GREEN_RIGHT_WALL_OFF), resourceLib->GetBitmap(MR_GREEN_RIGHT_WALL), 200, 4, 4000);
			break;

		case 57:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_GREEN_LEFT_WALL_OFF), resourceLib->GetBitmap(MR_GREEN_LEFT_WALL), -200, 4, 4000);
			break;

		case 58:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_STEP_WALL));
			break;

		case 59:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_PASS_RIGHT_WALL));
			break;

		case 60:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_PASS_LEFT_WALL));
			break;

		case 61:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_DO_NOT_ENTER_WALL1));
			break;

		case 62:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_DO_NOT_ENTER_WALL2));
			break;

		case 63:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_BLUE_BUBBLE_FLOOR));
			break;

		case 64:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_SPEED_ZONE));
			break;

		case 65:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_FUEL_ZONE));
			break;

		case 66:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_YELLOW_STEP));
			break;

		case 67:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_CHECKER));
			break;

		case 68:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_PIT_WORD));
			break;

		case 69:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_FINISH_WORD));
			break;

		case 70:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_YELLOW_NEON), resourceLib->GetBitmap(MR_YELLOW_NEON_FLASH), 50, 20);
			break;

		case 71:
			lReturnValue = new MR_BitmapSurface(lId, resourceLib->GetBitmap(MR_YELLOW_NEON), resourceLib->GetBitmap(MR_YELLOW_NEON_FLASH), -50, 20);
			break;

		case 72:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_STD_WALL), 4000);
			break;

		case 73:
			lReturnValue = new MR_VStretchBitmapSurface(lId, resourceLib->GetBitmap(MR_STD_WALL_TOP), 6000);
			break;

		case 100:
			lReturnValue = new HoverRender(lId, resourceLib);
			break;

		case 150:
			lReturnValue = new MR_Missile(lId, resourceLib);
			break;

		case 151:
			lReturnValue = new MR_Mine(lId, resourceLib);
			break;

		case 152:
			lReturnValue = new MR_PowerUp(lId, resourceLib);
			break;

		case 170:
			lReturnValue = new MR_BumperGate(lId, resourceLib);
			break;

		case 200:
			lReturnValue = new MR_FuelSource(lId);
			break;

		case 201:
			lReturnValue = new MR_DoubleSpeedSource(lId);
			break;

		case 202:
			lReturnValue = new MR_FinishLine(lId, MR_CheckPoint::eFinishLine);
			break;

		case 203:
			lReturnValue = new MR_FinishLine(lId, MR_CheckPoint::eCheck1);
			break;

		case 204:
			lReturnValue = new MR_FinishLine(lId, MR_CheckPoint::eCheck2);
			break;

		case 1000:
			lReturnValue = new MR_SpriteHandle(lId, resourceLib->GetSprite(MR_FONT1));
			break;

		case 1100:
			lReturnValue = new MR_SpriteHandle(lId, resourceLib->GetSprite(MR_MISSILE_STAT));
			break;

		case 1101:
			lReturnValue = new MR_SpriteHandle(lId, resourceLib->GetSprite(MR_HOVER_ICONS));
			break;

		case 1102:
			lReturnValue = new MR_SpriteHandle(lId, resourceLib->GetSprite(MR_MINE_STAT));
			break;

		case 1103:
			lReturnValue = new MR_SpriteHandle(lId, resourceLib->GetSprite(MR_PWRUP_STAT));
			break;

	}

	return lReturnValue;
}
