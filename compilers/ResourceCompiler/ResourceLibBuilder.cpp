// ResourceLibBuilder.cpp
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

#include "StdAfx.h"

#include <boost/foreach.hpp>

#include "../../engine/Parcel/MfcRecordFile.h"
#include "../../engine/Parcel/ObjStream.h"

#include "ResourceLibBuilder.h"

using namespace HoverRace::Parcel;

namespace {
	template<class T>
	void WriteRes(ObjStream &os, T &res)
	{
		MR_UInt32 num = res.size();
		os << num;
		BOOST_FOREACH(typename const T::value_type &ent, res) {
			MR_Int32 key = ent.first;
			os << key;
			ent.second->Serialize(os);
		}
	}
}

MR_ResourceLibBuilder::MR_ResourceLibBuilder()
{
	// Notting to do
}

MR_ResourceLibBuilder::~MR_ResourceLibBuilder()
{
	// Notting to do
}

void MR_ResourceLibBuilder::AddBitmap(MR_ResBitmap * pBitmap)
{
	bitmaps.insert(bitmaps_t::value_type(pBitmap->GetResourceId(), pBitmap));
}

void MR_ResourceLibBuilder::AddActor(MR_ResActor * pActor)
{
	actors.insert(actors_t::value_type(pActor->GetResourceId(), pActor));
}

void MR_ResourceLibBuilder::AddSprite(MR_ResSprite * pSprite)
{
	sprites.insert(sprites_t::value_type(pSprite->GetResourceId(), pSprite));
}

void MR_ResourceLibBuilder::AddSound(MR_ResShortSound * pSound)
{
	shortSounds.insert(shortSounds_t::value_type(pSound->GetResourceId(), pSound));
}

void MR_ResourceLibBuilder::AddSound(MR_ResContinuousSound * pSound)
{
	continuousSounds.insert(continuousSounds_t::value_type(pSound->GetResourceId(), pSound));
}

/**
 * This is the function that is called when the file is being written.
 * It creates the output file, then writes (in this order):
 * - bitmaps
 * - actors (meshes)
 * - sprites
 * - sounds
 *
 * @param pFileName Filename of output file
 * @return BOOL indicating the success of the operation
 */
BOOL MR_ResourceLibBuilder::Export(const char *pFileName)
{
	BOOL lReturnValue = TRUE;

	MfcRecordFile lFile;

	lReturnValue = lFile.CreateForWrite(pFileName, 1, "\x8\rFireball object factory resource file, (c)GrokkSoft 1996\n\x1a");

	if(!lReturnValue) {
		fprintf(stderr, "ERROR: Unable to create output file\n");
	}
	else {
		lFile.BeginANewRecord();

		{
			// Write the magic number
			int lMagicNumber = FILE_MAGIC;

			ObjStreamPtr archivePtr(lFile.StreamOut());
			ObjStream &lArchive = *archivePtr;

			lArchive << lMagicNumber;

			WriteRes(lArchive, bitmaps);
			WriteRes(lArchive, actors);
			WriteRes(lArchive, sprites);
			WriteRes(lArchive, shortSounds);
			WriteRes(lArchive, continuousSounds);
		}
	}

	return lReturnValue;
}
