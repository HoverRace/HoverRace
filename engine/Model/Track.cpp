
// Track.cpp
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

#include "../Parcel/RecordFile.h"
#include "../Util/InspectMapNode.h"

#include "Level.h"

#include "Track.h"

namespace HoverRace {
namespace Model {

/**
 * Constructor.
 * @param name The name of the track.
 * @param recFile The record file to load the track from (may not be @c NULL).
 * @throw Parcel::ObjStreamExn
 */
Track::Track(const std::string &name, Parcel::RecordFilePtr recFile) :
	SUPER(), recFile(recFile), level(nullptr)
{
	recFile->SelectRecord(0);
	header.Serialize(*recFile->StreamIn());
	header.name = name;
}

Track::~Track()
{
	if (level) delete level;
}

void Track::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddSubobject("metadata", &header);
}

void Track::Load(bool allowRendering, char gameOpts)
{
	using namespace HoverRace::Parcel;

	if (level) delete level;

	level = new Level(allowRendering, gameOpts);
	recFile->SelectRecord(1);

	ObjStreamPtr archivePtr(recFile->StreamIn());
	ObjStream &lArchive = *archivePtr;

	level->Serialize(lArchive);
}

}  // namespace Model
}  // namespace HoverRace
