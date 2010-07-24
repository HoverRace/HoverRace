// ResActor.cpp
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

#include "../Parcel/ObjStream.h"
#include "ResourceLib.h"

#include "ResActor.h"

using HoverRace::Parcel::ObjStream;
using HoverRace::Parcel::ObjStreamExn;

namespace HoverRace {
namespace ObjFacTools {

ResActor::ResActor(int pResourceId)
{
	mResourceId = pResourceId;
	mNbSequence = 0;
	mSequenceList = NULL;
}

ResActor::~ResActor()
{
	delete[]mSequenceList;
}

int ResActor::GetResourceId() const
{
	return mResourceId;
}

int ResActor::GetSequenceCount() const
{
	return mNbSequence;
}

int ResActor::GetFrameCount(int pSequence) const
{
	return mSequenceList[pSequence].mNbFrame;
} 

void ResActor::Serialize(ObjStream &pArchive, ResourceLib *pLib)
{

	if(pArchive.IsWriting()) {
		pArchive << mNbSequence;
	}
	else {
		delete[]mSequenceList;
		mSequenceList = NULL;

		pArchive >> mNbSequence;

		if(mNbSequence != 0) {
			mSequenceList = new Sequence[mNbSequence];
		}
	}

	for(int lCounter = 0; lCounter < mNbSequence; lCounter++) {
		mSequenceList[lCounter].Serialize(pArchive, pLib);
	}

}

void ResActor::Draw(VideoServices::Viewport3D * pDest, const VideoServices::PositionMatrix & pMatrix, int pSequence, int pFrame) const
{
	ASSERT(pSequence < mNbSequence);

	mSequenceList[pSequence].Draw(pDest, pMatrix, pFrame);
}

// Sequence 
ResActor::Sequence::Sequence()
{
	mNbFrame = 0;
	mFrameList = NULL;
}

ResActor::Sequence::~Sequence()
{
	delete[]mFrameList;
}

void ResActor::Sequence::Serialize(ObjStream &pArchive, ResourceLib *pLib)
{
	if(pArchive.IsWriting()) {
		pArchive << mNbFrame;
	}
	else {
		delete[]mFrameList;
		mFrameList = NULL;

		pArchive >> mNbFrame;

		if(mNbFrame != 0) {
			mFrameList = new Frame[mNbFrame];
		}
	}

	for(int lCounter = 0; lCounter < mNbFrame; lCounter++) {
		mFrameList[lCounter].Serialize(pArchive, pLib);
	}
}

void ResActor::Sequence::Draw(VideoServices::Viewport3D * pDest, const VideoServices::PositionMatrix & pMatrix, int pFrame) const
{
	ASSERT(pFrame < mNbFrame);

	mFrameList[pFrame].Draw(pDest, pMatrix);
}

// Frame 
ResActor::Frame::Frame()
{
	mNbComponent = 0;
	mComponentList = NULL;

}

ResActor::Frame::~Frame()
{
	Clean();
}

void ResActor::Frame::Clean()
{
	if(mComponentList != NULL) {
		for(int lCounter = 0; lCounter < mNbComponent; lCounter++) {
			delete mComponentList[lCounter];
		}

		delete[]mComponentList;
		mComponentList = NULL;
	}
}

void ResActor::Frame::Serialize(ObjStream &pArchive, ResourceLib *pLib)
{
	int lCounter;

	if(pArchive.IsWriting()) {
		pArchive << mNbComponent;

		for(lCounter = 0; lCounter < mNbComponent; lCounter++) {
			pArchive << (int) mComponentList[lCounter]->GetType();
			mComponentList[lCounter]->Serialize(pArchive, pLib);
		}
	}
	else {
		Clean();

		pArchive >> mNbComponent;

		if(mNbComponent != 0) {
			mComponentList = new ActorComponent *[mNbComponent];

			for(lCounter = 0; lCounter < mNbComponent; lCounter++) {
				int lType;

				pArchive >> lType;

				switch ((eComponentType) lType) {
					case ePatch:
						mComponentList[lCounter] = new Patch;
						break;

					default:
						ASSERT(FALSE);
						throw ObjStreamExn(pArchive.GetName(),
							boost::str(boost::format("Unhandled component type: %d") % lType));
				}

				mComponentList[lCounter]->Serialize(pArchive, pLib);
			}
		}
	}
}

void ResActor::Frame::Draw(VideoServices::Viewport3D * pDest, const VideoServices::PositionMatrix & pMatrix) const
{
	// Draw each component of the frame
	for(int lCounter = 0; lCounter < mNbComponent; lCounter++) {
		mComponentList[lCounter]->Draw(pDest, pMatrix);
	}
}

// class ActorComponent 
ResActor::ActorComponent::~ActorComponent()
{
	// Notting to do
}

// Patch
//
ResActor::Patch::Patch()
{
	mBitmap = NULL;
	mVertexList = NULL;
}

ResActor::Patch::~Patch()
{
	delete[]mVertexList;
}

ResActor::eComponentType ResActor::Patch::GetType() const
{
	return ePatch;
}

void ResActor::Patch::Serialize(ObjStream &pArchive, ResourceLib *pLib)
{
	int lCounter;

	if(pArchive.IsWriting()) {
		pArchive << mURes;
		pArchive << mVRes;
		pArchive << mBitmap->GetResourceId();	  //bitmaptype is serialize using the Id of the bitmap

		for(lCounter = 0; lCounter < mURes * mVRes; lCounter++) {
			mVertexList[lCounter].Serialize(pArchive);
		}
	}
	else {
		int lBitmapId;

		delete[]mVertexList;

		pArchive >> mURes;
		pArchive >> mVRes;
		pArchive >> lBitmapId;

		mBitmap = pLib->GetBitmap(lBitmapId);

		mVertexList = new MR_3DCoordinate[mURes * mVRes];

		for(lCounter = 0; lCounter < mURes * mVRes; lCounter++) {
			mVertexList[lCounter].Serialize(pArchive);
		}
	}
}

void ResActor::Patch::Draw(VideoServices::Viewport3D *pDest, const VideoServices::PositionMatrix & pMatrix) const
{
	pDest->RenderPatch(*this, pMatrix, mBitmap);
}

int ResActor::Patch::GetURes() const
{
	return mURes;
}

int ResActor::Patch::GetVRes() const
{
	return mVRes;
}

const MR_3DCoordinate *ResActor::Patch::GetNodeList() const
{
	return mVertexList;
}

}  // namespace ObjFacTools
}  // namespace HoverRace
