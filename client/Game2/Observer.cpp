// Observer.cpp
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

#include "../../engine/VideoServices/VideoBuffer.h"
#include "ClientSession.h"
#include "Observer.h"
#include "../../engine/Model/Level.h"
#include "../../engine/Model/MazeElement.h"
#include "../../engine/Util/Profiler.h"
#include "../../engine/Util/Config.h"
#include "resource.h"
#include "../../engine/VideoServices/StaticText.h"

#include <math.h>

using HoverRace::Util::Config;
using HoverRace::VideoServices::StaticText;

#define NB_PLAYER_PAGE 10
#define MR_CHAT_EXPIRATION     20

static const std::string &GetCraftName(int id)
{
	static const std::string names[4] = {
		std::string(_("Basic craft")),
		std::string(_("CX craft")),
		std::string(_("Bi-Turbo craft")),
		std::string(_("Eon craft")),
		};
	static const std::string unknown = std::string(_("Unknown craft"));
	return (id >= 0 && id < 4) ? names[id] : unknown;
}

namespace HoverRace {
namespace Client {

namespace {
	// Global table of formatted HUD strings.
	struct
	{
		std::string rankTitle;
		std::string hitTitle;

		std::string firstLap;
		std::string chartFinish;
		std::string chart;
		std::string hitChart;
		std::string countdown;
		std::string finish;
		std::string finishSingle;
		std::string bestLap;
		std::string header;
		std::string lastLap;
		std::string curLap;

