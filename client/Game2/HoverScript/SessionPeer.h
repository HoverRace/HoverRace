
// SessionPeer.h
//
// Copyright (c) 2010, 2013-2016 Michael Imamura.
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

#include "../../../engine/Util/WorldCoordinates.h"
#include "../../../engine/Script/Peer.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class MetaPlayer;
			class MetaSession;
			class TrackPeer;
		}
		class ClientSession;
	}
	namespace Script {
		class Core;
	}
	namespace Util {
		class Clock;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Scripting peer for a game session.
 * @author Michael Imamura
 */
class SessionPeer : public Script::Peer
{
	using SUPER = Script::Peer;

protected:
	class PlayerRef
	{
	public:
		PlayerRef(std::shared_ptr<MetaPlayer> meta);

	public:
		std::shared_ptr<MetaPlayer> meta;
	private:
		boost::signals2::scoped_connection startedConn;
		boost::signals2::scoped_connection finishedConn;
	};

public:
	SessionPeer(Script::Core &scripting, ClientSession *session);
	virtual ~SessionPeer();

public:
	static void Register(Script::Core &scripting);

public:
	void SetMeta(MetaSession *meta) { this->meta = meta; }

public:
	template<class Fn>
	void ForEachPlayer(Fn fn)
	{
		for (auto &ref : playerRefs) {
			fn(ref->meta);
		}
	}
	std::shared_ptr<MetaPlayer> &GetPlayer(unsigned idx)
	{
		return playerRefs[idx]->meta;
	}

	void OnSessionStart(ClientSession *session);
	void OnSessionEnd();

protected:
	void VerifySession() const;

public:
	int LGetNumPlayers() const;
	MR_SimulationTime LGetTime() const;
	std::shared_ptr<Util::Clock> LGetClock() const;
	std::shared_ptr<Util::Clock> LGetCountdown() const;
	void LCountdownToNextPhase(const std::string &s) const;
	std::shared_ptr<TrackPeer> LGetTrack() const;

private:
	ClientSession *session;
	MetaSession *meta;
	luabind::object rules;
	luabind::object players;
	std::vector<std::unique_ptr<PlayerRef>> playerRefs;
};
typedef std::shared_ptr<SessionPeer> SessionPeerPtr;

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
