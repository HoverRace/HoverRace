
// MetaSession.h
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
			class SessionPeer;
		}
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Native base class for scripted sessions.
 * @author Michael Imamura
 */
class MetaSession
{
	protected:
		MetaSession(std::shared_ptr<SessionPeer> session) :
			session(std::move(session)) { }
	public:
		virtual ~MetaSession() { }

	public:
		virtual void OnInit() { }

		// Phases
		virtual void OnPregame() { }
		virtual void OnPostgame() { }

	public:
		static void Register(Script::Core *scripting);

	private:
		std::shared_ptr<SessionPeer> session;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