		// This is initialized once via Observer's constructor.
		// We have to defer the initialization until first use because the
		// locale isn't set until the app is initialized.
		void Init()
		{
			static bool initialized = false;
			if (initialized) return;

			std::string s;

			//TODO: This is the original hard-coded spacing.
			//      The revamped HUD will render these bits separately.
			s = _("Rank"); s += "            "; s += _("Result"); s += "   "; s += _("Best Lap");
			rankTitle = Ascii2Simple(s.c_str());
			s = _("Rank"); s += "                 "; s += _("For"); s += "   "; s += _("Against");
			hitTitle = Ascii2Simple(s.c_str());

			firstLap = _("%d %-0.10s #%d%*s   --First lap--  %c");
			chartFinish = _("%d %-0.10s #%d%*s %2d.%02d.%03d %2d.%02d.%03d%c");
			chart = _("%d %-0.10s #%d%*s lap:%-2d   %2d.%02d.%03d%c");
			hitChart = _("%d %-0.10s #%d%*s  %2d      %2d");
			countdown = _("Starting in %02d.%02d sec. for %d laps");
			finish = _("Finished in %d.%02d.%03d, placed %d of %d");
			finishSingle = _("Finished in %d.%02d.%03d");
			bestLap = _("Best lap %d.%02d.%03d");
			header = _("%d.%02d.%02d  Lap:%d/%d");
			lastLap = _("Last lap %d.%02d.%03d  Best %d.%02d.%03d");
			curLap = _("Current lap %d.%02d.%03d  Best %d.%02d.%03d");

			initialized = true;
		}
	} globalFmts;
}

Observer::Observer()
{
	const Config *cfg = Config::GetInstance();

	globalFmts.Init();

	mLastCameraPosValid = FALSE;
	mScroll = 0;
	mApperture = MR_PI / 2;

	mXMargin_1024 = 0;
	mYMargin_1024 = 0;

	mMoreMessages = FALSE;
	mDispPlayers = 1;

	mSplitMode = eNotSplit;

	mCockpitView = FALSE;

	Util::ObjectFromFactoryId lBaseFontId = { 1, 1000 };
	mBaseFont = (ObjFac1::SpriteHandle *) Util::DllObjectFactory::CreateObject(lBaseFontId);

	Util::ObjectFromFactoryId lMissileLevelId = { 1, 1100 };
	mMissileLevel = (ObjFac1::SpriteHandle *) Util::DllObjectFactory::CreateObject(lMissileLevelId);

	Util::ObjectFromFactoryId lMineDispId = { 1, 1102 };
	mMineDisp = (ObjFac1::SpriteHandle *) Util::DllObjectFactory::CreateObject(lMineDispId);

	Util::ObjectFromFactoryId lPowerUpDispId = { 1, 1103 };
	mPowerUpDisp = (ObjFac1::SpriteHandle *) Util::DllObjectFactory::CreateObject(lPowerUpDispId);

	Util::ObjectFromFactoryId lHoverIconsId = { 1, 1101 };
	mHoverIcons = (ObjFac1::SpriteHandle *) Util::DllObjectFactory::CreateObject(lHoverIconsId);

	std::string selectStr("<==   ");
	selectStr += _("Select your craft with the arrow keys");
	selectStr += "   ==>";
	selectCraftTxt = new StaticText(selectStr, cfg->GetDefaultFontName(), 16,
		true, true, 0x2c, StaticText::EFFECT_SHADOW);

	craftTxt = new StaticText("", cfg->GetDefaultFontName(), 20, true, false,
		0x47, StaticText::EFFECT_SHADOW);
}

Observer::~Observer()
{
	delete craftTxt;
	delete selectCraftTxt;

	delete mBaseFont;
	delete mMissileLevel;
	delete mMineDisp;
	delete mPowerUpDisp;
	delete mHoverIcons;
}

Observer *Observer::New()
{
	return new Observer;
}

void Observer::Delete()
{
	delete this;
}

void Observer::SetCockpitView(BOOL pOn)
{
	mCockpitView = pOn;
}

void Observer::Scroll(int pOffset)
{
	mScroll += pOffset;

	if(mScroll != 0)
		mScroll = 0;

	//if( mScroll < -0 )
	//{
	// mScroll = -0;
	//}
	//else if( mScroll > 1 )
	//{
	//   mScroll = 0;
	//}
}

void Observer::ZoomIn()
{
	if(mApperture > MR_PI / 10) {
		mApperture = MR_Angle((mApperture * 4) / 5);
	}
}

void Observer::ZoomOut()
{
	if(mApperture < 3 * MR_PI / 4) {
		mApperture = MR_Angle(mApperture + mApperture / 4);
	}
}

void Observer::Home()
{
	mScroll = 0;
	mApperture = MR_PI / 2;
}

void Observer::EnlargeMargin()
{
	if(mXMargin_1024 < 400) {
		mXMargin_1024 += 64;
	}

	if(mYMargin_1024 < 400) {
		mYMargin_1024 += 64;
	}
}

void Observer::ReduceMargin()
{
	mXMargin_1024 -= 64;
	mYMargin_1024 -= 64;

	if(mXMargin_1024 < 0) {
		mXMargin_1024 = 0;
	}

	if(mYMargin_1024 < 0) {
		mYMargin_1024 = 0;
	}
}

void Observer::SetSplitMode(eSplitMode pMode)
{
	mSplitMode = pMode;
}

void Observer::MoreMessages()
{
	if(mDispPlayers != 0) {
		mDispPlayers = 0;
		mMoreMessages = TRUE;
	}
	else {
		mMoreMessages = !mMoreMessages;
	}
}

void Observer::PlayersListPageDn()
{
	if(mDispPlayers == 0) {
		// mMoreMessages = FALSE;
	}
	mDispPlayers++;
}

// Rendering functions
void Observer::Render2DDebugView(MR_VideoBuffer * pDest, const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter)
{
	// WARNING Calculations are done using floats..it is only a debug view

	// Draw a surface of 100m x 60m centered on the character

	// Initialize the viewport
	int lXRes = m2DDebugView.GetXRes();
	int lYRes = m2DDebugView.GetYRes();

	// Compute the scaling factor and the origin
												  // lXRes/100000.0; // 1m == 2mm on screen
	double lXScaling = pDest->GetXPixelMeter() / (500.0 * 1000.0);
												  //-lYRes/60000.0;
	double lYScaling = -pDest->GetYPixelMeter() / (500.0 * 1000.0);

	MR_3DCoordinate lCharacterPos = pViewingCharacter->mPosition;
	MR_Angle lOrientation = pViewingCharacter->mOrientation;
	int lRoom = pViewingCharacter->mRoom;

	m2DDebugView.Clear();
	// pDest->Clear(); // Faster

	int lRoomCount;
	const int *lRoomList = pLevel->GetVisibleZones(lRoom, lRoomCount);

	// Draw each room of the maze
	for(int lRoomId = 0; lRoomId < pLevel->GetRoomCount(); lRoomId++) {
		MR_PolygonShape *lSectionShape = pLevel->GetRoomShape(lRoomId);

		// Determine the Drawing color
		MR_UInt8 lColor = 6;

		if(lRoomId == lRoom) {
			lColor = 7;
		}
		else {
			// verify if the zone is visible

			for(int lCounter = 0; lCounter < lRoomCount; lCounter++) {
				if(lRoomList[lCounter] == lRoomId) {
					lColor = 9;
					break;
				}
			}
		}

		// Draw the contour

		int lVertexCount = lSectionShape->VertexCount();

		MR_Int32 lX0;
		MR_Int32 lY0;
		MR_Int32 lX1;
		MR_Int32 lY1;

		lX1 = (MR_Int32) ((lSectionShape->X(lVertexCount - 1) - lCharacterPos.mX) * lXScaling) + lXRes / 2;
		lY1 = (MR_Int32) ((lSectionShape->Y(lVertexCount - 1) - lCharacterPos.mY) * lYScaling) + lYRes / 2;

		for(int lVertex = 0; lVertex < lVertexCount; lVertex++) {
			lX0 = (MR_Int32) ((lSectionShape->X(lVertex) - lCharacterPos.mX) * lXScaling) + lXRes / 2;
			lY0 = (MR_Int32) ((lSectionShape->Y(lVertex) - lCharacterPos.mY) * lYScaling) + lYRes / 2;

			m2DDebugView.DrawLine(lX0, lY0, lX1, lY1, lColor);

			lX1 = lX0;
			lY1 = lY0;
		}

		// Draw the FreeElements

		delete lSectionShape;
	}

	// Draw the main character
	int lXSideLen = int (1000.0 * lXScaling);
	int lYSideLen = int (1000.0 * lYScaling);

	m2DDebugView.DrawLine((-lXSideLen + lXRes) / 2, (lYSideLen + lYRes) / 2, (lXSideLen + lXRes) / 2, (lYSideLen + lYRes) / 2, 9);

	m2DDebugView.DrawLine((-lXSideLen + lXRes) / 2, (-lYSideLen + lYRes) / 2, (lXSideLen + lXRes) / 2, (-lYSideLen + lYRes) / 2, 9);

	m2DDebugView.DrawLine((lXSideLen + lXRes) / 2, (-lYSideLen + lYRes) / 2, (lXSideLen + lXRes) / 2, (lYSideLen + lYRes) / 2, 9);

	m2DDebugView.DrawLine((-lXSideLen + lXRes) / 2, (-lYSideLen + lYRes) / 2, (-lXSideLen + lXRes) / 2, (lYSideLen + lYRes) / 2, 9);

	m2DDebugView.DrawLine(lXRes / 2, lYRes / 2, ((MR_Cos[lOrientation] * 4 * lXSideLen / MR_TRIGO_FRACT) + lXRes) / 2, ((MR_Sin[lOrientation] * 4 * lYSideLen / MR_TRIGO_FRACT) + lYRes) / 2, 9);

}

void Observer::RenderWireFrameView(const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter)
{

	mWireFrameView.Clear(0);
	// mWireFrameView.ClearZ( );

	MR_3DCoordinate lCharacterPos = pViewingCharacter->mPosition;
	MR_Angle lOrientation = pViewingCharacter->mOrientation;
	int lRoom = pViewingCharacter->mRoom;

	lCharacterPos.mZ += 1800;					  // Fix the eyes at 1m80

	mWireFrameView.SetupCameraPosition(lCharacterPos, lOrientation, mScroll);

	// Draw the walls and features of the visibles rooms
	int lRoomCount;
	const int *lRoomList = pLevel->GetVisibleZones(lRoom, lRoomCount);

	for(int lCounter = -1; lCounter < lRoomCount; lCounter++) {
		int lRoomId;
		MR_UInt8 lColor = 7;

		if(lCounter == -1) {
			lRoomId = lRoom;
		}
		else {
			lRoomId = lRoomList[lCounter];
		}

		// Draw the room and all the features
		for(int lCounter2 = -1; lCounter2 < pLevel->GetFeatureCount(lRoom); lCounter2++) {

			Model::SectionId lSectionId;

			if(lCounter2 == -1) {
				lSectionId.mType = Model::SectionId::eRoom;
				lSectionId.mId = lRoom;
			}
			else {
				lSectionId.mType = Model::SectionId::eFeature;
				lSectionId.mId = pLevel->GetFeature(lRoom, lCounter2);
			}
			DrawWFSection(pLevel, lSectionId, lColor);
		}
	}

}

void Observer::DrawWFSection(const Model::Level * pLevel, const Model::SectionId & pSectionId, MR_UInt8 pColor)
{
	MR_PolygonShape *lSectionShape;

	if(pSectionId.mType == Model::SectionId::eRoom) {
		lSectionShape = pLevel->GetRoomShape(pSectionId.mId);
	}
	else {
		lSectionShape = pLevel->GetFeatureShape(pSectionId.mId);
	}

	// Draw the contour

	int lVertexCount = lSectionShape->VertexCount();
	MR_3DCoordinate lP0;
	MR_3DCoordinate lP1;

	lP1.mX = lSectionShape->X(lVertexCount - 1);
	lP1.mY = lSectionShape->Y(lVertexCount - 1);

	for(int lVertex = 0; lVertex < lVertexCount; lVertex++) {
		MR_3DCoordinate lP0Top;

		lP0.mX = lSectionShape->X(lVertex);
		lP0.mY = lSectionShape->Y(lVertex);

		lP0.mZ = lSectionShape->ZMax();
		lP1.mZ = lP0.mZ;
		lP0Top = lP0;

		mWireFrameView.DrawWFLine(lP0, lP1, pColor);

		lP0.mZ = lSectionShape->ZMin();
		lP1.mZ = lP0.mZ;

		mWireFrameView.DrawWFLine(lP0, lP1, pColor);
		mWireFrameView.DrawWFLine(lP0, lP0Top, pColor);

		lP1 = lP0;
	}

	delete lSectionShape;

}

void Observer::Render3DView(const ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage)
{

	const Model::Level *lLevel = pSession->GetCurrentLevel();

	MR_3DCoordinate lCameraPos;
	MR_Angle lOrientation = pViewingCharacter->mOrientation;
	int lRoom = pViewingCharacter->mRoom;
	double lAbsSpeedRatio = pViewingCharacter->GetAbsoluteSpeed();

	if(mCockpitView) {
		lOrientation = pViewingCharacter->GetCabinOrientation();
		lCameraPos.mX = pViewingCharacter->mPosition.mX - 256 * MR_Cos[lOrientation] / MR_TRIGO_FRACT;
		lCameraPos.mY = pViewingCharacter->mPosition.mY - 256 * MR_Sin[lOrientation] / MR_TRIGO_FRACT;
		lCameraPos.mZ = pViewingCharacter->mPosition.mZ + 1050;
	}
	else {
		int lDist = 3400;

		lOrientation = pViewingCharacter->mOrientation;

		if(pTime < -3000) {
			int lFactor = (-pTime - 3000) * 2 / 3;
			lOrientation = MR_NORMALIZE_ANGLE(lOrientation + lFactor * 4096 / 11000);
			lDist += lFactor;
		}

		lCameraPos.mX = pViewingCharacter->mPosition.mX - lDist * MR_Cos[lOrientation] / MR_TRIGO_FRACT;
		lCameraPos.mY = pViewingCharacter->mPosition.mY - lDist * MR_Sin[lOrientation] / MR_TRIGO_FRACT;
		lCameraPos.mZ = pViewingCharacter->mPosition.mZ + 1700;

		if(mLastCameraPosValid) {
			lCameraPos.mX = (3 * lCameraPos.mX + mLastCameraPos.mX) / 4;
			lCameraPos.mY = (3 * lCameraPos.mY + mLastCameraPos.mY) / 4;
			lCameraPos.mZ = (2 * lCameraPos.mZ + mLastCameraPos.mZ) / 3;
		}
	}

	mLastCameraPos = lCameraPos;
	mLastCameraPosValid = TRUE;

	m3DView.SetupCameraPosition(lCameraPos, lOrientation, mScroll);

	MR_SAMPLE_START(Clear, "ClearScreen");

	// Clear background
	if(pBackImage == NULL) {
		m3DView.Clear(0);						  // Will have to be replace by a bitmapped background
	}
	else {
		m3DView.RenderBackground(pBackImage);
	}

	MR_SAMPLE_END(Clear);
	MR_SAMPLE_START(ClearZ, "ClearZScreen");

	m3DView.ClearZ();

	MR_SAMPLE_END(ClearZ);

	int lCounter;

	// Floor and ceiling drawing
	MR_SAMPLE_START(FloorRendering, "Floor Rendering");

	int lTotalSections = lLevel->GetNbVisibleSurface(lRoom);
	const Model::SectionId *lFloorList = lLevel->GetVisibleFloorList(lRoom);
	const Model::SectionId *lCeilingList = lLevel->GetVisibleCeilingList(lRoom);

	for(lCounter = 0; lCounter < lTotalSections; lCounter++) {
		// Draw the floor
		RenderFloorOrCeiling(lLevel, lFloorList[lCounter], TRUE, pTime);

		// Render the ceiling
		RenderFloorOrCeiling(lLevel, lCeilingList[lCounter], FALSE, pTime);

	}

	MR_SAMPLE_END(FloorRendering);

	// Draw the walls and features of the visibles rooms
	MR_SAMPLE_START(WallRendering, "Wall Rendering");

	int lRoomCount;
	const int *lRoomList = lLevel->GetVisibleZones(lRoom, lRoomCount);

	for(lCounter = -1; lCounter < lRoomCount; lCounter++) {
		int lRoomId;

		if(lCounter == -1) {
			lRoomId = lRoom;
		}
		else {
			lRoomId = lRoomList[lCounter];
		}

		// Draw all the features

		int lNbFeature = lLevel->GetFeatureCount(lRoomId);

		for(int lCounter2 = 0; lCounter2 < lNbFeature; lCounter2++) {
			RenderFeatureWalls(lLevel, lLevel->GetFeature(lRoomId, lCounter2), pTime);
		}

		RenderRoomWalls(lLevel, lRoomId, pTime);
	}

	MR_SAMPLE_END(WallRendering);

	// Draw all the elements of the visibles room
	MR_SAMPLE_START(ActorRendering, "Actor Rendering");

	for(lCounter = -1; lCounter < lRoomCount; lCounter++) {
		int lRoomId;

		if(lCounter == -1) {
			lRoomId = lRoom;
		}
		else {
			lRoomId = lRoomList[lCounter];
		}

		MR_FreeElementHandle lHandle = lLevel->GetFirstFreeElement(lRoomId);

		while(lHandle != NULL) {
			Model::FreeElement *lElement = Model::Level::GetFreeElement(lHandle);

			lElement->Render(&m3DView, pTime);

			lHandle = Model::Level::GetNextFreeElement(lHandle);
		}
	}

	// Display cockpit
	int lXRes = m3DView.GetXRes();
	int lYRes = m3DView.GetYRes();

	int lSpeedMeterLen = lXRes / 2;
	int lFuelMeterLen = lXRes / 4;
	int lMeterHight = lYRes / 32;
	int lXMargin = lXRes / 32;
	int lYMargin = lMeterHight;

	int lAbsSpeedLen = (int)(lAbsSpeedRatio * lSpeedMeterLen);
	int lDirSpeedLen = (int)(pViewingCharacter->GetDirectionalSpeed() * lSpeedMeterLen);
	double lFuelLevel = pViewingCharacter->GetFuelLevel();
	int lFuelLen = (int)(lFuelLevel * lFuelMeterLen);

	m3DView.DrawHorizontalMeter(lXMargin, lSpeedMeterLen, lYMargin, lMeterHight, lAbsSpeedLen, 54, 56);
	if(lDirSpeedLen < 0) {
		m3DView.DrawHorizontalMeter(lXMargin, lSpeedMeterLen, lYMargin + lMeterHight, lMeterHight, -lDirSpeedLen, 44, 56);
	}
	else {
		m3DView.DrawHorizontalMeter(lXMargin, lSpeedMeterLen, lYMargin + lMeterHight, lMeterHight, lDirSpeedLen, 54, 56);
	}
	m3DView.DrawHorizontalMeter(lXRes - lXMargin - lFuelMeterLen, lFuelMeterLen, lYMargin, lMeterHight * 2, lFuelLen, 54, (lFuelLevel < 0.20) ? 35 : 56);

	// MissileLevel
	ObjFac1::SpriteHandle *lWeaponSprite = NULL;
	int lWeaponSpriteIndex = 0;

	if(pViewingCharacter->GetCurrentWeapon() == MainCharacter::MainCharacter::eMissile) {
		lWeaponSprite = mMissileLevel;
		lWeaponSpriteIndex = pViewingCharacter->GetMissileRefillLevel(mMissileLevel->GetSprite()->GetNbItem());
	}
	else if(pViewingCharacter->GetCurrentWeapon() == MainCharacter::MainCharacter::eMine) {
		lWeaponSprite = mMineDisp;
		lWeaponSpriteIndex = pViewingCharacter->GetMineCount();

		if(lWeaponSpriteIndex > 0) {
			lWeaponSpriteIndex = ((lWeaponSpriteIndex - 1) * 2) + 1;
			if((pTime >> 9) & 1) {
				lWeaponSpriteIndex++;
			}
		}
	}
	else if(pViewingCharacter->GetCurrentWeapon() == MainCharacter::MainCharacter::ePowerUp) {
		lWeaponSprite = mPowerUpDisp;
		lWeaponSpriteIndex = pViewingCharacter->GetPowerUpFraction(4);
		if(lWeaponSpriteIndex == 0) {
			lWeaponSpriteIndex = pViewingCharacter->GetPowerUpCount();
		}
		else {
			lWeaponSpriteIndex = 9 - lWeaponSpriteIndex;
		}

	}

	if(lWeaponSprite != NULL) {
		int lMissileScaling = 1 + (310 / lXRes);

		lWeaponSprite->GetSprite()->Blt(lXRes, lYRes / 16, &m3DView, MR_Sprite::eRight, MR_Sprite::eTop, lWeaponSpriteIndex, lMissileScaling);
	}
	// Map

	if(pSession->GetMap() != NULL) {
		const MR_Sprite *lHoverIcons = mHoverIcons->GetSprite();

		int lMapScaling = 1 + (3 * pSession->GetMap()->GetItemHeight() / lYRes);
		int lIconsScaling = 1 + (14 * lHoverIcons->GetItemHeight() / lYRes);
		int lMargin = lHoverIcons->GetItemHeight() / lIconsScaling;

		pSession->GetMap()->Blt(lMargin, lMargin, &m3DView, MR_Sprite::eLeft, MR_Sprite::eTop, 0, lMapScaling);

		// display all the car icons
		int lNbPlayers = pSession->GetNbPlayers();
		int lNbIcons = lHoverIcons->GetNbItem();

		for(int lCounter = 0; (lCounter < 20) && (lNbPlayers > 0); lCounter++) {
			int lX;
			int lY;

			const MainCharacter::MainCharacter *lPlayer = pSession->GetPlayer(lCounter);

			if(lPlayer != NULL) {
				lNbPlayers--;

				lX = lPlayer->mPosition.mX;
				lY = lPlayer->mPosition.mY;

				pSession->ConvertMapCoordinate(lX, lY, lMapScaling);

				lHoverIcons->Blt(lX + lMargin, lY + lMargin, &m3DView, MR_Sprite::eCenter, MR_Sprite::eCenter, (lPlayer->GetHoverId()) % lNbIcons, lIconsScaling);
			}
		}
	}
	// Print text

	if(mBaseFont != NULL) {
		char lStrBuffer[170];

		const MR_Sprite *lFont = mBaseFont->GetSprite();

		// Display chat messages
		int lFontScaling = 1 + (lFont->GetItemHeight() * 30) / (lYRes);
		int lLineSpacing = lFont->GetItemHeight() / lFontScaling;

		int lXMargin = 2 * lFont->GetItemWidth() / lFontScaling;
		int lYMargin = lYRes - lYRes / 15 - lLineSpacing;
		int lPrintLen = (int)((lXRes - lXMargin) * 3.9 / (lFont->GetItemWidth() * 3 / lFontScaling));
		// subject to div/0

		pSession->GetCurrentMessage(lStrBuffer);

		if(lStrBuffer[0] != 0) {
			lFont->StrBlt(lXMargin, lYMargin, lStrBuffer, &m3DView, MR_Sprite::eLeft, MR_Sprite::eTop, lFontScaling);
		}
		lYMargin -= lLineSpacing;

		int lMaxDepth = (mMoreMessages && (mDispPlayers == 0)) ? 10 : 6;
		int lMessageLife = (mMoreMessages && (mDispPlayers == 0)) ? MR_CHAT_EXPIRATION * 4 : MR_CHAT_EXPIRATION;

		int lStackLevel = 0;
		int lLineLevel = 0;
		while(pSession->GetMessageStack(lStackLevel++, lStrBuffer, lMessageLife) && (lLineLevel < lMaxDepth) && (lYMargin > lYRes / 4)) {
			int lStrLen = strlen(lStrBuffer);

			if(lStrLen > lPrintLen) {
				lFont->StrBlt(lXMargin, lYMargin, lStrBuffer + lPrintLen, &m3DView, MR_Sprite::eLeft, MR_Sprite::eTop, lFontScaling);
				lYMargin -= lLineSpacing;
				lStrBuffer[lPrintLen] = 0;
				lLineLevel++;
			}
			lFont->StrBlt(lXMargin, lYMargin, lStrBuffer, &m3DView, MR_Sprite::eLeft, MR_Sprite::eTop, lFontScaling);
			lYMargin -= lLineSpacing;
			lLineLevel++;
		}
	}

	if((mBaseFont != NULL) && mDispPlayers) {

		int lNbResultAvail = pSession->ResultAvaillable();
		int lNbPages = (lNbResultAvail + NB_PLAYER_PAGE - 1) / NB_PLAYER_PAGE;

		int lCurrentPage = mDispPlayers = mDispPlayers % (lNbPages * 2 + 1);

		if(mDispPlayers != 0) {
			BOOL lShowHits = FALSE;

			lCurrentPage--;

			if(lCurrentPage >= lNbPages) {
				lShowHits = TRUE;
				lCurrentPage -= lNbPages;
			}
			// Display rank list
			int lFirstPlayer = lCurrentPage * NB_PLAYER_PAGE;
			int lLastPlayer = min(lFirstPlayer + NB_PLAYER_PAGE, lNbResultAvail);
			int lFontScaling = 1 + (mBaseFont->GetSprite()->GetItemHeight() * 30) / (lYRes);
			int lLineSpacing = mBaseFont->GetSprite()->GetItemHeight() / lFontScaling;

			int lCurrentLine = lYRes / 6;

			// Display banner

			if(lShowHits) {
				mBaseFont->GetSprite()->StrBlt(lXRes / 2, lCurrentLine, globalFmts.hitTitle.c_str(),
					&m3DView, MR_Sprite::eCenter, MR_Sprite::eTop, lFontScaling);
			}
			else {
				mBaseFont->GetSprite()->StrBlt(lXRes / 2, lCurrentLine, globalFmts.rankTitle.c_str(),
					&m3DView, MR_Sprite::eCenter, MR_Sprite::eTop, lFontScaling);
			}
			lCurrentLine += lLineSpacing;

			// Display list
			for(int lCounter = lFirstPlayer; lCounter < lLastPlayer; lCounter++) {
				char lBuffer[80];

				const char *lPlayerName;
				int lHoverId;
				BOOL lConnected;
				int lNbLap;
				MR_SimulationTime lFinishTime;
				MR_SimulationTime lBestLap;
				int lNbFor;
				int lNbAgain;

				if(lShowHits) {
					pSession->GetHitResult(lCounter, lPlayerName, lHoverId, lConnected, lNbFor, lNbAgain);
				}
				else {
					pSession->GetResult(lCounter, lPlayerName, lHoverId, lConnected, lNbLap, lFinishTime, lBestLap);
				}

				int lPlayerNameLen = min(strlen(lPlayerName), 10);

				if(lShowHits) {
					sprintf(lBuffer, globalFmts.hitChart.c_str(), lCounter + 1, lPlayerName, lHoverId + 1, 10 - lPlayerNameLen, "", lNbFor, lNbAgain);

				}
				else if(lNbLap == 0) {
					sprintf(lBuffer, globalFmts.firstLap.c_str(), lCounter + 1, lPlayerName, lHoverId + 1, 10 - lPlayerNameLen, "", lConnected ? '*' : '-');

				}
				else if(lNbLap == -1) {
					sprintf(lBuffer, globalFmts.chartFinish.c_str(), lCounter + 1, lPlayerName, lHoverId + 1, 10 - lPlayerNameLen, "", lFinishTime / 60000, (lFinishTime % 60000) / 1000, (lFinishTime % 1000), lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000), lConnected ? '*' : '-');

				}
				else {
					sprintf(lBuffer, globalFmts.chart.c_str(), lCounter + 1, lPlayerName, lHoverId + 1, 10 - lPlayerNameLen, "", lNbLap + 1, lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000), lConnected ? '*' : '-');

				}

				mBaseFont->GetSprite()->StrBlt(lXRes / 2, lCurrentLine, Ascii2Simple(lBuffer), &m3DView, MR_Sprite::eCenter, MR_Sprite::eTop, lFontScaling);
				lCurrentLine += lLineSpacing;
			}
		}
	}

