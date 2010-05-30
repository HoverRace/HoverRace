// HoverRender.h
//
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

#ifndef HOVER_RENDER_H
#define HOVER_RENDER_H

#include "../ObjFacTools/FreeElementBase.h"
#include "../MainCharacter/MainCharacterRenderer.h"

class MR_HoverRender : public MR_MainCharacterRenderer
{
	protected:
		const MR_ResActor *mActor0;
		const MR_ResActor *mActor1;
		const MR_ResActor *mActor2;
		const MR_ResActor *mActor3;

		int mFrame;

		HoverRace::VideoServices::ShortSound *mLineCrossingSound;
		HoverRace::VideoServices::ShortSound *mStartSound;
		HoverRace::VideoServices::ShortSound *mFinishSound;
		HoverRace::VideoServices::ShortSound *mBumpSound;
		HoverRace::VideoServices::ShortSound *mJumpSound;
		HoverRace::VideoServices::ShortSound *mFireSound;
		HoverRace::VideoServices::ShortSound *mMisJumpSound;
		HoverRace::VideoServices::ShortSound *mMisFireSound;
		HoverRace::VideoServices::ShortSound *mOutOfCtrlSound;
		HoverRace::VideoServices::ContinuousSound *mMotorSound;
		HoverRace::VideoServices::ContinuousSound *mFrictionSound;

		const MR_ResBitmap *mCockpitBitmap[10];
		const MR_ResBitmap *mCockpitBitmap2[10];
		const MR_ResBitmap *mEonCockpitBitmap[10];

	public:
		MR_HoverRender(const MR_ObjectFromFactoryId &pId, HoverRace::ObjFacTools::ResourceLib *resourceLib);
		~MR_HoverRender();

		void Render(MR_3DViewPort *pDest, const MR_3DCoordinate &pPosition, MR_Angle pOrientation, BOOL pMotorOn, int pHoverId, int pModel);

		virtual HoverRace::VideoServices::ShortSound *GetLineCrossingSound();
		virtual HoverRace::VideoServices::ShortSound *GetStartSound();
		virtual HoverRace::VideoServices::ShortSound *GetFinishSound();
		virtual HoverRace::VideoServices::ShortSound *GetBumpSound();
		virtual HoverRace::VideoServices::ShortSound *GetJumpSound();
		virtual HoverRace::VideoServices::ShortSound *GetFireSound();
		virtual HoverRace::VideoServices::ShortSound *GetMisJumpSound();
		virtual HoverRace::VideoServices::ShortSound *GetMisFireSound();
		virtual HoverRace::VideoServices::ShortSound *GetOutOfCtrlSound();
		virtual HoverRace::VideoServices::ContinuousSound *GetMotorSound();
		virtual HoverRace::VideoServices::ContinuousSound *GetFrictionSound();

};
#endif
