// ResActorExt.cpp
// Provides an extension to the MR_ResActor class to allow easy transitions back into .msh files.

#include "ResActorExt.h"

/**
 * Nothing to do
 */
//ResActorExt::ResActorExt() { }

int ResActorExt::GetComponentCount(int sequence, int frame) {
	return mSequenceList[sequence].mFrameList[frame].mNbComponent;
}

int ResActorExt::GetVertexCount(int sequence, int frame, int patch) {
	if(mSequenceList[sequence].mFrameList[frame].mComponentList[patch]->GetType() == MR_ResActor::ePatch)
		return ((MR_ResActor::Patch *) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mURes * 
		((MR_ResActor::Patch*) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mVRes;
	else
		return 0;
}

int ResActorExt::GetPatchURes(int sequence, int frame, int patch) {
	if(mSequenceList[sequence].mFrameList[frame].mComponentList[patch]->GetType() == MR_ResActor::ePatch)
		return ((MR_ResActor::Patch *) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mURes;
	else
		return 0;
}

int ResActorExt::GetPatchVRes(int sequence, int frame, int patch) {
	if(mSequenceList[sequence].mFrameList[frame].mComponentList[patch]->GetType() == MR_ResActor::ePatch)
		return ((MR_ResActor::Patch *) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mVRes;
	else
		return 0;
}

int ResActorExt::GetPatchBitmapId(int sequence, int frame, int patch) {
	if(mSequenceList[sequence].mFrameList[frame].mComponentList[patch]->GetType() == MR_ResActor::ePatch)
		return ((MR_ResActor::Patch *) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mBitmap->GetResourceId();
	else
		return 0;
}

MR_3DCoordinate *ResActorExt::GetVertexList(int sequence, int frame, int patch) {
	if(mSequenceList[sequence].mFrameList[frame].mComponentList[patch]->GetType() == MR_ResActor::ePatch)
		return ((MR_ResActor::Patch *) mSequenceList[sequence].mFrameList[frame].mComponentList[patch])->mVertexList;
	else
		return NULL;
}