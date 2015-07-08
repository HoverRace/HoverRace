
// Rules.h
//
// Copyright (c) 2013, 2015 Michael Imamura.
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

#include <luabind/object.hpp>

#include "../../engine/Model/GameOptions.h"

namespace HoverRace {
	namespace Client {
		class Rulebook;
	}
	namespace Model {
		class TrackEntry;
	}
}

namespace HoverRace {
namespace Client {

/**
 * The options and settings for a single game session.
 * @author Michael Imamura
 */
class Rules
{
public:
	Rules(std::shared_ptr<const Rulebook> rulebook =
		std::shared_ptr<const Rulebook>());

public:
	std::shared_ptr<const Rulebook> GetRulebook() const { return rulebook; }
	void SetRulebook(std::shared_ptr<const Rulebook> rulebook);

	std::shared_ptr<const Model::TrackEntry> GetTrackEntry() const { return trackEntry; }
	void SetTrackEntry(std::shared_ptr<const Model::TrackEntry> trackEntry) { this->trackEntry = std::move(trackEntry); }

	/// Retrieve the the Lua view of the rules.
	luabind::object &GetRules() { return rules; }

	// Temporary; will be removed once all logic is moved into Lua.
	int GetLaps() const
	{
		try {
			return luabind::object_cast<int>(rules["laps"]);
		} catch (luabind::cast_failed&) {
			return 1;
		}
	}
	void SetLaps(int laps) { rules["laps"] = laps; }
	const Model::GameOptions &GetGameOpts() const { return gameOpts; }
	void SetGameOpts(const Model::GameOptions &gameOpts) { this->gameOpts = gameOpts; }

private:
	std::shared_ptr<const Rulebook> rulebook;
	std::shared_ptr<const Model::TrackEntry> trackEntry;

	luabind::object rules;

	Model::GameOptions gameOpts;
};

}  // namespace Client
}  // namespace HoverRace