	if(mBaseFont != NULL) {

		// Display timers
		char lMainLineBuffer[80];
		char lLapLineBuffer[80];

		lLapLineBuffer[0] = 0;

		if(pTime < 0) {
			pTime = -pTime;
			sprintf(lMainLineBuffer, globalFmts.countdown.c_str(), (pTime % 60000) / 1000, (pTime % 1000) / 10, pViewingCharacter->GetTotalLap());
			
			int lFontScaling = 1 + (mBaseFont->GetSprite()->GetItemHeight() * 30) / (lYRes);
			int lLineHeight = (mBaseFont->GetSprite()->GetItemHeight() / lFontScaling);

			selectCraftTxt->Blt(lXRes / 2, lYRes / 16 + lLineHeight, &m3DView, true);

			craftTxt->SetText(GetCraftName(pViewingCharacter->GetHoverModel()));
			craftTxt->Blt(lXRes / 2, lYRes / 16 + lLineHeight + selectCraftTxt->GetHeight(), &m3DView, true);
		}
		else if(pViewingCharacter->GetTotalLap() <= pViewingCharacter->GetLap()) {
			MR_SimulationTime lTotalTime = pViewingCharacter->GetTotalTime();
			MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();

			// Race is finish
			if(pSession->GetNbPlayers() > 1) {
				sprintf(lMainLineBuffer, globalFmts.finish.c_str(), lTotalTime / 60000, (lTotalTime % 60000) / 1000, (lTotalTime % 1000), pSession->GetRank(pViewingCharacter), pSession->GetNbPlayers());
			}
			else {
				sprintf(lMainLineBuffer, globalFmts.finishSingle.c_str(), lTotalTime / 60000, (lTotalTime % 60000) / 1000, (lTotalTime % 1000));
			}
			sprintf(lLapLineBuffer, globalFmts.bestLap.c_str(), lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000));

		}
		else if(pViewingCharacter->GetLap() == 0) {
			// First lap
			sprintf(lMainLineBuffer, globalFmts.header.c_str(), pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10, 1, pViewingCharacter->GetTotalLap());
			// sprintf( lLapLineBuffer, "Current lap %d.%02d.%02d", pTime/60000, (pTime%60000)/1000, (pTime%1000) );

		}
		else if(pViewingCharacter->GetLastLapCompletion() > (pTime - 8000)) {
			// Lap terminated less than 8 sec ago
			MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();
			MR_SimulationTime lLastLap = pViewingCharacter->GetLastLapDuration();

			// More than one lap completed
			sprintf(lMainLineBuffer, globalFmts.header.c_str(), pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10, pViewingCharacter->GetLap() + 1, pViewingCharacter->GetTotalLap());
			sprintf(lLapLineBuffer, globalFmts.lastLap.c_str(), lLastLap / 60000, (lLastLap % 60000) / 1000, (lLastLap % 1000), lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000));
		}
		else {
			MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();
			MR_SimulationTime lCurrentLap = pTime - pViewingCharacter->GetLastLapCompletion();

			// More than one lap completed
			sprintf(lMainLineBuffer, globalFmts.header.c_str(), pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10, pViewingCharacter->GetLap() + 1, pViewingCharacter->GetTotalLap());
			sprintf(lLapLineBuffer, globalFmts.curLap.c_str(), lCurrentLap / 60000, (lCurrentLap % 60000) / 1000, (lCurrentLap % 1000), lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000));

		}

		int lFontScaling = 1 + (mBaseFont->GetSprite()->GetItemHeight() * 30) / (lYRes);

		mBaseFont->GetSprite()->StrBlt(lXRes / 2, lYRes / 16, Ascii2Simple(lMainLineBuffer), &m3DView, MR_Sprite::eCenter, MR_Sprite::eTop, lFontScaling);
		mBaseFont->GetSprite()->StrBlt(lXRes / 2, lYRes - 1, Ascii2Simple(lLapLineBuffer), &m3DView, MR_Sprite::eCenter, MR_Sprite::eBottom, lFontScaling);

	}

	MR_SAMPLE_END(ActorRendering);

}

