
// ResourceLib.cpp
//
// Copyright (c) 2010, 2014, 2015 Michael Imamura.
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
void LoadRes(ObjStream &os, std::map<int, std::unique_ptr<T>> &res, ResourceLib *self)
{
	MR_UInt32 num;
	os >> num;
	for (MR_UInt32 i = 0; i < num; ++i) {
		MR_Int32 key;
		os >> key;

		T *val = new T(key);
		NewRes(val, os, self);

		res.emplace(key, std::unique_ptr<T>(val));
	}
}

}  // namespace

/**
 * Constructor for empty library.
 */
ResourceLib::ResourceLib()
{
}

/**
 * Constructor for loaded library.
 * @param filename The resource data file.
 */
ResourceLib::ResourceLib(const Util::OS::path_t &filename) :
	recordFile(new ClassicRecordFile())
{
	if (!recordFile->OpenForRead(filename)) {
		throw ObjStreamExn(filename, "File not found or not readable");
	}

	recordFile->SelectRecord(0);

	ObjStreamPtr osPtr(recordFile->StreamIn());
	ObjStream &os = *osPtr;

	const MR_UInt32 expectedMagic = FILE_MAGIC;
	MR_UInt32 magic;
	os >> magic;
	if (magic != expectedMagic) {
		throw ObjStreamExn(filename,
			boost::str(boost::format(
				"Invalid magic number: Expected %08x, got %08x") %
				expectedMagic % magic));
	}

	LoadRes(os, bitmaps, this);
	LoadRes(os, actors, this);
	LoadRes(os, sprites, this);
	LoadRes(os, shortSounds, this);
	LoadRes(os, continuousSounds, this);
}

ResourceLib::~ResourceLib()
{
}

ResBitmap *ResourceLib::GetBitmap(int id)
{
	auto iter = bitmaps.find(id);
	return (iter == bitmaps.end()) ? nullptr : iter->second.get();
}

const ResActor *ResourceLib::GetActor(int id)
{
	auto iter = actors.find(id);
	return (iter == actors.end()) ? nullptr : iter->second.get();
}

const ResSprite *ResourceLib::GetSprite(int id)
{
	auto iter = sprites.find(id);
	return (iter == sprites.end()) ? nullptr : iter->second.get();
}

const ResShortSound *ResourceLib::GetShortSound(int id)
{
	auto iter = shortSounds.find(id);
	return (iter == shortSounds.end()) ? nullptr : iter->second.get();
}

const ResContinuousSound *ResourceLib::GetContinuousSound(int id)
{
	auto iter = continuousSounds.find(id);
	return (iter == continuousSounds.end()) ? nullptr : iter->second.get();
}

}  // namespace HoverRace
}  // namespace ObjFacTools
