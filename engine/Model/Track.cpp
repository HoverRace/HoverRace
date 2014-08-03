
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

#include "../Display/SpriteTextureRes.h"
#include "../Parcel/RecordFile.h"
#include "../Util/InspectMapNode.h"
#include "../Util/Log.h"

#include "Level.h"

#include "Track.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Model {

/**
 * Constructor.
 * @param name The name of the track.
 * @param recFile The record file to load the track from (may not be @c NULL).
 * @throw Parcel::ObjStreamExn
 */
Track::Track(const std::string &name, Parcel::RecordFilePtr recFile) :
	SUPER(), recFile(recFile), level(nullptr), offset(0, 0), size(0, 0), map()
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

void Track::LoadLevel(bool allowRendering, char gameOpts)
{
	using namespace HoverRace::Parcel;

	level = new Level(allowRendering, gameOpts);

	recFile->SelectRecord(1);
	ObjStreamPtr archivePtr(recFile->StreamIn());
	ObjStream &archive = *archivePtr;
	level->Serialize(archive);
}

void Track::LoadMap()
{
	using namespace HoverRace::Parcel;

	if (recFile->GetNbRecords() < 4) {
		Log::Warn("Track does not have a map: %s", header.name.c_str());
		return;
	}

	recFile->SelectRecord(3);
	ObjStreamPtr archivePtr(recFile->StreamIn());
	ObjStream &archive = *archivePtr;

	//TODO: Refactor into shared code with TrackBundle::LoadMap().

	MR_Int32 x0, x1, y0, y1;
	archive >> x0 >> x1 >> y0 >> y1;
	Log::Debug("Track bounds: x = <%d, %d>  y = <%d, %d>", x0, y0, x1, y1);

	offset.x = static_cast<double>(x0);
	offset.y = static_cast<double>(y0);
	size.x = static_cast<double>(x1) - offset.x;
	size.y = static_cast<double>(y1) - offset.y;
	
	map = std::make_shared<Display::SpriteTextureRes>(
		"map:" + header.name, archive);
}

void Track::Load(bool allowRendering, char gameOpts)
{
	if (level) delete level;
	if (map) map.reset();

	LoadLevel(allowRendering, gameOpts);
	LoadMap();
}

}  // namespace Model
}  // namespace HoverRace