void Observer::RenderRoomWalls(const Model::Level * pLevel, int lRoomId, MR_SimulationTime pTime)
{
	MR_PolygonShape *lSectionShape = pLevel->GetRoomShape(lRoomId);

	int lVertexCount = lSectionShape->VertexCount();

	// Draw the walls
	MR_3DCoordinate lP0;
	MR_3DCoordinate lP1;

	MR_Int32 lFloorLevel = lSectionShape->ZMin();
	MR_Int32 lCeilingLevel = lSectionShape->ZMax();

	lP0.mX = lSectionShape->X(0);
	lP0.mY = lSectionShape->Y(0);

	for(int lVertex = 0; lVertex < lVertexCount; lVertex++) {
		int lNext = lVertex + 1;

		if(lNext == lVertexCount) {
			lNext = 0;
		}

		lP1.mX = lSectionShape->X(lNext);
		lP1.mY = lSectionShape->Y(lNext);

		Model::SurfaceElement *lElement = pLevel->GetRoomWallElement(lRoomId, lVertex);

		if(lElement != NULL) {
			int lNeighbor = pLevel->GetNeighbor(lRoomId, lVertex);

			if(lNeighbor == -1) {
				lP0.mZ = lCeilingLevel;
				lP1.mZ = lFloorLevel;

				lElement->RenderWallSurface(&m3DView, lP0, lP1, pLevel->GetRoomWallLen(lRoomId, lVertex), pTime);
			}
			else {
				MR_Int32 lNeighborFloor = pLevel->GetRoomBottomLevel(lNeighbor);
				MR_Int32 lNeighborCeiling = pLevel->GetRoomTopLevel(lNeighbor);

				if(lFloorLevel < lNeighborFloor) {
					lP0.mZ = lNeighborFloor;
					lP1.mZ = lFloorLevel;

					lElement->RenderWallSurface(&m3DView, lP0, lP1, pLevel->GetRoomWallLen(lRoomId, lVertex), pTime);
				}

				if(lCeilingLevel > lNeighborCeiling) {
					lP0.mZ = lCeilingLevel;
					lP1.mZ = lNeighborCeiling;

					lElement->RenderWallSurface(&m3DView, lP0, lP1, pLevel->GetRoomWallLen(lRoomId, lVertex), pTime);
				}
			}
		}

		lP0.mX = lP1.mX;
		lP0.mY = lP1.mY;

	}

	delete lSectionShape;
}

