
// Rulebook.h
// Defines the rules of a game session.
//
// Copyright (c) 2010 Michael Imamura.
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

/**
 * Defines the rules for a particular game session.
 * @todo This is very basic right now; the idea is that later we will 
 *       make this a base class and move some game state logic into
 *       subclasses.
 */
class Rulebook
{
	public:
		Rulebook(const std::string &trackName, int laps, char gameOpts) :
			trackName(trackName), laps(laps), gameOpts(gameOpts) {}

	public:
		const std::string &GetTrackName() const { return trackName; }
		int GetLaps() const { return laps; }
		char GetGameOpts() const { return gameOpts; }

	private:
		std::string trackName;
		int laps;
		char gameOpts;
};

}  // namespace Client
}  // namespace HoverRace
