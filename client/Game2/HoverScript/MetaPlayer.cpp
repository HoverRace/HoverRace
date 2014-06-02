
// MetaPlayer.cpp
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

#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../../../engine/Script/Core.h"
#include "../../../engine/Script/Wrapper.h"
#include "../../../engine/Util/Log.h"

#include "MetaSession.h"
#include "PlayerPeer.h"

#include "MetaPlayer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	class Wrapper : public MetaPlayer, public Script::Wrapper
	{
		typedef MetaPlayer SUPER;
		public:
			Wrapper(std::shared_ptr<PlayerPeer> player) : SUPER(player) { }
			virtual ~Wrapper() { }

		public:
			void OnInit() override { pcall<void>("on_init"); }
			static void OnInit_def(SUPER *super) { super->SUPER::OnInit(); }

			void OnJoined(std::shared_ptr<MetaSession> session) override { pcall<void>("on_joined", session); }
			static void OnJoined_def(SUPER *super, std::shared_ptr<MetaSession> session) { super->SUPER::OnJoined(session); }

			void OnStart() override { pcall<void>("on_start"); }
			static void OnStart_def(SUPER *super) { super->SUPER::OnStart(); }

			void OnFinishLine() override { pcall<void>("on_finish_line"); }
			static void OnFinishLine_def(SUPER *super) { super->SUPER::OnFinishLine(); }

			void OnFinish() override { pcall<void>("on_finish"); }
			static void OnFinish_def(SUPER *super) { super->SUPER::OnFinish(); }
	};
}

MetaPlayer::MetaPlayer(std::shared_ptr<PlayerPeer> player) :
	player(std::move(player))
{
	this->player->SetMeta(this);

	finishLineConn = this->player->GetPlayer()->GetFinishLineSignal().connect(
		[&](MainCharacter::MainCharacter*) {
			OnFinishLine();
		});
}

MetaPlayer::~MetaPlayer()
{
	player->SetMeta(nullptr);
}

/**
 * Register this peer in an environment.
 */
void MetaPlayer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L)[
		class_<MetaPlayer, Wrapper, std::shared_ptr<MetaPlayer>>("MetaPlayer")
			.def(constructor<std::shared_ptr<PlayerPeer>>())
			.def("on_init", &MetaPlayer::OnInit, &Wrapper::OnInit_def)
			.def("on_joined", &MetaPlayer::OnJoined, &Wrapper::OnJoined_def)
			.def("on_start", &MetaPlayer::OnStart, &Wrapper::OnStart_def)
			.def("on_finish_line", &MetaPlayer::OnFinishLine, &Wrapper::OnFinishLine_def)
			.def("on_finish", &MetaPlayer::OnFinish, &Wrapper::OnFinish_def)
			.def_readonly("player", &MetaPlayer::player)
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