void Observer::RenderFeatureWalls(const Model::Level * pLevel, int lFeatureId, MR_SimulationTime pTime)
{
	MR_PolygonShape *lSectionShape = pLevel->GetFeatureShape(lFeatureId);

	int lVertexCount = lSectionShape->VertexCount();

	// Draw the walls
	MR_3DCoordinate lP0;
	MR_3DCoordinate lP1;

	lP0.mZ = lSectionShape->ZMax();

	lP1.mX = lSectionShape->X(0);
	lP1.mY = lSectionShape->Y(0);
	lP1.mZ = lSectionShape->ZMin();

	for(int lVertex = 0; lVertex < lVertexCount; lVertex++) {
		int lNext = lVertex + 1;

		if(lNext == lVertexCount) {
			lNext = 0;
		}

		lP0.mX = lSectionShape->X(lNext);
		lP0.mY = lSectionShape->Y(lNext);

		Model::SurfaceElement *lElement = pLevel->GetFeatureWallElement(lFeatureId, lVertex);

		if(lElement != NULL) {
			lElement->RenderWallSurface(&m3DView, lP0, lP1, pLevel->GetFeatureWallLen(lFeatureId, lVertex), pTime);
		}

		lP1.mX = lP0.mX;
		lP1.mY = lP0.mY;
	}

	delete lSectionShape;
}

