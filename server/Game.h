// GrokkSoft HoverRace SourceCode License v1.0, November 29, 2008
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// - Redistributions in source code must retain the accompanying copyright notice,
//   this list of conditions, and the following disclaimer. 
// - Redistributions in binary form must reproduce the accompanying copyright
//   notice, this list of conditions, and the following disclaimer in the
//   documentation and/or other materials provided with the distribution. 
// - Names of the copyright holders (Richard Langlois and Grokksoft inc.) must not
//   be used to endorse or promote products derived from this software without
//   prior written permission from the copyright holders. 
// - This software, or its derivates must not be used for commercial activities
//   without prior written permission from the copyright holders. 
// - If any files are modified, you must cause the modified files to carry
//   prominent notices stating that you changed the files and the date of any
//   change. 
// 
// Disclaimer: 
//   The author makes no representations about the suitability of this software for
//   any purpose.  It is provided "AS IS", WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied.
// 

/***
 * Game.h
 * Abstraction of a game that the dedicated server might be hosting.
 *
 * @author Ryan Curtin
 */
#include <string>
#include <vector>

/***
 * Player class holds player-specific information that the Game needs to keep
 * track of.
 *
 * It has no methods and should be dealt with more like a C struct.
 */
class Player {
	public:
		std::string name;
		int id; // theoretically should not be necessary?
		int lap;
		int position;
		float *laptimes; // we can save laptimes as we get them (which HoverRace
							  // does not do at this time)
		int hitsFor;
		int hitsAgainst;

		float x;
		float y;
		float z;
	
		Connection *conn; // may be NULL if the player doesn't exist
}	

/***
 * The Game class is the abstraction of a HoverRace game, which holds
 * information relevant to the game.
 */
class Game {
	public:
		Game();

		void addPlayer(Player p);
		void removePlayer(int id);
		Player getPlayer(int id);

	private:
		int laps;
		std::string trackName;

		vector<Player> players;
};
