// ResourceLib.cpp
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

#define new DEBUG_NEW

using namespace HoverRace::Parcel;

namespace HoverRace {
namespace ObjFacTools {

/**
 * Create a new ResourceLib object, reading from the specified filename.
 *
 * @param pResFile Filename of object
 */
ResourceLib::ResourceLib(const char *pResFile)
{
	if(mRecordFile.OpenForRead(pResFile)) {

		mRecordFile.SelectRecord(0);

		{
			int lMagicNumber;

			ObjStreamPtr archivePtr(mRecordFile.StreamIn());
			ObjStream &lArchive = *archivePtr;

			lArchive >> lMagicNumber;

			if(lMagicNumber == MR_RESOURCE_FILE_MAGIC) {
				// Load the Bitmaps
				LoadBitmaps(lArchive);

				// Load the Actors
				LoadActors(lArchive);

				// Load the sprites
				LoadSprites(lArchive);

				// Load the sounds
				LoadSounds(lArchive);

			}
			else {
				ASSERT(FALSE);
				throw ObjStreamExn(pResFile,
					boost::str(boost::format("Invalid magic number: Expected %08x, got %08x instead") %
						MR_RESOURCE_FILE_MAGIC % lMagicNumber));
			}
		}
	}
	else {
		ASSERT(FALSE);
		throw ObjStreamExn(pResFile, "File not found or not readable");
	}
}

/**
 * Empty constructor
 */
ResourceLib::ResourceLib() { }

/**
 * Close the ResourceLib, deleting all of the objects that were created.
 */
ResourceLib::~ResourceLib()
{
	POSITION lPos;

	lPos = mActorList.GetStartPosition();

	while(lPos != NULL) {
		int lKey;
		MR_ResActor *lValue;

		mActorList.GetNextAssoc(lPos, lKey, lValue);

		delete lValue;
	}

	lPos = mBitmapList.GetStartPosition();

	while(lPos != NULL) {
		int lKey;
		MR_ResBitmap *lValue;

		mBitmapList.GetNextAssoc(lPos, lKey, lValue);

		delete lValue;
	}

	lPos = mSpriteList.GetStartPosition();

	while(lPos != NULL) {
		int lKey;
		MR_ResSprite *lValue;

		mSpriteList.GetNextAssoc(lPos, lKey, lValue);

		delete lValue;
	}

	lPos = mShortSoundList.GetStartPosition();

	while(lPos != NULL) {
		int lKey;
		MR_ResShortSound *lValue;

		mShortSoundList.GetNextAssoc(lPos, lKey, lValue);

		delete lValue;
	}

	lPos = mContinuousSoundList.GetStartPosition();

	while(lPos != NULL) {
		int lKey;
		MR_ResContinuousSound *lValue;

		mContinuousSoundList.GetNextAssoc(lPos, lKey, lValue);

		delete lValue;
	}

}

/*const*/ MR_ResBitmap *ResourceLib::GetBitmap(int pBitmapId)
{

	MR_ResBitmap *lValue = NULL;

	mBitmapList.Lookup(pBitmapId, lValue);

	return lValue;
}

const MR_ResActor *ResourceLib::GetActor(int pActorId)
{
	MR_ResActor *lValue = NULL;

	mActorList.Lookup(pActorId, lValue);

	return lValue;
}

const MR_ResSprite *ResourceLib::GetSprite(int pSpriteId)
{
	MR_ResSprite *lValue = NULL;

	mSpriteList.Lookup(pSpriteId, lValue);

	return lValue;
}

const MR_ResShortSound *ResourceLib::GetShortSound(int pSoundId)
{
	MR_ResShortSound *lValue = NULL;

	mShortSoundList.Lookup(pSoundId, lValue);

	return lValue;
}

const MR_ResContinuousSound *ResourceLib::GetContinuousSound(int pSoundId)
{
	MR_ResContinuousSound *lValue = NULL;

	mContinuousSoundList.Lookup(pSoundId, lValue);

	return lValue;
}

/**
 * Load each bitmap from the archive, inserting it into mBitmapList.
 *
 * @param pArchive The archive to read from
 */
void ResourceLib::LoadBitmaps(ObjStream &pArchive)
{
	int lNbBitmap;

	pArchive >> lNbBitmap;

	for(int lCounter = 0; lCounter < lNbBitmap; lCounter++) {
		int lKey;
		MR_ResBitmap *lValue;

		pArchive >> lKey;

		lValue = new MR_ResBitmap(lKey);

		lValue->Serialize(pArchive);

		mBitmapList.SetAt(lKey, lValue);
	}
}

/**
 * Load each actor (mesh) from the archive, inserting it into mActorList.
 *
 * @param pArchive The archive to read from
 */
void ResourceLib::LoadActors(ObjStream &pArchive)
{
	int lNbActor;

	pArchive >> lNbActor;

	for(int lCounter = 0; lCounter < lNbActor; lCounter++) {
		int lKey;
		MR_ResActor *lValue;

		pArchive >> lKey;

		lValue = new MR_ResActor(lKey);
		lValue->Serialize(pArchive, this);

		mActorList.SetAt(lKey, lValue);
	}
}

/**
 * Load each sprite from the archive, inserting it into mSpriteList.
 *
 * @param pArchive The archive to read from
 */
void ResourceLib::LoadSprites(ObjStream &pArchive)
{
	int lNbSprite;

	pArchive >> lNbSprite;

	for(int lCounter = 0; lCounter < lNbSprite; lCounter++) {
		int lKey;
		MR_ResSprite *lValue;

		pArchive >> lKey;

		lValue = new MR_ResSprite(lKey);
		lValue->Serialize(pArchive);

		mSpriteList.SetAt(lKey, lValue);
	}
}

/**
 * Load each sound (ShortSounds and then ContinuousSounds) and insert them into
 * the mShortSoundList and mContinuousSoundList objects.
 *
 * @param pArchive The archive to read from
 */
void ResourceLib::LoadSounds(ObjStream &pArchive)
{
	int lNbSound;
	int lCounter;

	pArchive >> lNbSound;

	for(lCounter = 0; lCounter < lNbSound; lCounter++) {
		int lKey;
		MR_ResShortSound *lValue;

		pArchive >> lKey;

		lValue = new MR_ResShortSound(lKey);
		lValue->Serialize(pArchive);

		mShortSoundList.SetAt(lKey, lValue);
	}

	pArchive >> lNbSound;

	for(lCounter = 0; lCounter < lNbSound; lCounter++) {
		int lKey;
		MR_ResContinuousSound *lValue;

		pArchive >> lKey;

		lValue = new MR_ResContinuousSound(lKey);
		lValue->Serialize(pArchive);

		mContinuousSoundList.SetAt(lKey, lValue);
	}

}

}  // namespace ObjFacTools
}  // namespace HoverRace