void Observer::RenderFloorOrCeiling(const Model::Level * pLevel, const Model::SectionId & pSectionId, BOOL pFloor, MR_SimulationTime pTime)
{
	int lCounter;

	MR_Int32 lLevel;
	int lNbVertex;
	MR_2DCoordinate lVertexList[MR_MAX_POLYGON_VERTEX];

	MR_PolygonShape *lShape;
	Model::SurfaceElement *lElement;

	// Extract the surface geometry
	if(pSectionId.mType == Model::SectionId::eRoom) {
		lShape = pLevel->GetRoomShape(pSectionId.mId);
		if(pFloor) {
			lLevel = lShape->ZMin();
			lElement = pLevel->GetRoomBottomElement(pSectionId.mId);
		}
		else {
			lLevel = lShape->ZMax();
			lElement = pLevel->GetRoomTopElement(pSectionId.mId);
		}
	}
	else {
		lShape = pLevel->GetFeatureShape(pSectionId.mId);

		if(!pFloor) {
			lLevel = lShape->ZMin();
			lElement = pLevel->GetFeatureBottomElement(pSectionId.mId);
		}
		else {
			lLevel = lShape->ZMax();
			lElement = pLevel->GetFeatureTopElement(pSectionId.mId);
		}
	}

	if(lElement != NULL) {
		lNbVertex = lShape->VertexCount();

		for(lCounter = 0; lCounter < lNbVertex; lCounter++) {
			lVertexList[lCounter].mX = lShape->X(lCounter);
			lVertexList[lCounter].mY = lShape->Y(lCounter);
		}

		lElement->RenderHorizontalSurface(&m3DView, lNbVertex, lVertexList, lLevel, !pFloor, pTime);
	}

	delete lShape;
}

