
// MetaPlayer.h
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

#pragma once

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class MetaSession;
			class PlayerPeer;
		}
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Native base class for scripted players.
 * @author Michael Imamura
 */
class MetaPlayer
{
	public:
		MetaPlayer(std::shared_ptr<PlayerPeer> player);
		virtual ~MetaPlayer();

	public:
		std::shared_ptr<PlayerPeer> GetPlayer() const { return player; }

	public:
		virtual void OnInit() { }

		virtual void OnJoined(std::shared_ptr<MetaSession> session) { HR_UNUSED(session); }
		virtual void OnStart() { }
		virtual void OnCheckpoint(int i) { }
		virtual void OnFinishLine() { }
		virtual void OnFinish() { }

	public:
		static void Register(Script::Core *scripting);

	private:
		std::shared_ptr<PlayerPeer> player;
		boost::signals2::scoped_connection checkpointConn;
		boost::signals2::scoped_connection finishLineConn;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
