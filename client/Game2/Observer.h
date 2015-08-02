// Observer.h
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

#include "../../engine/Display/HudCell.h"
#include "../../engine/VideoServices/Viewport3D.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/ObjFacTools/SpriteHandle.h"

namespace HoverRace {
	namespace Client {
		class ClientSession;
	}
	namespace Model {
		struct SectionId;
	}
}

namespace HoverRace {
namespace Client {

class Observer
{
private:
	MR_3DCoordinate mLastCameraPos;
	BOOL mLastCameraPosValid;
	BOOL mCockpitView;
	bool hudVisible;
	bool demoMode;

	VideoServices::Viewport2D m2DDebugView;
	VideoServices::Viewport3D mWireFrameView;
	VideoServices::Viewport3D m3DView;

	Display::HudCell splitMode;

	int mScroll;
	MR_Angle mApperture;
	int mXMargin_1024;						  // Fraction of Horizontal screen that is a margin
	int mYMargin_1024;						  // Fraction of vertical screen that is a margin

	BOOL mMoreMessages;
	int mDispPlayers;						  // Index of the players list to display
	// 0 mean do not display

	std::shared_ptr<ObjFac1::SpriteHandle> mBaseFont;
	std::shared_ptr<ObjFac1::SpriteHandle> mMissileLevel;
	std::shared_ptr<ObjFac1::SpriteHandle> mMineDisp;
	std::shared_ptr<ObjFac1::SpriteHandle> mPowerUpDisp;
	std::shared_ptr<ObjFac1::SpriteHandle> mHoverIcons;

public:
	Observer();
	~Observer() { }

private:
	void Render2DDebugView(VideoServices::VideoBuffer * pDest, const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter);
	void RenderWireFrameView(const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter);
	void Render3DView(const HoverRace::Client::ClientSession * pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

	void DrawWFSection(const Model::Level * pLevel, const Model::SectionId & pSectionId, MR_UInt8 pColor);
	void RenderRoomWalls(const Model::Level * pLevel, int pRoomId, MR_SimulationTime pTime);
	void RenderFeatureWalls(const Model::Level * pLevel, int pFeatureId, MR_SimulationTime pTime);
	void RenderFloorOrCeiling(const Model::Level * pLevel, const Model::SectionId & pSectionId, BOOL pFloor, MR_SimulationTime pTime);

	static void DrawBackground(VideoServices::VideoBuffer * pDest);

public:
	static const std::string &GetCraftName(int id);

	// Camera control
	void Scroll(int pOffset);
	void Zoom(int factor); ///< A positive factor zooms in; a negative factor zooms out.
	void Home();
	void ToggleHudVisible();
	void SetHudVisible(bool visible);
	void StartDemoMode();

	void EnlargeMargin();
	void ReduceMargin();

	void MoreMessages();
	void PlayersListPageDn();

	void SetCockpitView(BOOL pOn);

	void SetSplitMode(Display::HudCell pMode);

	// Rendering function
	void RenderDebugDisplay(VideoServices::VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);
	void RenderNormalDisplay(VideoServices::VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

	void PlaySounds(const Model::Level * pLevel, MainCharacter::MainCharacter * pViewingCharacter);

};

}  // namespace Client
}  // namespace HoverRace
