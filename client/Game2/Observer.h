// Observer.h// MR_Observer object interface
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

#pragma once

#include "../../engine/VideoServices/3DViewPort.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/ObjFacTools/SpriteHandle.h"

namespace HoverRace {
	namespace Client {
		class ClientSession;
	}
	namespace Model {
		class SectionId;
	}
	namespace VideoServices {
		class StaticText;
	}
}

namespace HoverRace {
namespace Client {

class Observer
{
	public:
		enum eSplitMode
		{
			eNotSplit,
			eUpperSplit,
			eLowerSplit,
			eUpperLeftSplit,
			eUpperRightSplit,
			eLowerLeftSplit,
			eLowerRightSplit,
		};

	private:
		MR_3DCoordinate mLastCameraPos;
		BOOL mLastCameraPosValid;
		BOOL mCockpitView;

		MR_2DViewPort m2DDebugView;
		MR_3DViewPort mWireFrameView;
		MR_3DViewPort m3DView;

		eSplitMode mSplitMode;

		int mScroll;
		MR_Angle mApperture;
		int mXMargin_1024;						  // Fraction of Horizontal screen that is a margin
		int mYMargin_1024;						  // Fraction of vertical screen that is a margin

		BOOL mMoreMessages;
		int mDispPlayers;						  // Index of the players list to display
		// 0 mean do not display

		ObjFac1::SpriteHandle *mBaseFont;
		ObjFac1::SpriteHandle *mMissileLevel;
		ObjFac1::SpriteHandle *mMineDisp;
		ObjFac1::SpriteHandle *mPowerUpDisp;
		ObjFac1::SpriteHandle *mHoverIcons;

		VideoServices::StaticText *selectCraftTxt;
		VideoServices::StaticText *craftTxt;

		Observer();
		~Observer();

		void Render2DDebugView(MR_VideoBuffer * pDest, const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter);
		void RenderWireFrameView(const Model::Level * pLevel, const MainCharacter::MainCharacter * pViewingCharacter);
		void Render3DView(const HoverRace::Client::ClientSession * pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

		void DrawWFSection(const Model::Level * pLevel, const Model::SectionId & pSectionId, MR_UInt8 pColor);
		void RenderRoomWalls(const Model::Level * pLevel, int pRoomId, MR_SimulationTime pTime);
		void RenderFeatureWalls(const Model::Level * pLevel, int pFeatureId, MR_SimulationTime pTime);
		void RenderFloorOrCeiling(const Model::Level * pLevel, const Model::SectionId & pSectionId, BOOL pFloor, MR_SimulationTime pTime);

		static void DrawBackground(MR_VideoBuffer * pDest);

	public:
		static Observer *New();				  // Local construction and destruction
		void Delete();							  // to be able to change the dll without problem

		// Camera control
		void Scroll(int pOffset);
		void ZoomIn();
		void ZoomOut();
		void Home();

		void EnlargeMargin();
		void ReduceMargin();

		void MoreMessages();
		void PlayersListPageDn();

		void SetCockpitView(BOOL pOn);

		void SetSplitMode(eSplitMode pMode);

		// Rendering function
		void RenderDebugDisplay(MR_VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);
		void RenderNormalDisplay(MR_VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MainCharacter::MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

		void PlaySounds(const Model::Level * pLevel, MainCharacter::MainCharacter * pViewingCharacter);

};

}  // namespace Client
}  // namespace HoverRace
