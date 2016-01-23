
// PlayerPeer.h
//
// Copyright (c) 2010, 2014-2016 Michael Imamura.
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

#pragma once

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "../../../engine/Script/Handlers.h"
#include "../../../engine/Script/Peer.h"

#include "HudPeer.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class MetaPlayer;
			class SessionPeer;
		}
	}
	namespace MainCharacter {
		class MainCharacter;
	}
	namespace Player {
		class Player;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Scripting peer for players (main characters).
 * @author Michael Imamura
 */
class PlayerPeer : public Script::Peer {
	using SUPER = Script::Peer;

public:
	PlayerPeer(Script::Core &scripting, std::shared_ptr<Player::Player> player);
	virtual ~PlayerPeer() { }

public:
	static void Register(Script::Core &scripting);

public:
	Player::Player *GetPlayer() const { return player.get(); }

	void SetMeta(MetaPlayer *meta) { this->meta = meta; }
	void SetHud(std::shared_ptr<HudPeer> hud);

protected:
	MainCharacter::MainCharacter *VerifyAttached() const;

public:
	void LFinish();

	double LGetFuel() const;

	std::shared_ptr<HudPeer> LGetHud() const;

	int LGetIndex() const;

	const std::string &LGetName() const;

	void LGetPos();

private:
	std::shared_ptr<Player::Player> player;
	MetaPlayer *meta;
	std::shared_ptr<HudPeer> hud;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
