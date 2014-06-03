// ClientSession.h
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2013, 2014 Michael Imamura.
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
//

#pragma once

#include "../../engine/Model/GameSession.h"
#include "../../engine/VideoServices/Sprite.h"
#include "../../engine/Util/OS.h"

namespace HoverRace {
	namespace Client {
		class Rules;
	}
	namespace MainCharacter {
		class MainCharacter;
	}
	namespace Util {
		class Clock;
	}
}

namespace HoverRace {
namespace Client {

class ClientSession
{
	public:
		ClientSession(std::shared_ptr<Rules> rules=nullptr);
		virtual ~ClientSession();

	public:
		enum class Phase
		{
			PREGAME,  ///< Players are at the starting line.
			PLAYING,  ///< Players are released and the clock is running.
			POSTGAME,  ///< The first player has finished; waiting for others.
			DONE  ///< All players have finished.
		};

	protected:
		struct ChatMessage
		{
			ChatMessage() : mCreationTime(0) { }

			time_t mCreationTime;
			std::string mBuffer;
		};

	public:
		Phase GetPhase() const { return phase; }
		bool AdvancePhase(Phase nextPhase);

		// Simulation control
		virtual void Process();

		virtual BOOL LoadNew(const char *pTitle, Parcel::RecordFilePtr pMazeFile, VideoServices::VideoBuffer *pVideo);

		// Main character control and interrogation
		bool CreateMainCharacter(int i);

		std::shared_ptr<Util::Clock> GetClock() { return clock; }
		virtual void SetSimulationTime(MR_SimulationTime pTime);
		MR_SimulationTime GetSimulationTime() const;
		void UpdateCharacterSimulationTimes();

		const MR_UInt8 *GetBackImage() const;

		void SetMap(VideoServices::Sprite *pMap, int pX0, int pY0, int pX1, int pY1);
		const VideoServices::Sprite *GetMap() const;
		void ConvertMapCoordinate(int &pX, int &pY, int pRatio) const;

		virtual int ResultAvaillable() const;	  // Return the number of players desc avail
		virtual void GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbLap, MR_SimulationTime &pFinishTime, MR_SimulationTime &pBestLap) const;
		virtual void GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbHitOther, int &pNbHitHimself) const;

		virtual int GetNbPlayers() const;
		virtual int GetRank(const MainCharacter::MainCharacter * pPlayer) const;
		virtual MainCharacter::MainCharacter *GetPlayer(int pPlayerIndex) const;

		// Chat related functions (all messages are already converted in internal ASCII
		virtual void AddMessageKey(char pKey);
		virtual void GetCurrentMessage(char *pDest) const;
		BOOL GetMessageStack(int pLevel, char *pDest, int pExpiration) const;
		void AddMessage(const char *pMessage);

		// Rendering access to level
		const Model::Level *GetCurrentLevel() const;

		std::shared_ptr<Rules> GetRules() { return rules; }

	private:
		Phase phase;

		mutable boost::mutex chatMutex;
		static const int CHAT_MESSAGE_STACK = 8;
		ChatMessage mMessageStack[CHAT_MESSAGE_STACK];

		Model::GameSession mSession;
		static const int MAX_PLAYERS = 4;
		MainCharacter::MainCharacter *mainCharacter[MAX_PLAYERS];

		MR_UInt8 *mBackImage;

		VideoServices::Sprite *mMap;
		int mX0Map;
		int mY0Map;
		int mWidthMap;
		int mHeightMap;
		int mWidthSprite;
		int mHeightSprite;

		std::shared_ptr<Util::Clock> clock;
		std::shared_ptr<Rules> rules;

		void ReadLevelAttrib(Parcel::RecordFilePtr pFile, VideoServices::VideoBuffer *pVideo);
};

}  // namespace Client
}  // namespace HoverRace
