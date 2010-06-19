// NetworkSession.h
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
//

#pragma once

#include "ClientSession.h"
#include "RoomList.h"
#include "NetInterface.h"

namespace HoverRace {
namespace Client {

/**
 * The MR_NetworkSession is a child of the MR_ClientSession; it is a networked
 * game session.  The MR_NetworkSession class itself manages the network
 * interface, connected clients, scores, and general game data.
 */
class NetworkSession : public ClientSession
{
	typedef ClientSession SUPER;
	protected:
		class PlayerResult
		{
			public:
				int mPlayerIndex;				  // -1 = CurrentPlayer
				int mPlayerId;
				int mCraftModel;
				int mNbCompletedLap;
				MR_SimulationTime mFinishTime;
				MR_SimulationTime mBestLap;

				PlayerResult *mNext;

				// Hit Result
				int mNbTimeHit;					  // Nb of missiles and mines hit
				int mNbGoodShot;				  // nb of missiles that hit someone else

				PlayerResult *mNextHitResult;	  // Create a second list from the initial structure
		};

		BOOL mMasterMode;
		NetworkInterface mNetInterface;
		BOOL mTimeToSendCharacterCreation;		  // 0 mean sended
		BOOL mSended12SecClockUpdate;			  // User by server to adjust client clock
		BOOL mSended8SecClockUpdate;
		int mMajorID;
		int mMinorID;

		HoverRace::Client::RoomListPtr roomList;

		int mSendedPlayerStats;
		MR_FreeElementHandle mClient[NetworkInterface::eMaxClient];
		MR_MainCharacter *mClientCharacter[NetworkInterface::eMaxClient];

		int mLastSendElemStateFuncTime;
		int mLastSendElemStateTime[NetworkInterface::eMaxClient];

		PlayerResult *mResultList;
		PlayerResult *mHitList;

		char mChatEditBuffer[120];

		BOOL mInternetGame;
		HWND mWindow;

		// Awfull Ladder patch
		int mOpponendMajorID;
		int mOpponendMinorID;

		// Network functions
												  // Must be called only once
		void BroadcastMainElementCreation(const MR_ObjectFromFactoryId &pId, const MR_ElementNetState &pState, int pRoom, int pHoverId);
												  // Creation of autonomous elements
		void BroadcastAutoElementCreation(const MR_ObjectFromFactoryId &pId, const MR_ElementNetState &pState, int pRoom);
		void BroadcastPermElementState(int pPermId, const MR_ElementNetState &pState, int pRoom);
		void BroadcastMainElementState(const MR_ElementNetState &pState);
		void BroadcastMainElementStats(MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap);
		void BroadcastChatMessage(const char *pMessage);
		void BroadcastTime();
		void BroadcastHit(int pHoverIdSrc);

		void AddChatMessage(int pPlayerIndex, const char *Message, int pMessageLen);
		void AddResultEntry(int pPlayerIndex, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap);
		void AddHitEntry(int pPlayerIndex, int pPlayerFromID);
												  // helper
		void InsertHitEntry(PlayerResult *pEntry);

		void ReadNet();
		void WriteNet();

		static void ElementCreationHook(MR_FreeElement * pElement, int pRoom, void *pThis);
		static void PermElementStateHook(MR_FreeElement * pElement, int pRoom, int pPermId, void *pThis);

	public:
		// Creation and destruction
		NetworkSession(BOOL pInternetGame, int pMajorID, int pMinorID, HWND pWindow);
		~NetworkSession();

		// Simulation control
		void Process(int pSpeedFactor = 1);		  // Simulation, speed factor can be used to reduce processing speed to create AVI files

		BOOL LoadNew(const char *pTitle, HoverRace::Parcel::RecordFilePtr pMazeFile, int pNbLap, char pGameOpts, MR_VideoBuffer * pVideo);

		BOOL CreateMainCharacter();

		void SetSimulationTime(MR_SimulationTime pTime);

		void SetPlayerName(const char *pPlayerName);
		const char *GetPlayerName() const;
		void SetRoomList(HoverRace::Client::RoomListPtr roomList);
		BOOL WaitConnections(HWND pWindow, const char *pTrackName, BOOL pPromptForPort = TRUE,
			unsigned pDefaultPort = HoverRace::Util::Config::GetInstance()->net.tcpServPort,
			HWND * pModalessDlg = NULL, int pReturnMessage = 0);
		BOOL PreConnectToServer(HWND pWindow, std::string &pTrackName);
		BOOL ConnectToServer(HWND pWindow, const char *pServerIP = NULL,
			unsigned pPort = HoverRace::Util::Config::GetInstance()->net.tcpServPort,
			const char *pGameName = NULL, HWND * pModalessDlg = NULL,
			int pReturnMessage = 0);

		int ResultAvaillable() const;			  // Return the number of players desc avail
		void GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbLap, MR_SimulationTime & pFinishTime, MR_SimulationTime & pBestLap) const;
		void GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbHitOther, int &pNbHitHimself) const;

		int GetNbPlayers() const;
		int GetRank(const MR_MainCharacter * pPlayer) const;

		void AddMessageKey(char pKey);
		void GetCurrentMessage(char *pDest) const;

		MR_MainCharacter *GetPlayer(int pPlayerIndex) const;
};

}  // namespace Client
}  // namespace HoverRace
