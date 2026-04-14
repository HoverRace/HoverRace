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

#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include "ClientSession.h"
#include "NetInterface.h"
#include "RoomList.h"

class ISteamUser;

/**
 * The MR_NetworkSession is a child of the MR_ClientSession; it is a networked
 * game session.  The MR_NetworkSession class itself manages the network
 * interface, connected clients, scores, and general game data.
 */
class MR_NetworkSession : public MR_ClientSession
{
	protected:
		class PlayerResult
		{
			public:
				int mPlayerHoverId;
				CString mPlayerName;
				int mPlayerId;
				int mCraftModel;
				int mNbCompletedLap;
				MR_SimulationTime mFinishTime;
				MR_SimulationTime mBestLap;
				int mNbCompletedSplit;
				MR_SimulationTime mFinishFirstSplit;
				MR_SimulationTime mFirstSplitDifference;
				MR_SimulationTime mFinishSecondSplit;
				MR_SimulationTime mSecondSplitDifference;

				PlayerResult *mNext;

				// Hit Result
				int mNbTimeHit;					  // Nb of missiles and mines hit
				int mNbGoodShot;				  // nb of missiles that hit someone else

				PlayerResult *mNextHitResult;	  // Create a second list from the initial structure
		};

		BOOL mMasterMode;
		MR_NetworkInterface mNetInterface;
		BOOL mTimeToSendCharacterCreation;		  // 0 mean sended
		BOOL mSended12SecClockUpdate;			  // User by server to adjust client clock
		BOOL mSended8SecClockUpdate;
		int mMajorID;
		int mMinorID;

		HoverRace::Client::RoomListPtr roomList;

		int mSendedPlayerStats[MR_MAX_LOCAL_PLAYER];
		int mSendedCheckpointStats[MR_MAX_LOCAL_PLAYER];
		enum { eMaxRemoteHover = MR_MAX_LOCAL_PLAYER * (MR_NetworkInterface::eMaxClient + 1) };
		MR_FreeElementHandle mRemoteClient[eMaxRemoteHover];
		MR_MainCharacter *mRemoteCharacter[eMaxRemoteHover];
		int mRemoteOwnerClient[eMaxRemoteHover];

		int mLastSendElemStateFuncTime[MR_MAX_LOCAL_PLAYER];
		int mLastSendElemStateTime[MR_NetworkInterface::eMaxClient];

		PlayerResult *mResultList;
		PlayerResult *mHitList;

		char mChatEditBuffer[120];

		BOOL mInternetGame;
		HWND mWindow;
		CString mRaceHash;
		int mLastBroadcastCraftModel[MR_MAX_LOCAL_PLAYER];
		int mLocalPartySize;
		CString mLocalPartyNames[MR_MAX_LOCAL_PLAYER];
		int mLocalHoverBase;
		int mConfiguredPlayerCount;
		int mRemoteHoverBase[MR_NetworkInterface::eMaxClient];
		int mRemotePartySize[MR_NetworkInterface::eMaxClient];

		// Awfull Ladder patch
		int mOpponendMajorID;
		int mOpponendMinorID;

		// Network functions
												  // Must be called only once
		void BroadcastMainElementCreation(const MR_ObjectFromFactoryId & pId, const MR_ElementNetState & pState, int pRoom, int pHoverId);
												  // Creation of autonomous elements
		void BroadcastAutoElementCreation(const MR_ObjectFromFactoryId & pId, const MR_ElementNetState & pState, int pRoom);
		void BroadcastPermElementState(int pPermId, const MR_ElementNetState & pState, int pRoom);
		void BroadcastMainElementState(const MR_ElementNetState & pState,
			int pHoverId, int pReqLevel = MR_NET_DATAGRAM);
		void BroadcastLocalPartyStates(int pReqLevel);
		void BroadcastMainElementStats(int pHoverId, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLaps, int pNbSplits, MR_SimulationTime pFinishFirstSplit, MR_SimulationTime pFirstSplitDifference, MR_SimulationTime pFinishSecondSplit, MR_SimulationTime pSecondSplitDifference);
		void BroadcastChatMessage(const char *pMessage);
		void BroadcastTime();
		void BroadcastHit(int pVictimHoverId, int pHoverIdSrc, int pElementId);
		void DestroyElementByNetworkId(int pElementId);

		void AddChatMessage(int pPlayerIndex, const char *Message, int pMessageLen);
		void AddResultEntry(int pHoverId, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap, int pNbSplits, MR_SimulationTime pFinishFirstSplit, MR_SimulationTime pFirstSplitDifference, MR_SimulationTime pFinishSecondSplit, MR_SimulationTime pSecondSplitDifference);
		void AddHitEntry(int pVictimHoverId, int pPlayerFromID);
												  // helper
		void InsertHitEntry(PlayerResult * pEntry);
		void RefreshHoverAssignments();
		int GetHoverBaseForMachineId(int pMachineId) const;
		int GetLocalHoverId(int pLocalIndex) const;
		const char *ResolvePlayerName(int pHoverId) const;
		MR_MainCharacter *GetRemoteCharacterByHoverId(int pHoverId) const;
		MR_MainCharacter *GetRepresentativeRemoteCharacter(int pClient) const;
		void RemoveRemoteClientCharacters(int pClient);
		void ApplyRemoteMainElementState(int pClientId, int pHoverId,
			const MR_UInt8 *pStateData, int pStateLen);

