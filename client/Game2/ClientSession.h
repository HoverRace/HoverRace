// ClientSession.h
//
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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

class MR_MainCharacter;

namespace HoverRace {
namespace Client {

class ClientSession
{
	protected:
		class ChatMessage
		{
			public:
				time_t mCreationTime;
				std::string mBuffer;
		};

		CRITICAL_SECTION mChatMutex;
		static const int CHAT_MESSAGE_STACK = 8;
		ChatMessage mMessageStack[CHAT_MESSAGE_STACK];

		MR_GameSession mSession;
		MR_MainCharacter *mMainCharacter1;
		MR_MainCharacter *mMainCharacter2;
		MR_MainCharacter *mMainCharacter3;
		MR_MainCharacter *mMainCharacter4;

		MR_UInt8 *mBackImage;

		MR_Sprite *mMap;
		int mX0Map;
		int mY0Map;
		int mWidthMap;
		int mHeightMap;
		int mWidthSprite;
		int mHeightSprite;

		int mNbLap;
		char mGameOpts;

		// Stats counters.
		unsigned int frameCount;
		HoverRace::Util::OS::timestamp_t lastTimestamp;
		double fps;

		void ReadLevelAttrib(MR_RecordFile *pFile, MR_VideoBuffer *pVideo);
	public:
		// Creation and destruction
		ClientSession();
		virtual ~ClientSession();

		// Simulation control
												  // Simulation, speed factor can be used to reduce processing speed to create AVI files
		virtual void Process(int pSpeedFactor = 1);

		virtual BOOL LoadNew(const char *pTitle, MR_RecordFile *pMazeFile, int pNbLap, char pGameOpts, MR_VideoBuffer *pVideo);

		// Main character control and interrogation
		BOOL CreateMainCharacter();
		BOOL CreateMainCharacter2();
		BOOL CreateMainCharacter3();
		BOOL CreateMainCharacter4();

		MR_MainCharacter *GetMainCharacter() const;
		MR_MainCharacter *GetMainCharacter2() const;
		MR_MainCharacter *GetMainCharacter3() const;
		MR_MainCharacter *GetMainCharacter4() const;

		virtual void SetSimulationTime(MR_SimulationTime pTime);
		MR_SimulationTime GetSimulationTime() const;
		void SetControlState(int pState1, int pState2, int pState3, int pState4);

		const MR_UInt8 *GetBackImage() const;

		void SetMap(MR_Sprite *pMap, int pX0, int pY0, int pX1, int pY1);
		const MR_Sprite *GetMap() const;
		void ConvertMapCoordinate(int &pX, int &pY, int pRatio) const;

		virtual int ResultAvaillable() const;	  // Return the number of players desc avail
		virtual void GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbLap, MR_SimulationTime &pFinishTime, MR_SimulationTime &pBestLap) const;
		virtual void GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbHitOther, int &pNbHitHimself) const;

		virtual int GetNbPlayers() const;
		virtual int GetRank(const MR_MainCharacter * pPlayer) const;
		virtual const MR_MainCharacter *GetPlayer(int pPlayerIndex) const;

		// Chat related functions (all messages are already converted in internal ASCII
		virtual void AddMessageKey(char pKey);
		virtual void GetCurrentMessage(char *pDest) const;
		BOOL GetMessageStack(int pLevel, char *pDest, int pExpiration) const;
		void AddMessage(const char *pMessage);

		// Rendering access to level
		const MR_Level *GetCurrentLevel() const;

		// Client stats.
		void IncFrameCount();
		double GetCurrentFramerate() const;
};

}  // namespace Client
}  // namespace HoverRace
