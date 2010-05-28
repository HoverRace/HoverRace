
// ResourceLib.h
// Loadable resource manager.
//
// Copyright (c) 2010 Michael Imamura.
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

#include "StdAfx.h"

#include "../Parcel/ClassicRecordFile.h"
#ifdef _WIN32
#	include "../Parcel/MfcRecordFile.h"
#endif
#include "../Parcel/ObjStream.h"

#include "ResourceLib.h"

using namespace HoverRace::Parcel;

namespace {
	using HoverRace::ObjFacTools::ResourceLib;

	template<class T>
	void NewRes(T *val, ObjStream &os, ResourceLib *self)
	{
		val->Serialize(os);
	}

	template<>
	void NewRes<MR_ResActor>(MR_ResActor *val, ObjStream &os, ResourceLib *self)
	{
		val->Serialize(os, self);
	}

	template<class T>
	void LoadRes(ObjStream &os, std::map<int, T*> &res, ResourceLib *self)
	{
		MR_UInt32 num;
		os >> num;
		for (MR_UInt32 i = 0; i < num; ++i) {
			MR_Int32 key;
			os >> key;

			T *val = new T(key);
			NewRes(val, os, self);

			res.insert(std::map<int, T*>::value_type(key, val));
		}
	}
}

namespace HoverRace {
namespace ObjFacTools {

/**
 * Constructor.
 * @param filename The resource data file.
 */
ResourceLib::ResourceLib(const char *filename)
{
#	ifdef _WIN32
		recordFile = MfcRecordFile::New();
#	else
		recordFile = new ClassicRecordFile();
#	endif

	if (!recordFile->OpenForRead(filename)) {
		throw ObjStreamExn(filename, "File not found or not readable");
	}

	recordFile->SelectRecord(0);

	ObjStreamPtr osPtr(recordFile->StreamIn());
	ObjStream &os = *osPtr;

	MR_UInt32 magic;
	os >> magic;
	if (magic != MR_RESOURCE_FILE_MAGIC) {
		throw ObjStreamExn(filename,
			boost::str(boost::format("Invalid magic number: Expected %08x, got %08x instead") %
				MR_RESOURCE_FILE_MAGIC % magic));
	}

	LoadRes(os, bitmaps, this);
	LoadRes(os, actors, this);
	LoadRes(os, sprites, this);
	LoadRes(os, shortSounds, this);
	LoadRes(os, continuousSounds, this);
}

/**
 * Close the MR_ResourceLib, deleting all of the objects that were created.
 */
ResourceLib::~ResourceLib()
{
	BOOST_FOREACH(const bitmaps_t::value_type &ent, bitmaps) delete ent.second;
	BOOST_FOREACH(const actors_t::value_type &ent, actors) delete ent.second;
	BOOST_FOREACH(const sprites_t::value_type &ent, sprites) delete ent.second;
	BOOST_FOREACH(const shortSounds_t::value_type &ent, shortSounds) delete ent.second;
	BOOST_FOREACH(const continuousSounds_t::value_type &ent, continuousSounds) delete ent.second;

	delete recordFile;
}

MR_ResBitmap *ResourceLib::GetBitmap(int id)
{
	bitmaps_t::const_iterator iter = bitmaps.find(id);
	return (iter == bitmaps.end()) ? NULL : iter->second;
}

const MR_ResActor *ResourceLib::GetActor(int id)
{
	actors_t::const_iterator iter = actors.find(id);
	return (iter == actors.end()) ? NULL : iter->second;
}

const MR_ResSprite *ResourceLib::GetSprite(int id)
{
	sprites_t::const_iterator iter = sprites.find(id);
	return (iter == sprites.end()) ? NULL : iter->second;
}

const MR_ResShortSound *ResourceLib::GetShortSound(int id)
{
	shortSounds_t::const_iterator iter = shortSounds.find(id);
	return (iter == shortSounds.end()) ? NULL : iter->second;
}

const MR_ResContinuousSound *ResourceLib::GetContinuousSound(int id)
{
	continuousSounds_t::const_iterator iter = continuousSounds.find(id);
	return (iter == continuousSounds.end()) ? NULL : iter->second;
}

}  // namespace HoverRace
}  // namespace ObjFacTools
