
// Track.cpp
//
// Copyright (c) 2010, 2014-2015 Michael Imamura.
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

#include "../Display/SpriteTextureRes.h"
#include "../Parcel/RecordFile.h"
#include "../Util/InspectMapNode.h"
#include "../Util/Log.h"
#include "GameOptions.h"
#include "Level.h"

#include "Track.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Model {

Track::Physics::Physics() :
	gravity(1.0)
{
}

/**
 * Constructor.
 * @param name The name of the track.
 * @param recFile The record file to load the track from
 *                (may be @c nullptr if in-memory only).
 * @throw Parcel::ObjStreamExn
 */
Track::Track(const std::string &name,
	std::shared_ptr<Parcel::RecordFile> recFile) :
	SUPER(), recFile(std::move(recFile)), offset(0, 0), size(0, 0), map(),
	physics()
{
	LoadHeader();
	header.name = name;
}

Track::~Track()
{
}

void Track::Inspect(Util::InspectMapNode &node) const
{
	node.
		AddSubobject("metadata", &header);
}

void Track::LoadHeader()
{
	if (recFile) {
		recFile->SelectRecord(0);
		header.Serialize(*recFile->StreamIn());
	}
}

void Track::LoadLevel(bool allowRendering, const GameOptions &gameOpts)
{
	using namespace HoverRace::Parcel;

	level.reset(new Level(*this, allowRendering, gameOpts.ToFlags()));

	if (recFile) {
		recFile->SelectRecord(1);
		ObjStreamPtr archivePtr(recFile->StreamIn());
		ObjStream &archive = *archivePtr;
		level->Serialize(archive);
	}
}

void Track::LoadMap()
{
	using namespace HoverRace::Parcel;

	if (!recFile) {
		HR_LOG(debug) << "Skipping map load for in-memory track: " <<
			header.name;
		return;
	}

	if (recFile->GetNbRecords() < 4) {
		HR_LOG(warning) << "Track does not have a map: " << header.name;
		return;
	}

	recFile->SelectRecord(3);
	ObjStreamPtr archivePtr(recFile->StreamIn());
	ObjStream &archive = *archivePtr;

	//TODO: Refactor into shared code with TrackBundle::LoadMap().

	MR_Int32 x0, x1, y0, y1;
	archive >> x0 >> x1 >> y0 >> y1;
	HR_LOG(debug) << "Track bounds: "
		"nw = <" << x0 << ", " << y0 << ">  "
		"se = <" << x1 << ", " << y1 << ">";

	offset.x = static_cast<double>(x0);
	offset.y = static_cast<double>(y0);
	size.x = static_cast<double>(x1) - offset.x;
	size.y = static_cast<double>(y1) - offset.y;

	map = std::make_shared<Display::SpriteTextureRes>(
		"map:" + header.name, archive);
}

void Track::Load(bool allowRendering, const GameOptions &gameOpts)
{
	physics = Physics();
	level.reset();
	map.reset();

	LoadLevel(allowRendering, gameOpts);
	LoadMap();
}

}  // namespace Model
}  // namespace HoverRace