void Observer::RenderDebugDisplay(MR_VideoBuffer * pDest, const ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage)
{
	int lXRes = pDest->GetXRes();
	int lYRes = pDest->GetYRes();
	int lYOffset = 0;
	int lXOffset = 0;

	switch (mSplitMode) {
		case eUpperSplit:
			lYRes /= 2;
			break;

		case eLowerSplit:
			lYRes /= 2;
			lYOffset = lYRes;
			break;
	}

	mWireFrameView.Setup(pDest, 0, lYOffset, lXRes / 2, lYRes / 2, mApperture);
	m3DView.Setup(pDest, 0, lYOffset + lYRes / 2, lXRes / 2, lYRes / 2, mApperture);
	m2DDebugView.Setup(pDest, lXRes / 2, lYOffset, lXRes / 2, lYRes);

	if(pViewingCharacter->mRoom != -1) {
		const Model::Level *lLevel = pSession->GetCurrentLevel();

		Render2DDebugView(pDest, lLevel, pViewingCharacter);
		RenderWireFrameView(lLevel, pViewingCharacter);
		Render3DView(pSession, pViewingCharacter, pTime, pBackImage);
	}

}

void Observer::RenderNormalDisplay(MR_VideoBuffer * pDest, const ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage)
{
	MR_SAMPLE_CONTEXT("RenderNormalDisplay");

	int lXRes = pDest->GetXRes();
	int lYRes = pDest->GetYRes();
	int lYOffset = 0;
	int lXOffset = 0;
	int lYMargin_1024 = mYMargin_1024;
	int lXMargin_1024 = mXMargin_1024;

	switch (mSplitMode) {
		case eUpperSplit:
			lYRes /= 2;
			lYMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}

			break;

		case eLowerSplit:
			lYRes /= 2;
			lYOffset = lYRes;
			lYMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}

			break;

		case eUpperLeftSplit:
			lYRes /= 2;
			lXRes /= 2;
			lYMargin_1024 -= 200;
			lXMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}
			if(lXMargin_1024 < 0) {
				lXMargin_1024 = 0;
			}

			break;

		case eUpperRightSplit:
			lYRes /= 2;
			lXRes /= 2;
			lXOffset = lXRes;
			lYMargin_1024 -= 200;
			lXMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}
			if(lXMargin_1024 < 0) {
				lXMargin_1024 = 0;
			}

			break;

		case eLowerLeftSplit:
			lYRes /= 2;
			lXRes /= 2;
			lYOffset = lYRes;
			lYMargin_1024 -= 200;
			lXMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}
			if(lXMargin_1024 < 0) {
				lXMargin_1024 = 0;
			}
			break;

		case eLowerRightSplit:
			lYRes /= 2;
			lXRes /= 2;
			lYOffset = lYRes;
			lXOffset = lXRes;
			lYMargin_1024 -= 200;
			lXMargin_1024 -= 200;
			if(lYMargin_1024 < 0) {
				lYMargin_1024 = 0;
			}
			if(lXMargin_1024 < 0) {
				lXMargin_1024 = 0;
			}
			break;
	}

												  // rounded to 32 bit boundary for best performances
	int lXMargin = (mXMargin_1024 * lXRes / 1024) & 0xFFFFFFFC;
	int lYMargin = lYMargin_1024 * lYRes / 1024;

	m3DView.Setup(pDest, lXOffset + lXMargin, lYOffset + lYMargin, lXRes - 2 * lXMargin, lYRes - 2 * lYMargin, mApperture);

	// Clear screen if needed
	if(lXMargin > 0) {

	}

	if(lYMargin > 0) {
	}

	if(pViewingCharacter->mRoom != -1) {
		Render3DView(pSession, pViewingCharacter, pTime, pBackImage);
	}
}

