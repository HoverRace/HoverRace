
// TrackPeer.cpp
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include <luabind/operator.hpp>

#include "../../engine/Model/Track.h"
#include "../../engine/Script/Core.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/Str.h"

#include "TrackPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

std::ostream &operator<<(std::ostream &os, const TrackPeer &peer)
{
	os << "TrackPeer{name=\"" << Str::Lua(peer.LGetName()) << "\"}";
	return os;
}

TrackPeer::TrackPeer(Script::Core *scripting,
                     std::shared_ptr<Model::Track> track) :
	SUPER(scripting, "Track"), track(track)
{
}

TrackPeer::~TrackPeer()
{
}

/**
 * Register this peer in an environment.
 * @param scripting The target environment.
 */
void TrackPeer::Register(Script::Core &scripting)
{
	using namespace luabind;
	lua_State *L = scripting.GetState();

	module(L)[
		class_<TrackPeer, SUPER, std::shared_ptr<TrackPeer>>("Track")
			.def(tostring(self))
			.property("description", &TrackPeer::LGetDescription)
			.def("get_bounds", &TrackPeer::LGetBounds)
			.property("gravity", &TrackPeer::LGetGravity, &TrackPeer::LSetGravity)
			.property("name", &TrackPeer::LGetName)
	];
}

const std::string &TrackPeer::LGetDescription() const
{
	return track->GetHeader().description;
}

void TrackPeer::LGetBounds() const
{
	lua_State *L = GetScripting()->GetState();

	const auto &offset = track->GetOffset();
	const auto &size = track->GetSize();

	lua_pushnumber(L, offset.x);
	lua_pushnumber(L, offset.y);
	lua_pushnumber(L, size.x);
	lua_pushnumber(L, size.y);
}

double TrackPeer::LGetGravity() const
{
	return track->GetGravity();
}

void TrackPeer::LSetGravity(double gravity)
{
	track->SetGravity(gravity);
}

const std::string &TrackPeer::LGetName() const
{
	return track->GetHeader().name;
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
