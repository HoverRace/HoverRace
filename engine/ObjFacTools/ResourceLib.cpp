
// ResourceLib.cpp
//
// Copyright (c) 2010, 2014 Michael Imamura.
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
#include "../Parcel/ObjStream.h"

#include "ResourceLib.h"

using namespace HoverRace::Parcel;

namespace HoverRace {
namespace ObjFacTools {

namespace {
	template<class T>
	void NewRes(T *val, ObjStream &os, ResourceLib*)
	{
		val->Serialize(os);
	}

	template<>
	void NewRes<ResActor>(ResActor *val, ObjStream &os, ResourceLib *self)
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

			res.insert(typename std::map<int, T*>::value_type(key, val));
		}
	}
}

/**
 * Constructor.
 * @param filename The resource data file.
 */
ResourceLib::ResourceLib(const Util::OS::path_t &filename)
{
	recordFile = new ClassicRecordFile();

	if (!recordFile->OpenForRead(filename)) {
		throw ObjStreamExn(filename, _("File not found or not readable"));
	}

	recordFile->SelectRecord(0);

	ObjStreamPtr osPtr(recordFile->StreamIn());
	ObjStream &os = *osPtr;

	const MR_UInt32 expectedMagic = FILE_MAGIC;
	MR_UInt32 magic;
	os >> magic;
	if (magic != expectedMagic) {
		throw ObjStreamExn(filename,
			boost::str(boost::format("%s: %s %08x, %s %08x") %
            _("Invalid magic number") % _("Expected") %
				expectedMagic % _("got") % magic));
	}

	LoadRes(os, bitmaps, this);
	LoadRes(os, actors, this);
	LoadRes(os, sprites, this);
	LoadRes(os, shortSounds, this);
	LoadRes(os, continuousSounds, this);
}

ResourceLib::~ResourceLib()
{
	for (const auto &ent : bitmaps) delete ent.second;
	for (const auto &ent : actors) delete ent.second;
	for (const auto &ent : sprites) delete ent.second;
	for (const auto &ent : shortSounds) delete ent.second;
	for (const auto &ent : continuousSounds) delete ent.second;

	delete recordFile;
}

ResBitmap *ResourceLib::GetBitmap(int id)
{
	bitmaps_t::const_iterator iter = bitmaps.find(id);
	return (iter == bitmaps.end()) ? NULL : iter->second;
}

const ResActor *ResourceLib::GetActor(int id)
{
	actors_t::const_iterator iter = actors.find(id);
	return (iter == actors.end()) ? NULL : iter->second;
}

const ResSprite *ResourceLib::GetSprite(int id)
{
	sprites_t::const_iterator iter = sprites.find(id);
	return (iter == sprites.end()) ? NULL : iter->second;
}

const ResShortSound *ResourceLib::GetShortSound(int id)
{
	shortSounds_t::const_iterator iter = shortSounds.find(id);
	return (iter == shortSounds.end()) ? NULL : iter->second;
}

const ResContinuousSound *ResourceLib::GetContinuousSound(int id)
{
	continuousSounds_t::const_iterator iter = continuousSounds.find(id);
	return (iter == continuousSounds.end()) ? NULL : iter->second;
}

}  // namespace HoverRace
}  // namespace ObjFacTools