		void ReadNet();
		void WriteNet();

		static void ElementCreationHook(MR_FreeElement * pElement, int pRoom, void *pThis);
		static void PermElementStateHook(MR_FreeElement * pElement, int pRoom, int pPermId, void *pThis);

	public:
		// Creation and destruction
		MR_NetworkSession(BOOL pInternetGame, int pMajorID, int pMinorID, HWND pWindow);
		~MR_NetworkSession();

		// Simulation control
		BOOL Process(int pSpeedFactor = 1);		  // Simulation, speed factor can be used to reduce processing speed to create AVI files

		BOOL LoadNew(const char *pTitle, MR_RecordFile * pMazeFile, int pNbLap,
			BOOL pAllowWeapons, BOOL pAllowCans, BOOL pAllowMines,
			unsigned pAllowedCraftMask,
			const MR_GameRuleSettings &pGameRuleSettings,
			MR_VideoBuffer * pVideo);

		BOOL CreateMainCharacter();

		void SetSimulationTime(MR_SimulationTime pTime);

		void SetPlayerName(const char *pPlayerName);
		const char *GetPlayerName() const;
		void SetLocalParty(int pPartySize, const std::string *pPartyNames);
		int GetLocalPartySize() const;
		const char *GetLocalPartyName(int pIndex) const;
		void SetRaceHash(const char *pRaceHash);
		const char *GetRaceHash() const;
		void SetRoomList(HoverRace::Client::RoomListPtr roomList);
		BOOL WaitConnections(HWND pWindow, const char *pGameName, BOOL pPromptForPort = TRUE, unsigned pDefaultPort = MR_Config::GetInstance()->net.tcpServPort, HWND * pModalessDlg = NULL, int pReturnMessage = 0, const char *pTrackName = NULL, int pNbLap = -1, BOOL pHasWeapons = FALSE, BOOL pAllowWeapons = FALSE, BOOL pHasCans = FALSE, BOOL pAllowCans = FALSE, BOOL pHasMines = FALSE, BOOL pAllowMines = FALSE);
		BOOL WaitConnections(HWND pWindow, const char *pGameName, BOOL pPromptForPort, unsigned pDefaultPort, HWND * pModalessDlg, int pReturnMessage, const char *pTrackName, int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts, unsigned pAllowedCraftMask);
		BOOL WaitConnections(HWND pWindow, const char *pGameName, BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg, int pReturnMessage, const char *pTrackName, int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts, unsigned pAllowedCraftMask, const MR_GameRuleSettings &pGameRuleSettings);
		BOOL PreConnectToServer(HWND pWindow, CString & pTrackName);
		BOOL ConnectToServer(HWND pWindow, const char *pServerIP = NULL, unsigned pPort = MR_Config::GetInstance()->net.tcpServPort, uint64 pSteamID = 0, const char *pGameName = NULL, HWND * pModalessDlg = NULL, int pReturnMessage = 0, const char *pTrackName = NULL, int pNbLap = -1, BOOL pHasWeapons = FALSE, BOOL pAllowWeapons = FALSE, BOOL pHasCans = FALSE, BOOL pAllowCans = FALSE, BOOL pHasMines = FALSE, BOOL pAllowMines = FALSE);
		BOOL ConnectToServer(HWND pWindow, const char *pServerIP, unsigned pPort, uint64 pSteamID, const char *pGameName, HWND *pModalessDlg, int pReturnMessage, const char *pTrackName, int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts, unsigned pAllowedCraftMask);
		BOOL ConnectToServer(HWND pWindow, const char *pServerIP, unsigned pPort, uint64 pSteamID, const char *pGameName, HWND *pModalessDlg, int pReturnMessage, const char *pTrackName, int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts, unsigned pAllowedCraftMask, const MR_GameRuleSettings &pGameRuleSettings);

		int ResultAvaillable() const;			  // Return the number of players desc avail
		void GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbLap, MR_SimulationTime & pFinishTime, MR_SimulationTime & pBestLap, int &pNbSplit, MR_SimulationTime & pFinishFirstSplit, MR_SimulationTime & pFirstSplitDifference, MR_SimulationTime & pFinishSecondSplit, MR_SimulationTime & pSecondSplitDifference) const;
		void GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbHitOther, int &pNbHitHimself) const;

		int GetNbPlayers() const;
		int GetRank(const MR_MainCharacter * pPlayer) const;

		void AddMessageKey(char pKey);
		void GetCurrentMessage(char *pDest) const;

		const MR_MainCharacter *GetPlayer(int pPlayerIndex) const;
		void SetPlayerCraftCollision(int pHoverId, BOOL pEnabled);
		void SetPlayerColumnInteraction(int pHoverId, BOOL pEnabled);
		void SetPlayerRenderOpacity(int pHoverId, float pOpacity);
		BOOL IsLocalHoverId(int pHoverId) const;
		const char *GetPlayerDisplayName(int pHoverId) const;
		const MR_MainCharacter *FindPlayerByHoverId(int pHoverId) const;
		BOOL ShouldProcessLocalHitQueues() const;
};
#endif
