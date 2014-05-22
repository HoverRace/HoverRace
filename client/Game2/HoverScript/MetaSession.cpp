
// MetaSession.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Log.h"

#include "SessionPeer.h"

#include "MetaSession.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	class Wrapper : public MetaSession, public luabind::wrap_base
	{
		typedef MetaSession SUPER;
		public:
			Wrapper(std::shared_ptr<SessionPeer> session) : SUPER(session) { }
			virtual ~Wrapper() { }

		public:
			void OnInit() override { call<void>("on_init"); }
			static void OnInit_def(SUPER *super) { super->SUPER::OnInit(); }
	};
}

/**
* Register this peer in an environment.
*/
void MetaSession::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L)[
		class_<MetaSession, Wrapper, std::shared_ptr<MetaSession>>("MetaSession")
			.def(constructor<std::shared_ptr<SessionPeer>>())
			.def("on_init", &MetaSession::OnInit, &Wrapper::OnInit_def)
			.def_readonly("session", &MetaSession::session)
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
