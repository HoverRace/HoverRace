
// ClientScriptCore.cpp
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

#include "../../StdAfx.h"

#include <luabind/class_info.hpp>
#include <luabind/operator.hpp>

#include "../../../engine/Display/UiFont.h"
#include "../../../engine/Util/Clock.h"
#include "../../../engine/Util/Duration.h"
#include "../../../engine/Util/Stopwatch.h"

#include "ConfigPeer.h"
#include "DebugPeer.h"
#include "GamePeer.h"
#include "HudPeer.h"
#include "InputPeer.h"
#include "MetaPlayer.h"
#include "MetaSession.h"
#include "PlayerPeer.h"
#include "SessionPeer.h"
#include "StyleEnv.h"
#include "TrackPeer.h"

#include "ClientScriptCore.h"

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Register miscellaneous classes that don't have peers and aren't
 * associated with anything else.
 */
void ClientScriptCore::RegisterMiscClasses()
{
	using namespace luabind;
	lua_State *L = GetState();

	{
		using namespace HoverRace::Display;
		using Style = UiFont::Style;

		module(L) [
			class_<UiFont, UiFont*>("UiFont")
				.enum_("Style") [
					value("PLAIN", 0),
					value("BOLD", Style::BOLD),
					value("ITALIC", Style::ITALIC),
					value("BOLD_ITALIC", Style::BOLD | Style::ITALIC)
				]
				.def(tostring(self))
				.def_readwrite("name", &UiFont::name)
				.def_readwrite("size", &UiFont::size)
				.def_readwrite("style", &UiFont::style)
		];
	}
	{
		using namespace HoverRace::Util;
		module(L) [
			class_<Clock, std::shared_ptr<Clock>>("Clock")
				.def(tostring(self))
				.property("time", &Clock::GetTime),
			class_<Duration, std::shared_ptr<Duration>>("Duration")
				.def(constructor<OS::timestamp_t, OS::timestamp_t>())
				.def(tostring(self))
				.def(self == other<Duration>())
				.def(self < other<Duration>())
				.def(self <= other<Duration>())
				.def("fmt_long", (std::string(Duration::*)()const)&Duration::FmtLong)
				.def("fmt_short", (std::string(Duration::*)()const)&Duration::FmtShort),
			class_<Stopwatch, std::shared_ptr<Stopwatch>>("Stopwatch")
				.def(constructor<std::shared_ptr<Clock>>())
				.def(constructor<std::shared_ptr<Clock>, const Duration&>())
				.def("next_lap", &Stopwatch::NextLap)
		];
	}
}

Script::Core *ClientScriptCore::Reset()
{
	SUPER::Reset();

	if (!classesRegistered) {

#		ifdef _DEBUG
			lua_State *L = GetState();
			// Enable class inspection.
			luabind::bind_class_info(L);
#		endif

		ConfigPeer::Register(this);
		DebugPeer::Register(this);
		GamePeer::Register(this);
		HudPeer::Register(this);
		InputPeer::Register(this);
		MetaPlayer::Register(this);
		MetaSession::Register(this);
		PlayerPeer::Register(this);
		SessionPeer::Register(this);
		StyleEnv::Register(this);
		TrackPeer::Register(this);
		RegisterMiscClasses();

		classesRegistered = true;
	}

	return this;
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
