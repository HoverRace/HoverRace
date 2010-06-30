// HoverRender.cpp
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

#include "HoverRender.h"
#include "ObjFac1Res.h"
#include "../Util/FuzzyLogic.h"

using HoverRace::ObjFacTools::ResourceLib;
using namespace HoverRace::VideoServices;

namespace HoverRace {
namespace ObjFac1 {

// Local stuff

class ResActorFriend
{
	public:
		static void Draw(const MR_ResActor *pActor, MR_3DViewPort *pDest, const MR_PositionMatrix &pMatrix, int pSequence, int pFrame, const MR_Bitmap *pCockpitBitmap);
};

HoverRender::HoverRender(const Util::ObjectFromFactoryId &pId, ResourceLib *resourceLib) :
	SUPER(pId)
{
	mFrame = 0;
	mActor0 = resourceLib->GetActor(MR_ELECTRO_CAR);
	mActor1 = resourceLib->GetActor(MR_HITECH_CAR);
	mActor2 = resourceLib->GetActor(MR_BITURBO_CAR);
	mActor3 = resourceLib->GetActor(MR_EON_CRAFT);

	mLineCrossingSound = resourceLib->GetShortSound(MR_SND_LINE_CROSSING)->GetSound();
	mStartSound = resourceLib->GetShortSound(MR_SND_START)->GetSound();
	mFinishSound = resourceLib->GetShortSound(MR_SND_FINISH)->GetSound();
	mBumpSound = resourceLib->GetShortSound(MR_SND_BUMP)->GetSound();
	mJumpSound = resourceLib->GetShortSound(MR_SND_JUMP)->GetSound();
	mFireSound = resourceLib->GetShortSound(MR_SND_FIRE)->GetSound();
	mMisJumpSound = resourceLib->GetShortSound(MR_SND_MIS_JUMP)->GetSound();
	mMisFireSound = resourceLib->GetShortSound(MR_SND_MIS_FIRE)->GetSound();
	mOutOfCtrlSound = resourceLib->GetShortSound(MR_SND_OUT_OF_CTRL)->GetSound();
	mMotorSound = resourceLib->GetContinuousSound(MR_SND_MOTOR)->GetSound();
	mFrictionSound = resourceLib->GetContinuousSound(MR_SND_FRICTION)->GetSound();

	for(int lCounter = 0; lCounter < 10; lCounter++) {
		mCockpitBitmap[lCounter] = resourceLib->GetBitmap(MR_CAR_COCKPIT1 + lCounter);
		mCockpitBitmap2[lCounter] = resourceLib->GetBitmap(MR_CAR_COCKPIT21 + lCounter);
		mEonCockpitBitmap[lCounter] = resourceLib->GetBitmap(MR_CAR_COCKPIT31 + lCounter);
	}
}

HoverRender::~HoverRender()
{

}

void HoverRender::Render(MR_3DViewPort *pDest, const MR_3DCoordinate &pPosition, MR_Angle pOrientation, BOOL pMotorOn, int pHoverId, int pModel)
{
	// Compute the required rotation matrix
	MR_PositionMatrix lMatrix;

	if(pDest->ComputePositionMatrix(lMatrix, pPosition, pOrientation, 1000 /* TODO Object ray must be precomputed at compilation */ )) {
		int lSeq = pMotorOn ? 1 : 0;

		if(pMotorOn) {
			mFrame = (mFrame + 1) % 2;
		}
		else {
			mFrame = 0;
		}

		if(pModel == 1) {
			ResActorFriend::Draw(mActor1, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap2[pHoverId % 10]);
		} else if(pModel == 2) {
			ResActorFriend::Draw(mActor2, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap[pHoverId % 10]);
		} else if(pModel == 3) {
			ResActorFriend::Draw(mActor3, pDest, lMatrix, lSeq, mFrame, mEonCockpitBitmap[pHoverId % 10]);
		} else {
			ResActorFriend::Draw(mActor0, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap[pHoverId % 10]);
		}
	}
}

ShortSound *HoverRender::GetLineCrossingSound()
{
	return mLineCrossingSound;
}

ShortSound *HoverRender::GetStartSound()
{
	return mStartSound;
}

ShortSound *HoverRender::GetFinishSound()
{
	return mFinishSound;
}

ShortSound *HoverRender::GetBumpSound()
{
	return mBumpSound;
}

ShortSound *HoverRender::GetJumpSound()
{
	return mJumpSound;
}

ShortSound *HoverRender::GetFireSound()
{
	return mFireSound;
}

ShortSound *HoverRender::GetMisJumpSound()
{
	return mMisJumpSound;
}

ShortSound *HoverRender::GetMisFireSound()
{
	return mMisFireSound;
}

ShortSound *HoverRender::GetOutOfCtrlSound()
{
	return mOutOfCtrlSound;
}

ContinuousSound *HoverRender::GetMotorSound()
{
	return mMotorSound;
}

ContinuousSound *HoverRender::GetFrictionSound()
{
	return mFrictionSound;
}

void ResActorFriend::Draw(const MR_ResActor *pActor, MR_3DViewPort *pDest, const MR_PositionMatrix &pMatrix, int pSequence, int pFrame, const MR_Bitmap *pCockpitBitmap)
{
	MR_ResActor::Frame * lFrame = &(pActor->mSequenceList[pSequence].mFrameList[pFrame]);

	ASSERT(lFrame != NULL);

	for(int lCounter = 0; lCounter < lFrame->mNbComponent; lCounter++) {
		MR_ResActor::Patch *lPatch = (MR_ResActor::Patch *) lFrame->mComponentList[lCounter];

		int lBitmapResId = lPatch->mBitmap->GetResourceId();

		if((lBitmapResId == MR_CAR_COCKPIT) || (lBitmapResId == MR_CAR2_COCKPIT) ||(lBitmapResId == MR_EON_COCKPIT)) {
			pDest->RenderPatch(*lPatch, pMatrix, pCockpitBitmap);
		}
		else {
			pDest->RenderPatch(*lPatch, pMatrix, lPatch->mBitmap);
		}
	}
}

}  // namespace ObjFac1
}  // namespace HoverRace
