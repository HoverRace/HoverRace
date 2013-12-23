
// Rulebook.h
//
// Copyright (c) 2010, 2013 Michael Imamura.
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

#include "../../engine/Script/Handlers.h"

#include "Rule.h"

namespace HoverRace {
	namespace HoverScript {
		class PlayerPeer;
		class SessionPeer;
		typedef std::shared_ptr<SessionPeer> SessionPeerPtr;
	}
	namespace Model {
		class TrackEntry;
		typedef std::shared_ptr<TrackEntry> TrackEntryPtr;
	}
	namespace Script {
		class Core;
		class Handlers;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Defines the rules for a particular game session.
 * @author Michael Imamura
 */
class Rulebook : private boost::noncopyable
{
	public:
		Rulebook(Script::Core *scripting,
			const std::string &name,
			const std::string &title,
			const std::string &description,
			int maxPlayers);

	public:
		const std::string &GetName() const { return name; }
		const std::string &GetTitle() const { return title; }
		const std::string &GetDescription() const { return description; }
		int GetMaxPlayers() const { return maxPlayers; }

	public:
		// Temporary until we get real rule classes.
		void AddRule(const std::string &name, const luabind::object &obj);

		luabind::object CreateDefaultRules() const;

	public:
		void SetOnPreGame(const luabind::object &fn);
		void OnPreGame(HoverScript::SessionPeerPtr session) const;

		void SetOnPostGame(const luabind::object &fn);
		void OnPostGame(HoverScript::SessionPeerPtr session) const;

		void SetOnPlayerJoined(const luabind::object &fn);
		void OnPlayerJoined(HoverScript::SessionPeerPtr session,
			std::shared_ptr<HoverScript::PlayerPeer> player) const;

	public:
		friend bool operator==(const Rulebook &lhs, const Rulebook &rhs);
		friend bool operator<(const Rulebook &lhs, const Rulebook &rhs);

	private:
		Script::Core *scripting;
		std::string name;
		std::string title;
		std::string description;
		int maxPlayers;

		typedef std::map<std::string, std::shared_ptr<Rule>> rules_t;
		rules_t rules;

		Script::Handlers onPreGame;
		Script::Handlers onPostGame;
		Script::Handlers onPlayerJoined;
};
typedef std::shared_ptr<Rulebook> RulebookPtr;

inline bool operator==(const Rulebook &lhs, const Rulebook &rhs)
{
	// Rulebook names are unique.
	return lhs.name == rhs.name;
}

inline bool operator<(const Rulebook &lhs, const Rulebook &rhs)
{
	//TODO: Sort by manually-defined sort index, then by name.
	return lhs.name < rhs.name;
}

}  // namespace Client
}  // namespace HoverRace