void Observer::PlaySounds(const Model::Level * pLevel, MainCharacter::MainCharacter * pViewingCharacter)
{
	// Play the sound of all moving elemnts arround

	int lCurrentRoom = pViewingCharacter->mRoom;
	int lNeighborCount = pLevel->GetRoomVertexCount(lCurrentRoom);

	for(int lCounter = -1; lCounter < lNeighborCount; lCounter++) {
		int lRoomId;

		if(lCounter == -1) {
			lRoomId = lCurrentRoom;
		}
		else {
			lRoomId = pLevel->GetNeighbor(lCurrentRoom, lCounter);
		}

		if(lRoomId != -1) {
			MR_FreeElementHandle lHandle = pLevel->GetFirstFreeElement(lRoomId);

			while(lHandle != NULL) {
				Model::FreeElement *lElement = Model::Level::GetFreeElement(lHandle);

				if(lElement != pViewingCharacter) {
					double lXDist = pViewingCharacter->mPosition.mX - lElement->mPosition.mX;
					double lYDist = pViewingCharacter->mPosition.mY - lElement->mPosition.mY;

					int lDB = (int)(-sqrt(lXDist * lXDist + lYDist * lYDist) / 15.0);

					lElement->PlayExternalSounds(lDB, 0);
				}

				lHandle = Model::Level::GetNextFreeElement(lHandle);
			}
		}
	}

	pViewingCharacter->PlayInternalSounds();
}

}  // namespace Client
}  // namespace HoverRace
