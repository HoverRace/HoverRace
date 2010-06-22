// ResSound.cpp
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

#include "ResSound.h"

using HoverRace::Parcel::ObjStream;
using namespace HoverRace::VideoServices;

MR_ResShortSound::MR_ResShortSound(int pResourceId)
{
	mResourceId = pResourceId;
	mNbCopy = 0;
	mDataLen = 0;
	mData = NULL;
	mSound = NULL;

}

MR_ResShortSound::~MR_ResShortSound()
{
	SoundServer::DeleteShortSound(mSound);
	mSound = NULL;

	delete[]mData;
	mData = NULL;
}

int MR_ResShortSound::GetResourceId() const
{
	return mResourceId;
}

void MR_ResShortSound::Serialize(ObjStream &pArchive)
{
	if(pArchive.IsWriting()) {
		pArchive << mNbCopy;
		pArchive << mDataLen;
		pArchive.Write(mData, mDataLen);

	}
	else {
		SoundServer::DeleteShortSound(mSound);
		mSound = NULL;

		delete[]mData;
		mData = NULL;

		pArchive >> mNbCopy;
		pArchive >> mDataLen;

		mData = new char[mDataLen];

		pArchive.Read(mData, mDataLen);

		mSound = SoundServer::CreateShortSound(mData, mNbCopy);

	}
}

ShortSound *MR_ResShortSound::GetSound() const
{
	return mSound;
}

MR_ResContinuousSound::MR_ResContinuousSound(int pResourceId)
{
	mResourceId = pResourceId;
	mNbCopy = 0;
	mDataLen = 0;
	mData = NULL;
	mSound = NULL;
}

MR_ResContinuousSound::~MR_ResContinuousSound()
{
	SoundServer::DeleteContinuousSound(mSound);
	mSound = NULL;

	delete[]mData;
	mData = NULL;
}

int MR_ResContinuousSound::GetResourceId() const
{
	return mResourceId;
}

void MR_ResContinuousSound::Serialize(ObjStream &pArchive)
{
	if(pArchive.IsWriting()) {
		pArchive << mNbCopy;
		pArchive << mDataLen;
		pArchive.Write(mData, mDataLen);

	}
	else {
		SoundServer::DeleteContinuousSound(mSound);
		mSound = NULL;

		delete[]mData;
		mData = NULL;

		pArchive >> mNbCopy;
		pArchive >> mDataLen;

		mData = new char[mDataLen];

		pArchive.Read(mData, mDataLen);

		mSound = SoundServer::CreateContinuousSound(mData, mNbCopy);

	}
}

HoverRace::VideoServices::ContinuousSound *MR_ResContinuousSound::GetSound() const
{
	return mSound;
}
