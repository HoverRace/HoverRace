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

#ifndef OBSERVER_H
#define OBSERVER_H

#include "../../engine/VideoServices/3DViewPort.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/ObjFacTools/SpriteHandle.h"

namespace HoverRace {
	namespace Client {
		class ClientSession;
	}
	namespace VideoServices {
		class StaticText;
	}
}
class MR_SectionId;

class MR_Observer
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

		MR_SpriteHandle *mBaseFont;
		MR_SpriteHandle *mMissileLevel;
		MR_SpriteHandle *mMineDisp;
		MR_SpriteHandle *mPowerUpDisp;
		MR_SpriteHandle *mHoverIcons;

		HoverRace::VideoServices::StaticText *selectCraftTxt;
		HoverRace::VideoServices::StaticText *craftTxt;

		MR_Observer();
		~MR_Observer();

		void Render2DDebugView(MR_VideoBuffer * pDest, const MR_Level * pLevel, const MR_MainCharacter * pViewingCharacter);
		void RenderWireFrameView(const MR_Level * pLevel, const MR_MainCharacter * pViewingCharacter);
		void Render3DView(const HoverRace::Client::ClientSession * pSession, const MR_MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

		void DrawWFSection(const MR_Level * pLevel, const MR_SectionId & pSectionId, MR_UInt8 pColor);
		void RenderRoomWalls(const MR_Level * pLevel, int pRoomId, MR_SimulationTime pTime);
		void RenderFeatureWalls(const MR_Level * pLevel, int pFeatureId, MR_SimulationTime pTime);
		void RenderFloorOrCeiling(const MR_Level * pLevel, const MR_SectionId & pSectionId, BOOL pFloor, MR_SimulationTime pTime);

		static void DrawBackground(MR_VideoBuffer * pDest);

	public:
		static MR_Observer *New();				  // Local construction and destruction
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
		void RenderDebugDisplay(MR_VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MR_MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);
		void RenderNormalDisplay(MR_VideoBuffer * pDest, const HoverRace::Client::ClientSession *pSession, const MR_MainCharacter * pViewingCharacter, MR_SimulationTime pTime, const MR_UInt8 * pBackImage);

		void PlaySounds(const MR_Level * pLevel, MR_MainCharacter * pViewingCharacter);

};

#endif
