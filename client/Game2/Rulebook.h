
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

namespace HoverRace {
	namespace HoverScript {
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
			const std::string &description);

	public:
		const std::string &GetName() const { return name; }
		const std::string &GetDescription() const { return description; }

		const Model::TrackEntryPtr &GetTrackEntry() const { return trackEntry; }
		void SetTrackEntry(Model::TrackEntryPtr trackEntry) { this->trackEntry = std::move(trackEntry); }

		// Temporary; will be generalized into RulebookOptions.
		int GetLaps() const { return laps; }
		void SetLaps(int laps) { this->laps = laps; }
		char GetGameOpts() const { return gameOpts; }
		void SetGameOpts(char gameOpts) { this->gameOpts = gameOpts; }

	public:
		void OnPreGame(HoverScript::SessionPeerPtr session);
		void OnPostGame(HoverScript::SessionPeerPtr session);

	public:
		friend bool operator==(const Rulebook &lhs, const Rulebook &rhs);
		friend bool operator<(const Rulebook &lhs, const Rulebook &rhs);

	private:
		Script::Core *scripting;
		std::string name;
		std::string description;
		Model::TrackEntryPtr trackEntry;
		
		int laps;
		char gameOpts;

		Script::Handlers onPreGame;
		Script::Handlers onPostGame;
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
