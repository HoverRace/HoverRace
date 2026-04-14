// NetworkSession.cpp
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

#include "stdafx.h"

#include <Mmsystem.h>
#include "NetworkSession.h"
#include "InternetRoom.h"
#include "TrackSelect.h"
#include "resource.h"
#include "../Util/StrRes.h"

// Messages
#define MRNM_SET_TIME                 1
#define MRNM_CREATE_MAIN_ELEM         2
#define MRNM_SET_MAIN_ELEM_STATE      3
#define MRNM_CREATE_AUTO_ELEM         4
// #define MRNM_FINISH_STATS          5
#define MRNM_CHAT_MESSAGE             6
#define MRNM_PLAYER_STATS             7
#define MRNM_SET_PERM_ELEMENT_STATE   8
#define MRNM_SEND_KEYID               9
#define MRNM_HIT_MESSAGE             10
#define MRNM_SET_MAIN_ELEM_STATE_BATCH 11

// Local structures
class MR_PlayerStats
{
	public:
		MR_Int16 mHoverId;

		MR_SimulationTime mFinishTime;
		MR_SimulationTime mBestLap;
		MR_Int16 mCompletedLaps;

		MR_Int8 mCompletedSplits;

		MR_SimulationTime mFinishFirstSplit;
		MR_SimulationTime mFirstSplitDifference;

		MR_SimulationTime mFinishSecondSplit;
		MR_SimulationTime mSecondSplitDifference;
};

class MR_HitMessage
{
	public:
		MR_Int16 mVictimHoverId;
		MR_Int16 mHoverIdSrc;
		MR_Int32 mElementId;
};

/**
 * Initializes the MR_NetworkSession.  Sets default values; main character
 * creation will be sent at least 5 seconds before the game starts.
 */
MR_NetworkSession::MR_NetworkSession(BOOL pInternetGame, int pMajorID, int pMinorID, HWND pWindow)
:MR_ClientSession()
{
	mMasterMode = FALSE;
	mInternetGame = pInternetGame;
	mMajorID = pMajorID;
	mMinorID = pMinorID;
	mWindow = pWindow;

	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		mLastSendElemStateTime[lCounter] = timeGetTime();
		mRemoteHoverBase[lCounter] = -1;
		mRemotePartySize[lCounter] = 0;
	}
	for(int lCounter = 0; lCounter < eMaxRemoteHover; lCounter++) {
		mRemoteCharacter[lCounter] = NULL;
		mRemoteClient[lCounter] = NULL;
		mRemoteOwnerClient[lCounter] = -1;
	}
	for(int lCounter = 0; lCounter < MR_MAX_LOCAL_PLAYER; lCounter++) {
		mLastSendElemStateFuncTime[lCounter] = timeGetTime();
	}

	mResultList = NULL;
	mHitList = NULL;
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mSendedPlayerStats[i] = 0;
		mSendedCheckpointStats[i] = 1;
		mLastBroadcastCraftModel[i] = -1;
	}

	mChatEditBuffer[0] = 0;
	mRaceHash = "";
	mLocalPartySize = 1;
	mLocalPartyNames[0] = "Player";
	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mLocalPartyNames[i] = "";
	}
	mLocalHoverBase = 0;
	mConfiguredPlayerCount = 1;

	mTimeToSendCharacterCreation = -5000;		  // send at least 5 sec before game start

	// Awful Ladder patch
	mOpponendMajorID = -1;
	mOpponendMinorID = -1;
}

/**
 * Destroy the MR_NetworkSession.  Clean things up, report the match.
 */
MR_NetworkSession::~MR_NetworkSession()
{
	if(mInternetGame && (mMainCharacter1 != NULL) && IsStandardGameRule()) {
		int lCurrentModel = mMainCharacter1->GetHoverModel();

		// Send results to the record server
		// regardless of registration info
		//if(mMajorID != -1) {
			/*
			   PlayerResult* lBest    = NULL;
			   PlayerResult* lCurrent = mResultList;

			   // Verify if we have the best lap
			   while( lCurrent!= NULL )
			   {
			   if(  (lCurrent->mCraftModel == lCurrentModel )
			   &&(lCurrent->mBestLap>0)
			   &&((lBest==NULL)||(lCurrent->mBestLap<lBest->mBestLap )))
			   {
			   lBest = lCurrent;
			   }
			   lCurrent = lCurrent->mNext;
			   }
			 */

			// Find current user results
			int lNbPlayer = 0;
			PlayerResult *lCurrent = mResultList;
			PlayerResult *lPlayer = NULL;

			// Verify if we have the best lap
			while(lCurrent != NULL) {
				if((mMainCharacter1 != NULL) &&
					(lCurrent->mPlayerHoverId == mMainCharacter1->GetHoverId()))
				{
					lPlayer = lCurrent;
				}
				lCurrent = lCurrent->mNext;
				lNbPlayer++;
			}

			if(lPlayer != NULL) {

				int lTotalLap = mMainCharacter1->GetTotalLap();

				MR_SendRaceResult(mWindow, mSession.GetTitle(), lPlayer->mBestLap,
					mMajorID, mMinorID, mNetInterface.GetPlayerName(-1),
					mSession.GetCurrentMazeFile()->GetCheckSum(),
					mMainCharacter1->GetHoverModel(),
					(lPlayer->mNbCompletedLap == -1) ? lPlayer->mFinishTime : 0,
					(lPlayer->mNbCompletedLap == -1) ? lTotalLap : 0,
					lNbPlayer, roomList, GetRaceHash());

				// Report ladder matchs
				if(lNbPlayer == 2) {
					const int lWinnerHoverId = mResultList->mPlayerHoverId;
					// If you lost and the other player have finished
					if((mMainCharacter1 != NULL) &&
						(lWinnerHoverId != mMainCharacter1->GetHoverId()) &&
						(mResultList->mNbCompletedLap == -1))
					{
						// Report the Lost
						MR_SendLadderResult(mWindow, ResolvePlayerName(lWinnerHoverId), mOpponendMajorID, mOpponendMinorID, mNetInterface.GetPlayerName(-1), mMajorID, mMinorID, mSession.GetTitle(), lTotalLap);

					}
				}
			}
		//}
	}

	PlayerResult *lCurrent = mResultList;

	mResultList = NULL;

	while(lCurrent != NULL) {
		PlayerResult *lPrev = lCurrent;

		lCurrent = lCurrent->mNext;
		delete lPrev;
	}

}

/**
 * Return the number of results that are available.  Richard can't spell.
 */
int MR_NetworkSession::ResultAvaillable() const
{
	int lReturnValue = 0;

	PlayerResult *lCurrent = mResultList;

	while(lCurrent != NULL) {
		lReturnValue++;
		lCurrent = lCurrent->mNext;
	}
	return lReturnValue;
}

void MR_NetworkSession::RefreshHoverAssignments()
{
	int lNextHoverId = 0;
	const int lLocalMachineId = mNetInterface.GetId();

	mLocalHoverBase = 0;
	for(int i = 0; i < MR_NetworkInterface::eMaxClient; ++i) {
		mRemoteHoverBase[i] = -1;
		mRemotePartySize[i] = 0;
	}

	for(int lMachineId = 0; lMachineId <= MR_NetworkInterface::eMaxClient; ++lMachineId) {
		if(lMachineId == lLocalMachineId) {
			mLocalHoverBase = lNextHoverId;
			lNextHoverId += mLocalPartySize;
		}
		else {
			const int lClient = mNetInterface.GetClientForMachineId(lMachineId);
			if((lClient >= 0) && mNetInterface.IsConnected(lClient)) {
				mRemoteHoverBase[lClient] = lNextHoverId;
				mRemotePartySize[lClient] = mNetInterface.GetRemotePartySize(lClient);
				lNextHoverId += mRemotePartySize[lClient];
			}
		}
	}

	mConfiguredPlayerCount = max(1, lNextHoverId);
}

int MR_NetworkSession::GetHoverBaseForMachineId(int pMachineId) const
{
	if(pMachineId == mNetInterface.GetId()) {
		return mLocalHoverBase;
	}

	const int lClient = mNetInterface.GetClientForMachineId(pMachineId);
	if((lClient < 0) || (lClient >= MR_NetworkInterface::eMaxClient)) {
		return -1;
	}

	return mRemoteHoverBase[lClient];
}

int MR_NetworkSession::GetLocalHoverId(int pLocalIndex) const
{
	if((pLocalIndex < 0) || (pLocalIndex >= mLocalPartySize)) {
		return -1;
	}

	return mLocalHoverBase + pLocalIndex;
}

const char *MR_NetworkSession::ResolvePlayerName(int pHoverId) const
{
	if((pHoverId >= mLocalHoverBase) &&
		(pHoverId < (mLocalHoverBase + mLocalPartySize)))
	{
		return mLocalPartyNames[pHoverId - mLocalHoverBase];
	}

	for(int lClient = 0; lClient < MR_NetworkInterface::eMaxClient; ++lClient) {
		const int lBase = mRemoteHoverBase[lClient];
		if((lBase >= 0) && (pHoverId >= lBase) &&
			(pHoverId < (lBase + mRemotePartySize[lClient])))
		{
			return mNetInterface.GetRemotePartyName(lClient, pHoverId - lBase);
		}
	}

	return "?";
}

MR_MainCharacter *MR_NetworkSession::GetRemoteCharacterByHoverId(int pHoverId) const
{
	if((pHoverId < 0) || (pHoverId >= eMaxRemoteHover)) {
		return NULL;
	}

	return mRemoteCharacter[pHoverId];
}

MR_MainCharacter *MR_NetworkSession::GetRepresentativeRemoteCharacter(int pClient) const
{
	if((pClient < 0) || (pClient >= MR_NetworkInterface::eMaxClient)) {
		return NULL;
	}

	const int lBase = mRemoteHoverBase[pClient];
	if(lBase < 0) {
		return NULL;
	}

	for(int i = 0; i < mRemotePartySize[pClient]; ++i) {
		MR_MainCharacter *lCharacter = GetRemoteCharacterByHoverId(lBase + i);
		if(lCharacter != NULL) {
			return lCharacter;
		}
	}

	return NULL;
}

void MR_NetworkSession::RemoveRemoteClientCharacters(int pClient)
{
	if((pClient < 0) || (pClient >= MR_NetworkInterface::eMaxClient)) {
		return;
	}

	MR_Level *lLevel = mSession.GetCurrentLevel();
	for(int lHoverId = 0; lHoverId < eMaxRemoteHover; ++lHoverId) {
		if((mRemoteOwnerClient[lHoverId] == pClient) &&
			(mRemoteCharacter[lHoverId] != NULL))
		{
			if((lLevel != NULL) && (mRemoteClient[lHoverId] != NULL)) {
				lLevel->DeleteElement(mRemoteClient[lHoverId]);
			}
			mRemoteClient[lHoverId] = NULL;
			mRemoteCharacter[lHoverId] = NULL;
			mRemoteOwnerClient[lHoverId] = -1;
		}
	}
}

void MR_NetworkSession::ApplyRemoteMainElementState(int pClientId, int pHoverId,
	const MR_UInt8 *pStateData, int pStateLen)
{
	MR_MainCharacter *lRemoteCharacter = GetRemoteCharacterByHoverId(pHoverId);
	if(lRemoteCharacter == NULL) {
		return;
	}

	int lCurrentSimTime = mSession.GetSimulationTime();
	int lLastCollisionAge = lCurrentSimTime - lRemoteCharacter->mLastCollisionTime;

	if((lCurrentSimTime >= 0) &&
		(lLastCollisionAge < (mNetInterface.GetAvgLag(pClientId) + 40)))
	{
		return;
	}

	int lOldRoom = lRemoteCharacter->mRoom;
	lRemoteCharacter->SetNetState(pStateLen, pStateData);

	if(lRemoteCharacter->mRoom != lOldRoom) {
		MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
		if((lCurrentLevel != NULL) && (pHoverId >= 0) && (pHoverId < eMaxRemoteHover)) {
			lCurrentLevel->MoveElement(mRemoteClient[pHoverId], lRemoteCharacter->mRoom);
		}
	}
}

/**
 * Get the results for the player in pPosition.  Writes out all the information
 * to the other passed parameters.
 *
 * @param pPosition Position of player we are looking for
 * @param pPlayerName Variable we will write the player name into
 * @param pId Variable we will write the player's ID into
 * @param pConnected Variable we will write whether or not the player is connected into
 * @param pNbLap Variable we will write the player's lap number into (-1 for finished)
 * @param pFinishTime Variable we will write the player's finish time into
 * @param pBestLap Variable we will write the player's best lap time into
 */
void MR_NetworkSession::GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbLap, MR_SimulationTime &pFinishTime, MR_SimulationTime &pBestLap, int &pNbSplit, MR_SimulationTime & pFinishFirstSplit, MR_SimulationTime &pFirstSplitDifference, MR_SimulationTime &pFinishSecondSplit, MR_SimulationTime &pSecondSplitDifference) const
{
	// The PlayerResult is a linked list, we have to step through it starting at
	// the head to find the position we are looking for.  Why the hell is it a
	// linked list anyway?
	PlayerResult *lCurrent = mResultList;

	for(int lCounter = 0; lCounter <= pPosition; lCounter++) {
		if(lCurrent == NULL) {
			ASSERT(FALSE);
			pPlayerName = "?";
			break;
		}
		else {
			if(lCounter == pPosition) {
				// We got it
				pPlayerName = lCurrent->mPlayerName;
				pId = lCurrent->mPlayerId;
				pConnected = (FindPlayerByHoverId(lCurrent->mPlayerHoverId) != NULL);
				pNbLap = lCurrent->mNbCompletedLap;
				pFinishTime = lCurrent->mFinishTime;
				pBestLap = lCurrent->mBestLap;
				pNbSplit = lCurrent->mNbCompletedSplit;
				pFinishFirstSplit = lCurrent->mFinishFirstSplit;
				pFirstSplitDifference = lCurrent->mFirstSplitDifference;
				pFinishSecondSplit = lCurrent->mFinishSecondSplit;
				pSecondSplitDifference = lCurrent->mSecondSplitDifference;
			}
			else {
				lCurrent = lCurrent->mNext;
			}
		}
	}
}

/**
 * Get war results for the player in pPosition.  Writes out all the information
 * to the other passed parameters.
 *
 * @param pPosition Position of player we are looking for
 * @param pPlayerName Variable we will write the player name into
 * @param pId Variable we will write the player's ID into
 * @param pConnected Variable we will write whether or not the player is connected into
 * @param pNbHitOther Variable we will write the number of times the player hit others into
 * @param pNbHitHimself Variable we will write the number of times the player was hit into
 */
void MR_NetworkSession::GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbHitOther, int &pNbHitHimself) const
{
	PlayerResult *lCurrent = mHitList;

	for(int lCounter = 0; lCounter <= pPosition; lCounter++) {
		if(lCurrent == NULL) {
			ASSERT(FALSE);
			pPlayerName = "?";
			break;
		}
		else {
			if(lCounter == pPosition) {
				// We got it
				pPlayerName = lCurrent->mPlayerName;
				pId = lCurrent->mPlayerId;
				pConnected = (FindPlayerByHoverId(lCurrent->mPlayerHoverId) != NULL);
				pNbHitOther = lCurrent->mNbGoodShot;
				pNbHitHimself = lCurrent->mNbTimeHit;

			}
			else {
				lCurrent = lCurrent->mNextHitResult;
			}
		}
	}
}

/*
BOOL MR_NetworkSession::GetResult( int pPosition, const char*& pPlayer, MR_SimulationTime& pFinishTime, MR_SimulationTime& pBestLap )const
{
   BOOL lReturnValue = TRUE;

   PlayerResult* lCurrent = mResultList;

   for( int lCounter = 0; (lCounter<=pPosition)&&lReturnValue; lCounter++ )
   {
	  if( lCurrent == NULL )
	  {
		 lReturnValue = FALSE;
	  }
	  else
	  {
		 if( lCounter == pPosition )
		 {
			// We got it
			pPlayer     = mNetInterface.GetPlayerName( lCurrent->mPlayerIndex );

			pFinishTime = lCurrent->mFinishTime;
			pBestLap    = lCurrent->mBestLap;
		 }
		 else
		 {
			lCurrent = lCurrent->mNext;
		 }
	  }
   }
   return lReturnValue;
}

BOOL MR_NetworkSession::ResultAvaillable()const
{
   return( mResultList!=NULL );
}
*/

/**
 * Returns the number of players still connected and playing.
 */
int MR_NetworkSession::GetNbPlayers() const
{
	return max(1, mConfiguredPlayerCount);
}

/**
 * Returns an MR_MainCharacter object pointer pointing to the player specified in pPlayerIndex.
 *
 * @param pPlayerIndex Index of the player being located
 */
const MR_MainCharacter *MR_NetworkSession::GetPlayer(int pPlayerIndex) const
{
	if((pPlayerIndex < 0) || (pPlayerIndex >= GetNbPlayers())) {
		return NULL;
	}

	if((pPlayerIndex >= mLocalHoverBase) &&
		(pPlayerIndex < (mLocalHoverBase + mLocalPartySize)))
	{
		return mMainCharacters[pPlayerIndex - mLocalHoverBase];
	}

	return GetRemoteCharacterByHoverId(pPlayerIndex);
}

void MR_NetworkSession::SetPlayerCraftCollision(int pHoverId, BOOL pEnabled)
{
	MR_ClientSession::SetPlayerCraftCollision(pHoverId, pEnabled);
	MR_MainCharacter *lRemote = GetRemoteCharacterByHoverId(pHoverId);
	if(lRemote != NULL) {
		lRemote->SetCraftCollisionEnabled(pEnabled);
	}
}

void MR_NetworkSession::SetPlayerColumnInteraction(int pHoverId, BOOL pEnabled)
{
	MR_ClientSession::SetPlayerColumnInteraction(pHoverId, pEnabled);
	MR_MainCharacter *lRemote = GetRemoteCharacterByHoverId(pHoverId);
	if(lRemote != NULL) {
		lRemote->SetColumnInteractionEnabled(pEnabled);
	}
}

void MR_NetworkSession::SetPlayerRenderOpacity(int pHoverId, float pOpacity)
{
	MR_ClientSession::SetPlayerRenderOpacity(pHoverId, pOpacity);
	MR_MainCharacter *lRemote = GetRemoteCharacterByHoverId(pHoverId);
	if(lRemote != NULL) {
		lRemote->SetRenderOpacity(pOpacity);
	}
}

BOOL MR_NetworkSession::IsLocalHoverId(int pHoverId) const
{
	return MR_ClientSession::IsLocalHoverId(pHoverId);
}

const char *MR_NetworkSession::GetPlayerDisplayName(int pHoverId) const
{
	return ResolvePlayerName(pHoverId);
}

const MR_MainCharacter *MR_NetworkSession::FindPlayerByHoverId(int pHoverId) const
{
	const MR_MainCharacter *lLocal = MR_ClientSession::FindPlayerByHoverId(pHoverId);
	if(lLocal != NULL) {
		return lLocal;
	}

	return GetRemoteCharacterByHoverId(pHoverId);
}

BOOL MR_NetworkSession::ShouldProcessLocalHitQueues() const
{
	return FALSE;
}

/**
 * Returns the rank (position) of the main character.  The parameter is ignored if provided, as this function makes the assumption that the position of the main player is the one being found.
 *
 * @param pPlayer Ignored.
 */
int MR_NetworkSession::GetRank(const MR_MainCharacter *pPlayer) const {
	int lReturnValue = 0;
	const int lHoverId = (pPlayer != NULL) ? pPlayer->GetHoverId() :
		((mMainCharacter1 != NULL) ? mMainCharacter1->GetHoverId() : -1);

	PlayerResult *lCurrent = mResultList;

	while(lCurrent != NULL) {
		lReturnValue++;
		if(lCurrent->mPlayerHoverId == lHoverId) {
			break;
		}
		lCurrent = lCurrent->mNext;
	}

	return lReturnValue;
}

/**
 * The main game loop.  Read, process, write, read.  I wonder why we read twice.
 */
BOOL MR_NetworkSession::Process(int pSpeedFactor)
{
	SteamAPI_RunCallbacks();
	ReadNet();
	BOOL lReturnValue = MR_ClientSession::Process(pSpeedFactor);
	WriteNet();
	ReadNet();

	return lReturnValue;
}

/**
 * Load a new level.  This function calls MR_ClientSession::LoadNew() and then tells the level to notify call ElementCreationHook() and PermElementStateHook() when
 * elements are created.
 */
BOOL MR_NetworkSession::LoadNew(const char *pTitle, MR_RecordFile *pMazeFile,
	int pNbLap, BOOL pAllowWeapons, BOOL pAllowCans, BOOL pAllowMines,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings,
	MR_VideoBuffer *pVideo)
{
	BOOL lReturnValue = MR_ClientSession::LoadNew(pTitle, pMazeFile, pNbLap,
		pAllowWeapons, pAllowCans, pAllowMines, pAllowedCraftMask,
		pGameRuleSettings, pVideo);

	if(lReturnValue) {
		mSession.GetCurrentLevel()->SetBroadcastHook(ElementCreationHook, PermElementStateHook, this);
	}

	return lReturnValue;
}

/**
 * A hook called when a free element is created.  We need to broadcast it once it is created.
 */
void MR_NetworkSession::ElementCreationHook(MR_FreeElement *pElement, int pRoom, void *pThis)
{
	((MR_NetworkSession *) pThis)->BroadcastAutoElementCreation(pElement->GetTypeId(), pElement->GetNetState(), pRoom);
}

/**
 * A hook called when a permanent element is created.  We need to broadcast it once it is created.
 */
void MR_NetworkSession::PermElementStateHook(MR_FreeElement *pElement, int pRoom, int pPermId, void *pThis)
{
	((MR_NetworkSession *) pThis)->BroadcastPermElementState(pPermId, pElement->GetNetState(), pRoom);
}

/**
 * Look for messages sent on the network.
 *
 * Valid messages are:
 *
 * MRNM_SET_TIME -- reset the time of the game to what we are told
 * MRNM_CREATE_MAIN_ELEM -- create a hovercraft
 * MRNM_SEND_KEYID -- for the unused ladder patch; does not do anything
 * MRNM_SET_MAIN_ELEM_STATE -- move a hovercraft to a different room or position
 */
void MR_NetworkSession::ReadNet()
{
	DWORD lTimeStamp;
	int lMessageType;
	int lMessageLen;
	const MR_UInt8 *lMessage;
	int lClientId;

	lTimeStamp = mSession.GetSimulationTime();

	while(mNetInterface.FetchMessage(lTimeStamp, lMessageType, lMessageLen, lMessage, lClientId)) {
		{
			// char lBuffer[100];
			// sprintf(lBuffer, "Got message from client %d, message %d\n", lClientId, lMessageType);
			// OutputDebugString((LPCTSTR) lBuffer);
		}

		switch(lMessageType) {
			case MRNM_SET_TIME: // reset the simulation time to what we are told to
				{
					MR_SimulationTime lNewTime = *(MR_Int32 *) & (lMessage[0]);
					// lNewTime += mNetInterface.GetAvgLag( lClientId ); Done by SetSimulationTime
	
					mSession.SetSimulationTime(lNewTime);
				}
				break;

			case MRNM_CREATE_MAIN_ELEM: // create a hovercraft
				{
					MR_ObjectFromFactoryId lTypeId;
					int lRoom;
					int lHoverId;
	
					lTypeId.mDllId = *(MR_Int16 *) & (lMessage[0]);
					lTypeId.mClassId = *(MR_Int16 *) & (lMessage[2]);
					lRoom = *(MR_Int16 *) & (lMessage[4]);
					lHoverId = *(MR_Int16 *) & (lMessage[6]);

					if((lHoverId >= 0) && (lHoverId < eMaxRemoteHover) &&
						(mRemoteCharacter[lHoverId] == NULL))
					{
						mRemoteCharacter[lHoverId] = (MR_MainCharacter *) MR_DllObjectFactory::CreateObject(lTypeId);
						mRemoteCharacter[lHoverId]->mRoom = lRoom;
						mRemoteCharacter[lHoverId]->SetAsSlave();
						mRemoteCharacter[lHoverId]->SetHoverId(lHoverId);
						mRemoteCharacter[lHoverId]->SetNetState(lMessageLen - 8, lMessage + 8);
						mRemoteCharacter[lHoverId]->SetNbLapForRace(mNbLap);

						MR_Level *lCurrentLevel = mSession.GetCurrentLevel();

						mRemoteClient[lHoverId] = lCurrentLevel->InsertElement(mRemoteCharacter[lHoverId], lRoom);
						mRemoteOwnerClient[lHoverId] = lClientId;
					}
				}
	
				break;

			case MRNM_SEND_KEYID:
				// Awfull patch, save opponent ID for ladder report
				/*
				   mOpponendMajorID = (*(MR_Int32*)&(lMessage[0]));
				   mOpponendMinorID = (*(MR_Int32*)&(lMessage[4]));

				   #ifndef _DEBUG

				   if(  ((*(MR_Int32*)&(lMessage[0])) == mMajorID)&&
				   ((*(MR_Int32*)&(lMessage[4])) == mMinorID) )
					   {
					   AddMessage( MR_LoadString( IDS_DUAL_REG_KEY ) );
					   mNetInterface.Disconnect();
	
					   }
					   #endif
					 */
					break;

			case MRNM_SET_MAIN_ELEM_STATE: // move a hovercraft (another player)
				{
					int lHoverId = -1;
					const MR_UInt8 *lStateData = lMessage;
					int lStateLen = lMessageLen;

					if(lMessageLen >= 2) {
						lHoverId = *(MR_Int16 *) & (lMessage[0]);
						lStateData = lMessage + 2;
						lStateLen = lMessageLen - 2;
					}
					else if((lClientId >= 0) && (lClientId < MR_NetworkInterface::eMaxClient)) {
						lHoverId = mRemoteHoverBase[lClientId];
					}
					ApplyRemoteMainElementState(lClientId, lHoverId, lStateData, lStateLen);
				}
				break;

			case MRNM_SET_MAIN_ELEM_STATE_BATCH:
				{
					if(lMessageLen >= 1) {
						int lOffset = 1;
						int lCount = lMessage[0];
						for(int lIndex = 0; (lIndex < lCount) && (lOffset + 3 <= lMessageLen); ++lIndex) {
							int lHoverId = *(MR_Int16 *) &(lMessage[lOffset]);
							int lStateLen = lMessage[lOffset + 2];
							lOffset += 3;
							if((lStateLen < 0) || ((lOffset + lStateLen) > lMessageLen)) {
								break;
							}

							ApplyRemoteMainElementState(lClientId, lHoverId,
								lMessage + lOffset, lStateLen);
							lOffset += lStateLen;
						}
					}
				}
				break;

			case MRNM_CREATE_AUTO_ELEM:
				{
					MR_ObjectFromFactoryId lTypeId;
					int lRoom;
					MR_FreeElement *lNewElement;
	
					lTypeId.mDllId = *(MR_Int16 *) & (lMessage[0]);
					lTypeId.mClassId = *(MR_Int16 *) & (lMessage[2]);
					lRoom = *(MR_Int16 *) & (lMessage[4]);
	
					lNewElement = (MR_FreeElement *) MR_DllObjectFactory::CreateObject(lTypeId);
	
					if(lNewElement != NULL) {
						lNewElement->SetNetState(lMessageLen - 6, lMessage + 6);
	
						MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	
						MR_FreeElementHandle lElementHandle = lCurrentLevel->InsertElement(lNewElement, lRoom);
	
						// Verify if it is not a good idea to simulate the network delay
						int lNetworkDelay = mNetInterface.GetMinLag(lClientId);
	
						if((mMainCharacter1 != NULL) && (lNetworkDelay > 50)) {
							// if the element is in the visible range do the simulation
							BOOL lVisible = (lRoom == mMainCharacter1->mRoom);
							int lVisibleRoomCount;
							const int *lVisibleRoom = lCurrentLevel->GetVisibleZones(mMainCharacter1->mRoom, lVisibleRoomCount);
	
							for(int lCounter = 0; !lVisible && (lCounter < lVisibleRoomCount); lCounter++) {
								if(lVisibleRoom[lCounter] == lRoom) {
									lVisible = TRUE;
								}
							}
							// ASSERT( !(!lVisible&&(mMainCharacter1->mRoom==lRoom)) );
	
							if(lVisible) {
													  // the 50ms extra is only a small bonus
								mSession.SimulateLateElement(lElementHandle, lNetworkDelay + 50, lRoom);
							}
						}
					}
				}
				break;

			/*
			   case MRNM_FINISH_STATS:
			   AddResultEntry( lClientId, ((MR_SimulationTime*)lMessage)[0], ((MR_SimulationTime*)lMessage)[1] );
			   break;
			 */

			case MRNM_SET_PERM_ELEMENT_STATE:
				{
					MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	
					if(lCurrentLevel != NULL) {
						int lPermId = *(MR_Int8 *) & (lMessage[0]);
						int lRoom = *(MR_Int16 *) & (lMessage[1]);
	
						MR_FreeElementHandle lPermElemHandle = lCurrentLevel->GetPermanentElementHandle(lPermId);
	
						if(lPermElemHandle != NULL) {
							MR_FreeElement *lElem = MR_Level::GetFreeElement(lPermElemHandle);
	
							ASSERT(lElem != NULL);
	
							lElem->SetNetState(lMessageLen - 3, lMessage + 3);
	
							lCurrentLevel->MoveElement(lPermElemHandle, lRoom);
						}
					}
				}
				break;

			case MRNM_PLAYER_STATS:
				{
					MR_PlayerStats *lStats = (MR_PlayerStats *) lMessage;
					TRACE("Received BroadcastMainElementStats %d, %d, %d, %d, %d, %d, %d, %d, %d\n", lStats->mHoverId, lStats->mFinishTime, lStats->mBestLap, lStats->mCompletedLaps, lStats->mCompletedSplits, lStats->mFinishFirstSplit, lStats->mFirstSplitDifference, lStats->mFinishSecondSplit, lStats->mSecondSplitDifference);
					AddResultEntry(lStats->mHoverId, lStats->mFinishTime, lStats->mBestLap, lStats->mCompletedLaps, lStats->mCompletedSplits, lStats->mFinishFirstSplit, lStats->mFirstSplitDifference, lStats->mFinishSecondSplit, lStats->mSecondSplitDifference);
				}
				break;

			case MRNM_CHAT_MESSAGE:
				EnterCriticalSection(&mChatMutex);
				AddChatMessage(lClientId, (const char *) lMessage, lMessageLen);
				LeaveCriticalSection(&mChatMutex);
				break;

			case MRNM_HIT_MESSAGE:
				{
					int lHoverIdSrc = -1;
					int lElementId = -1;
					int lVictimHoverId = -1;

					if(lMessageLen >= (int) sizeof(MR_HitMessage)) {
						const MR_HitMessage *lHitMessage = (const MR_HitMessage *) lMessage;
						lVictimHoverId = lHitMessage->mVictimHoverId;
						lHoverIdSrc = lHitMessage->mHoverIdSrc;
						lElementId = lHitMessage->mElementId;
					}
					else if(lMessageLen >= 1) {
						lHoverIdSrc = (char) lMessage[0];
					}

					AddHitEntry(lVictimHoverId, lHoverIdSrc);
					NotifyRuleHit(lVictimHoverId, lHoverIdSrc, lElementId);
					DestroyElementByNetworkId(lElementId);
				}
				break;

		}
		lTimeStamp = mSession.GetSimulationTime();
	}
}

/**
 * Send all necessary data to clients.  This is called by MR_NetworkSession::Process() as part of the game loop.
 *
 * If we are the server, we must send clock updates 12 and 8 seconds before the game starts.
 *
 * If our hovercraft has been created, we must broadcast that.  Otherwise, we broadcast its state and statistics.
 *
 * Then, we check if any clients are disconnected and remove them if necessary.
 */
void MR_NetworkSession::WriteNet()
{
	static unsigned int sClientToCheck = 0;	// verified ok even if it may seen weird

	// if we are the server we must send clock updates before the game
	if(mMasterMode) {
		if(!mSended8SecClockUpdate) {
			MR_SimulationTime lCurrentTime = mSession.GetSimulationTime();

			if(!mSended12SecClockUpdate) { // send an update at ~-12 seconds
				if(lCurrentTime >= -11500) {
					BroadcastTime();
					mSended12SecClockUpdate = TRUE;
				}
			}
			else {
				if(lCurrentTime >= -8500) { // send an update at ~-8 seconds
					BroadcastTime();
					mSended8SecClockUpdate = TRUE;
				}
			}
		}

	}

	if(mTimeToSendCharacterCreation != 0) {
		if(mSession.GetSimulationTime() >= mTimeToSendCharacterCreation) { // it is time to broadcast the created hovercraft
			for(int i = 0; i < mLocalPartySize; ++i) {
				if(mMainCharacters[i] != NULL) {
					BroadcastMainElementCreation(mMainCharacters[i]->GetTypeId(),
						mMainCharacters[i]->GetNetState(), mMainCharacters[i]->mRoom,
						mMainCharacters[i]->GetHoverId());
					mLastBroadcastCraftModel[i] = mMainCharacters[i]->GetHoverModel();
				}
			}
			mTimeToSendCharacterCreation = 0; // set to 0 so we don't broadcast it again
		}
	}
	else if(mMainCharacter1 != NULL) { // it has already been broadcast
		const BOOL lPregame = (mSession.GetSimulationTime() < 0);
		BOOL lSendReliableBatch = lPregame;

		for(int i = 0; i < mLocalPartySize; ++i) {
			MR_MainCharacter *lPlayer = mMainCharacters[i];
			if(lPlayer == NULL) {
				continue;
			}

			if(!lPregame && (lPlayer->GetHoverModel() != mLastBroadcastCraftModel[i])) {
				lSendReliableBatch = TRUE;
			}

			if(mSendedPlayerStats[i] != -1) {
				if((lPlayer->GetLap() >= mSendedPlayerStats[i]) ||
					(lPlayer->GetCurrentCheckpoint() >= mSendedCheckpointStats[i]))
				{
					TRACE("- Send Lap Data\n");

					if(lPlayer->GetCurrentCheckpoint() >= mSendedCheckpointStats[i]) {
						mSendedCheckpointStats[i] = lPlayer->GetCurrentCheckpoint();
						TRACE("--- CHECKPOINT COMPLETE %d\n", mSendedCheckpointStats[i]);
						mSendedCheckpointStats[i]++;
					}

					if(lPlayer->HasFinish()) {
						BroadcastMainElementStats(lPlayer->GetHoverId(),
							lPlayer->GetTotalTime(), lPlayer->GetBestLapDuration(), -1,
							lPlayer->GetCurrentCheckpoint(),
							lPlayer->GetFirstSplitCompletion(),
							lPlayer->GetFirstSplitDifference(),
							lPlayer->GetSecondSplitCompletion(),
							lPlayer->GetSecondSplitDifference());
						mSendedPlayerStats[i] = -1;

						if(mInternetGame && (i == 0)) {
							AddMessage(MR_LoadString(IDS_F2_TORETURN));
						}

						TRACE("--- FINISHED\n");
					}
					else if((lPlayer->GetLap() == 0) &&
						(mSendedCheckpointStats[i] == 1))
					{
						BroadcastMainElementStats(lPlayer->GetHoverId(), -1, 0, 0,
							0, 0, 0, 0, 0);
						mSendedPlayerStats[i] = 1;
						TRACE("--- FIRST LAP REPORT\n");
					}
					else {
						if(lPlayer->GetLap() >= mSendedPlayerStats[i]) {
							mSendedPlayerStats[i] = lPlayer->GetLap();
							mSendedPlayerStats[i]++;
							mSendedCheckpointStats[i] = 1;
							TRACE("--- LAP COMPLETE\n");
						}

						BroadcastMainElementStats(lPlayer->GetHoverId(),
							lPlayer->GetTotalTime(), lPlayer->GetBestLapDuration(),
							lPlayer->GetLap(), lPlayer->GetCurrentCheckpoint(),
							lPlayer->GetFirstSplitCompletion(),
							lPlayer->GetFirstSplitDifference(),
							lPlayer->GetSecondSplitCompletion(),
							lPlayer->GetSecondSplitDifference());
					}
				}
			}

			while(lPlayer->HitQueueCount() > 0) {
				MR_MainCharacter::HitEntry lHit = lPlayer->GetHitQueue();
				BroadcastHit(lPlayer->GetHoverId(), lHit.mHoverId, lHit.mElementId);
			}
		}

		if(lSendReliableBatch) {
			BroadcastLocalPartyStates(MR_NET_REQUIRED);
		}
		if(!lPregame) {
			BroadcastLocalPartyStates(MR_NET_DATAGRAM);
		}
		for(int i = 0; i < mLocalPartySize; ++i) {
			MR_MainCharacter *lPlayer = mMainCharacters[i];
			if(lPlayer != NULL) {
				mLastBroadcastCraftModel[i] = lPlayer->GetHoverModel();
			}
		}
	}

	// Remove disconnected opponents
	sClientToCheck++;
	if(sClientToCheck >= MR_NetworkInterface::eMaxClient) {
		sClientToCheck = 0;
	}

	if((GetRepresentativeRemoteCharacter(sClientToCheck) != NULL) &&
		!mNetInterface.IsConnected(sClientToCheck))
	{
		CString lMessage;
		// Add a message indicating the the guy disconnected
		lMessage = mNetInterface.GetPlayerName(sClientToCheck);
		lMessage += MR_LoadString(IDS_HAS_LEFT);

		AddMessage(lMessage);

		// Delete the client characters for that machine before checking who is left.
		RemoveRemoteClientCharacters(sClientToCheck);

		if(mNetInterface.GetClientCount() == 0) {
			if(GetNbPlayers() <= 1) {
				if(mInternetGame) {
					AddMessage(MR_LoadString(IDS_ALONE_F2));
				}
				else {
					AddMessage(MR_LoadString(IDS_ALONE));
				}
			}
			else if(mInternetGame) {
				AddMessage(MR_LoadString(IDS_REMOTE_LEFT_F2));
			}
			else {
				AddMessage(MR_LoadString(IDS_REMOTE_LEFT));
			}
		}
	}
}

/**
 * Set the local player's name.
 *
 * @param pPlayerName The local player's name
 */
void MR_NetworkSession::SetPlayerName(const char *pPlayerName)
{
	mNetInterface.SetPlayerName(pPlayerName);
	if((pPlayerName != NULL) && (*pPlayerName != 0)) {
		mLocalPartyNames[0] = pPlayerName;
	}
}

/**
 * Get the local player's name.
 */
const char *MR_NetworkSession::GetPlayerName() const
{
	return mNetInterface.GetPlayerName();
}

void MR_NetworkSession::SetLocalParty(int pPartySize,
	const std::string *pPartyNames)
{
	mLocalPartySize = max(1, min(pPartySize, MR_MAX_LOCAL_PLAYER));
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mLocalPartyNames[i] =
			(pPartyNames != NULL) ? pPartyNames[i].c_str() : "";
	}
	if(mLocalPartyNames[0].IsEmpty()) {
		mLocalPartyNames[0] = mNetInterface.GetPlayerName();
	}
	mNetInterface.SetPlayerName(mLocalPartyNames[0]);
	mNetInterface.SetLocalParty(mLocalPartySize, pPartyNames);
	mConfiguredPlayerCount = mLocalPartySize;
}

int MR_NetworkSession::GetLocalPartySize() const
{
	return mLocalPartySize;
}

const char *MR_NetworkSession::GetLocalPartyName(int pIndex) const
{
	if((pIndex < 0) || (pIndex >= MR_MAX_LOCAL_PLAYER)) {
		return "";
	}
	return mLocalPartyNames[pIndex];
}

void MR_NetworkSession::SetRaceHash(const char *pRaceHash)
{
	mRaceHash = (pRaceHash != NULL) ? pRaceHash : "";
}

const char *MR_NetworkSession::GetRaceHash() const
{
	return mRaceHash;
}

void MR_NetworkSession::SetRoomList(HoverRace::Client::RoomListPtr roomList)
{
	this->roomList = roomList;
}

/**
 * Wait for other clients to connect.  This is only used in server mode.
 *
 * This method is blocking, until the user either cancels the game or starts the game (or some bizarre fatal error is encountered).
 *
 * @param pWindow Parent window
 * @param pTrackName Track name
 * @param pPromptForPort Should we use the default port or choose our own?
 * @param pDefaultPort The default port (MR_DEFAULT_NET_PORT)
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkSession::WaitConnections(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines)
{
	return WaitConnections(pWindow, pGameName, pPromptForPort, pDefaultPort,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		FALSE, MR_GetDefaultAllowedCraftMask(), mGameRuleSettings);
}

BOOL MR_NetworkSession::WaitConnections(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask)
{
	return WaitConnections(pWindow, pGameName, pPromptForPort, pDefaultPort,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		pHasCrafts, pAllowedCraftMask, mGameRuleSettings);
}

BOOL MR_NetworkSession::WaitConnections(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings)
{
	mMasterMode = TRUE;
	mSended12SecClockUpdate = FALSE;
	mSended8SecClockUpdate = FALSE;

	BOOL lSuccess = mNetInterface.MasterConnect(pWindow, pGameName, pPromptForPort,
		pDefaultPort, pModalessDlg, pReturnMessage, pTrackName, pNbLap,
		pHasWeapons, pAllowWeapons, pHasCans, pAllowCans,
		pHasMines, pAllowMines, pHasCrafts, pAllowedCraftMask,
		pGameRuleSettings);
	if(lSuccess) {
		RefreshHoverAssignments();
	}
	return lSuccess;
}

/**
 * This function is called when a user decides to connect to a specified TCP
 * server.  MR_NetInterface::SlavePreConnect() is called.  Later in the
 * connection process ConnectToServer() is called.
 *
 * @param pWindow Handle to the current window
 * @param pTrackName A variable that the track name is written into
 */
BOOL MR_NetworkSession::PreConnectToServer(HWND pWindow, CString &pTrackName)
{
	mMasterMode = FALSE;
	return mNetInterface.SlavePreConnect(pWindow, pTrackName);
}

/**
 * This function is called when a user connects to a game from the IMR, or,
 * after game information has already been established after calling
 * PreConnectToServer().  This function calls MR_NetInterface::SlaveConnect().
 *
 * @param pWindow Handle to the current window
 * @param pServerIP IP of the server
 * @param pPort Server port number
 * @param pGameName String representing the name of the game (track name)
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkSession::ConnectToServer(HWND pWindow, const char *pServerIP,
	unsigned pPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines)
{
	return ConnectToServer(pWindow, pServerIP, pPort, pSteamID, pGameName,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		FALSE, MR_GetDefaultAllowedCraftMask(), mGameRuleSettings);
}

BOOL MR_NetworkSession::ConnectToServer(HWND pWindow, const char *pServerIP,
	unsigned pPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask)
{
	return ConnectToServer(pWindow, pServerIP, pPort, pSteamID, pGameName,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		pHasCrafts, pAllowedCraftMask, mGameRuleSettings);
}

BOOL MR_NetworkSession::ConnectToServer(HWND pWindow, const char *pServerIP,
	unsigned pPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings)
{
	mMasterMode = FALSE;

	BOOL lSuccess = mNetInterface.SlaveConnect(pWindow, pServerIP, pPort, pSteamID,
		pGameName, pModalessDlg, pReturnMessage, pTrackName, pNbLap,
		pHasWeapons, pAllowWeapons, pHasCans, pAllowCans,
		pHasMines, pAllowMines, pHasCrafts, pAllowedCraftMask,
		pGameRuleSettings);
	if(lSuccess) {
		RefreshHoverAssignments();
	}
	return lSuccess;
}

/**
 * Specify the simulation time outright.
 *
 * @param pTime New simulation time
 */
void MR_NetworkSession::SetSimulationTime(MR_SimulationTime pTime)
{
	pTime += mNetInterface.GetLagFromServer();

	if(mTimeToSendCharacterCreation != 0) {
		mTimeToSendCharacterCreation = min(pTime + 2000, mTimeToSendCharacterCreation);
	}
	MR_ClientSession::SetSimulationTime(pTime);
}

/**
 * Create the main character.  Get the default starting position for our client
 * ID and then place the hovercraft in that position.
 */
BOOL MR_NetworkSession::CreateMainCharacter()
{
	// Add a main character on the track
	ASSERT(mMainCharacter1 == NULL);			  // make sure we are not creating it twice
	ASSERT(mSession.GetCurrentLevel() != NULL);
	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	RefreshHoverAssignments();

	if((mLocalHoverBase + mLocalPartySize) > lCurrentLevel->GetPlayerCount()) {
		return FALSE;
	}

	for(int i = 0; i < mLocalPartySize; ++i) {
		MR_MainCharacter *lPlayer = MR_MainCharacter::New(mNbLap, mAllowWeapons,
			mAllowCans, mAllowMines, mAllowedCraftMask);
		MR_GameRuleSpawnContext lSpawnContext;
		lSpawnContext.mHoverId = GetLocalHoverId(i);
		lSpawnContext.mSpawnSlot = mLocalHoverBase + i;
		if(mGameRuleRuntime != NULL) {
			mGameRuleRuntime->OnPreSpawn(*this, lSpawnContext);
		}

		lPlayer->mPosition = lCurrentLevel->GetStartingPos(lSpawnContext.mSpawnSlot);
		lPlayer->SetOrientation(
			lCurrentLevel->GetStartingOrientation(lSpawnContext.mSpawnSlot));
		lPlayer->mRoom = lCurrentLevel->GetStartingRoom(lSpawnContext.mSpawnSlot);
		lPlayer->SetHoverId(lSpawnContext.mHoverId);

		lCurrentLevel->InsertElement(lPlayer, lPlayer->mRoom);
		mMainCharacters[i] = lPlayer;
	}
	SyncLegacyMainCharacterPointers();

	// Make the hovercraft visible for the full countdown so the other clients
	// can see pre-race craft changes as they happen.
	for(int i = 0; i < mLocalPartySize; ++i) {
		MR_ElementNetState lState = mMainCharacters[i]->GetNetState();
		BroadcastMainElementCreation(mMainCharacters[i]->GetTypeId(), lState,
			mMainCharacters[i]->mRoom, mMainCharacters[i]->GetHoverId());
		mLastBroadcastCraftModel[i] = mMainCharacters[i]->GetHoverModel();
	}
	mTimeToSendCharacterCreation = 0;

	return TRUE;
}

/**
 * Tell other clients we are connected to about our hovercraft.  Sends the DLL
 * ID and class ID of the object to be used by other clients as well as the room
 * and ID.  This sends the MRNM_CREATE_MAIN_ELEM message (parsed in ReadNet() by
 * other clients).
 *
 * @param pId ID of the hovercraft object
 * @param pState State information of the hovercraft; position, speed, direction, etc.
 * @param pRoom ID of the room the hovercraft is in
 * @param pHoverId ID of the hovercraft
 */
void MR_NetworkSession::BroadcastMainElementCreation(const MR_ObjectFromFactoryId &pId, const MR_ElementNetState &pState, int pRoom, int pHoverId)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_CREATE_MAIN_ELEM;
	lMessage.mDataLen = pState.mDataLen + 8;
	*(MR_Int16 *) & (lMessage.mData[0]) = pId.mDllId;
	*(MR_Int16 *) & (lMessage.mData[2]) = pId.mClassId;
	*(MR_Int16 *) & (lMessage.mData[4]) = pRoom;
	*(MR_Int16 *) & (lMessage.mData[6]) = pHoverId;
	memcpy(lMessage.mData + 8, pState.mData, pState.mDataLen);

	mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

	if(mMajorID != -1) {
		lMessage.mMessageType = MRNM_SEND_KEYID;
		lMessage.mDataLen = 8;
		*(MR_Int32 *) & (lMessage.mData[0]) = mMajorID;
		*(MR_Int32 *) & (lMessage.mData[4]) = mMinorID;

		mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);
	}
}

/**
 * This function is called when a non-permanent object is created (missile,
 * mine, or can).  It tells all the other clients where and what the object is.
 *
 * @param pId ID of the object
 * @param pState State of the object; position, speed, direction, etc.
 * @param pRoom Room the object is inside of
 */
void MR_NetworkSession::BroadcastAutoElementCreation(const MR_ObjectFromFactoryId &pId, const MR_ElementNetState &pState, int pRoom)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_CREATE_AUTO_ELEM;
	lMessage.mDataLen = pState.mDataLen + 6;
	*(MR_Int16 *) & (lMessage.mData[0]) = pId.mDllId;
	*(MR_Int16 *) & (lMessage.mData[2]) = pId.mClassId;
	*(MR_Int16 *) & (lMessage.mData[4]) = pRoom;
	memcpy(lMessage.mData + 6, pState.mData, pState.mDataLen);

	// Determine clients proximity
	int lPriorityLevel[MR_NetworkInterface::eMaxClient];

	const MR_Level *lLevel = GetCurrentLevel();
	// int             lVisibleRoomCount;
	// const int* lVisibleRoom = lLevel->GetVisibleZones( mMainCharacter1->mRoom, lVisibleRoomCount );
	int lCounter;

	// Init priority level
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		MR_MainCharacter *lRemoteCharacter = GetRepresentativeRemoteCharacter(lCounter);
		if(lRemoteCharacter != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (lRemoteCharacter->mPosition.mX - mMainCharacter1->mPosition.mX) / 8192;
			int lDistanceY = (lRemoteCharacter->mPosition.mY - mMainCharacter1->mPosition.mY) / 8192;

			MR_Int64 lSqrDistance = Int32x32To64(lDistanceX, lDistanceX) + Int32x32To64(lDistanceY, lDistanceY);

			if(lSqrDistance < (100)) {			  // arround 80m (10*8m)
				lPriorityLevel[lCounter] = 10;
			}
		}
		else {
			lPriorityLevel[lCounter] = 0;
		}
	}

	// First broadcast to near clients
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] >= 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, FALSE)) {
				TRACE("SendUDPA:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullA:%d\n", lCounter);
			}
		}
	}

	// Now to far clients
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] < 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, FALSE)) {
				TRACE("SendUDPB:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullB:%d\n", lCounter);
			}
		}
	}

	// Now to near clients (the second broadcast bring security)
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] >= 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, TRUE)) {
				TRACE("SendUDPC:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullC:%d\n", lCounter);
			}
		}
	}
}

/**
 * Broadcast the state of a permanent element.
 *
 * @param pPermId Permanent ID of the element
 * @param pState State of the permanent elemnt
 * @param pRoom Room the element is in
 */
void MR_NetworkSession::BroadcastPermElementState(int pPermId, const MR_ElementNetState &pState, int pRoom)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_SET_PERM_ELEMENT_STATE;
	lMessage.mDataLen = pState.mDataLen + 3;
	*(MR_Int8 *) & (lMessage.mData[0]) = pPermId;
	*(MR_Int16 *) & (lMessage.mData[1]) = pRoom;
	memcpy(lMessage.mData + 3, pState.mData, pState.mDataLen);

	// Determine clients proximity
	int lPriorityLevel[MR_NetworkInterface::eMaxClient];

	const MR_Level *lLevel = GetCurrentLevel();
	// int             lVisibleRoomCount;
	// const int* lVisibleRoom = lLevel->GetVisibleZones( mMainCharacter1->mRoom, lVisibleRoomCount );
	int lCounter;

	// Init priority level
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		MR_MainCharacter *lRemoteCharacter = GetRepresentativeRemoteCharacter(lCounter);
		if(lRemoteCharacter != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (lRemoteCharacter->mPosition.mX - mMainCharacter1->mPosition.mX) / 8192;
			int lDistanceY = (lRemoteCharacter->mPosition.mY - mMainCharacter1->mPosition.mY) / 8192;

			MR_Int64 lSqrDistance = Int32x32To64(lDistanceX, lDistanceX) + Int32x32To64(lDistanceY, lDistanceY);

			if(lSqrDistance < (100)) {			  // arround 80m (10*8m)
				lPriorityLevel[lCounter] = 10;
			}
		}
		else {
			lPriorityLevel[lCounter] = 0;
		}
	}

	// First broadcast to near clients
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] >= 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, FALSE)) {
				TRACE("SendUDPA:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullA:%d\n", lCounter);
			}
		}
	}

	// Now to far clients
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] < 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, FALSE)) {
				TRACE("SendUDPB:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullB:%d\n", lCounter);
			}
		}
	}

	// Now to near clients (the second broadcast bring security)
	for(lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(lPriorityLevel[lCounter] >= 5) {
			if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, TRUE)) {
				TRACE("SendUDPC:%d\n", lCounter);
			}
			else {
				TRACE("Buffer FullC:%d\n", lCounter);
			}
		}
	}
}

/**
 * Broadcast the current game time to other clients.  Sends an MRNM_SET_TIME
 * message to all other clients.
 */
void MR_NetworkSession::BroadcastTime()
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_SET_TIME;
	lMessage.mDataLen = 4;
	*(MR_Int32 *) & (lMessage.mData[0]) = mSession.GetSimulationTime();

	// Broadcat to everyone
	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(mNetInterface.UDPSend(lCounter, &lMessage, TRUE, FALSE)) {
			TRACE("SendUDPTime:%d\n", lCounter);
		}
		else {
			TRACE("Buffer FullTime:%d\n", lCounter);
		}
	}

}

/**
 * Broadcast the state of our hovercraft to all other clients.  Uses the
 * MRNM_SET_MAIN_ELEM_STATE message.
 *
 * @param pState Current state of the hovercraft
 */
void MR_NetworkSession::BroadcastMainElementState(const MR_ElementNetState &pState,
	int pHoverId, int pReqLevel)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_SET_MAIN_ELEM_STATE;
	lMessage.mDataLen = pState.mDataLen + 2;
	*(MR_Int16 *) & (lMessage.mData[0]) = pHoverId;
	memcpy(lMessage.mData + 2, pState.mData, pState.mDataLen);

	if(pReqLevel != MR_NET_DATAGRAM) {
		mNetInterface.BroadcastMessage(&lMessage, pReqLevel);
		return;
	}

	// Old method
	// mNetInterface.BroadcastMessage( &lMessage, MR_NET_DATAGRAM/*MR_NOT_REQUIRED*/ );

	// New UDP with priority method
	// Algo:
	//    PriorityLevel = ms sincelast send + Visibility bonus + Proximity bonus
	//    Visibility bonus = 50 if client is in a visible zone
	//    Proximity bonus  = 100 if the client is closer than 10m
	//                        50 if the client is closer than 60m
	//    MaxSend          = time since last broadcast*35/1000
	//    Eligible to be sended:
	//       All Connected client that have a Priority level above 220
	//
	//    Send until
	//       all eligible have been send
	//         or
	//       Output queue full
	//         or
	//       MaxSend is reach

	int lCurrentTime = timeGetTime();
	int lLocalSendIndex = pHoverId - mLocalHoverBase;
	if((lLocalSendIndex < 0) || (lLocalSendIndex >= MR_MAX_LOCAL_PLAYER)) {
		lLocalSendIndex = 0;
	}

	// DEPRECATED because nobody uses 14K anymore
												  // 14K calibration
	int lMaxSend =
		(lCurrentTime - mLastSendElemStateFuncTime[lLocalSendIndex]) * 27 / 1000;

	// Instead let's try refreshing more often
	//int lMaxSend = (lCurrentTime - mLastSendElemStateFuncTime) * (27 / 125);
	// That's eight times more often than Richard's original 14K calibration

	// TRACE( "lMaxSend:%d\n", lMaxSend );

	if(mSession.GetSimulationTime() <= 0) {
		lMaxSend /= 4;							  // relax on refresh before game start
	}
	else {
		const MR_MainCharacter *lPriorityPlayer = FindPlayerByHoverId(pHoverId);
		if((lPriorityPlayer != NULL) && lPriorityPlayer->mNetPriority) {
			lMaxSend += 2;						  // increse that base number.. there is a priority
		}
	}

	if(lMaxSend > 0) {
		mLastSendElemStateFuncTime[lLocalSendIndex] = lCurrentTime;

		int lPriorityLevel[MR_NetworkInterface::eMaxClient];

		const MR_Level *lLevel = GetCurrentLevel();
		int lVisibleRoomCount;
		const MR_MainCharacter *lSourcePlayer = FindPlayerByHoverId(pHoverId);
		if(lSourcePlayer == NULL) {
			return;
		}
		const int *lVisibleRoom = lLevel->GetVisibleZones(lSourcePlayer->mRoom, lVisibleRoomCount);

		int lNbEligible = 0;

		// Init priority level
		for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
			MR_MainCharacter *lRemoteCharacter = GetRepresentativeRemoteCharacter(lCounter);
			if(lRemoteCharacter != NULL) {
				lPriorityLevel[lCounter] = lCurrentTime - mLastSendElemStateTime[lCounter];

				// Do the visibilitytest
				if(lRemoteCharacter->mRoom == lSourcePlayer->mRoom) {
					if(lSourcePlayer->mNetPriority) {
												  // to keep priority state even when this broadcast will be finish
						mLastSendElemStateTime[lCounter] -= 100;
												  // major priority boost
						lPriorityLevel[lCounter] += 200;
					}
					else {
						lPriorityLevel[lCounter] += 60;
					}
				}
				else {
					for(int lRoom = 0; lRoom < lVisibleRoomCount; lRoom++) {
						if(lVisibleRoom[lRoom] == lRemoteCharacter->mRoom) {
							if(lSourcePlayer->mNetPriority) {
												  // to keep priority state even when this broadcast will be finish
								mLastSendElemStateTime[lCounter] -= 100;
												  // major priority boost
								lPriorityLevel[lCounter] += 200;
							}
							else {
								lPriorityLevel[lCounter] += 60;
							}
							break;
						}
					}
				}

				int lDistanceX = (lRemoteCharacter->mPosition.mX - lSourcePlayer->mPosition.mX) / 1024;
				int lDistanceY = (lRemoteCharacter->mPosition.mY - lSourcePlayer->mPosition.mY) / 1024;

				MR_Int64 lSqrDistance = Int32x32To64(lDistanceX, lDistanceX) + Int32x32To64(lDistanceY, lDistanceY);

				if(lSqrDistance < (100 * 100)) {  // within a 100 meter radius
					if(lSqrDistance < (10 * 10)) {
												  // major priority boost
						lPriorityLevel[lCounter] += 200;
					}
					else {
						lPriorityLevel[lCounter] += 100;
					}
				}
				if(lPriorityLevel[lCounter] > 280) {
					lNbEligible++;
				}
			}
			else {
				lPriorityLevel[lCounter] = 0;
			}
		}

		//TRACE("lNbEligible:%d %d\n", lNbEligible, lMaxSend);

		for(int lNbToSend = min(lNbEligible, lMaxSend); lNbToSend > 0; lNbToSend--) {
			// Find the highest priority element
			int lBestPriority = lPriorityLevel[0];
			int lSelectedClient = 0;

			for(int lClient = 1; lClient < MR_NetworkInterface::eMaxClient; lClient++) {
				if(lPriorityLevel[lClient] > lBestPriority) {
					lBestPriority = lPriorityLevel[lClient];
					lSelectedClient = lClient;
				}
			}

			if(mNetInterface.UDPSend(lSelectedClient, &lMessage, FALSE)) {
				// TRACE("SendUDP:%d %d\n", lSelectedClient, lBestPriority);

				lPriorityLevel[lSelectedClient] = 0;
				mLastSendElemStateTime[lSelectedClient] = lCurrentTime;
			}
			else {
				TRACE("Buffer Full:%d %d\n", lSelectedClient, lBestPriority);

				// Sending buffer full.. continue next time
				break;
			}
		}

		// Disable burst alarm
		MR_MainCharacter *lPlayer = const_cast<MR_MainCharacter *>(lSourcePlayer);
		if(lPlayer != NULL) {
			lPlayer->mNetPriority = FALSE;
		}
	}
}

void MR_NetworkSession::BroadcastLocalPartyStates(int pReqLevel)
{
	MR_NetMessageBuffer lMessage;
	lMessage.mMessageType = MRNM_SET_MAIN_ELEM_STATE_BATCH;
	lMessage.mDataLen = 1;
	lMessage.mData[0] = 0;

	for(int i = 0; i < mLocalPartySize; ++i) {
		MR_MainCharacter *lPlayer = mMainCharacters[i];
		if(lPlayer == NULL) {
			continue;
		}

		MR_ElementNetState lState = lPlayer->GetNetState();
		const int lEntryLen = 3 + lState.mDataLen;
		if(lEntryLen > MR_MAX_NET_MESSAGE_LEN) {
			ASSERT(FALSE);
			continue;
		}

		if((lMessage.mDataLen + lEntryLen) > MR_MAX_NET_MESSAGE_LEN) {
			if(lMessage.mData[0] > 0) {
				mNetInterface.BroadcastMessage(&lMessage, pReqLevel);
			}
			lMessage.mMessageType = MRNM_SET_MAIN_ELEM_STATE_BATCH;
			lMessage.mDataLen = 1;
			lMessage.mData[0] = 0;
		}

		*(MR_Int16 *) &(lMessage.mData[lMessage.mDataLen]) = lPlayer->GetHoverId();
		lMessage.mData[lMessage.mDataLen + 2] = (MR_UInt8) lState.mDataLen;
		memcpy(lMessage.mData + lMessage.mDataLen + 3, lState.mData, lState.mDataLen);
		lMessage.mDataLen += lEntryLen;
		lMessage.mData[0]++;
	}

	if(lMessage.mData[0] > 0) {
		mNetInterface.BroadcastMessage(&lMessage, pReqLevel);
	}
}

/**
 * Broadcast player statistics; finish time, best lap, and number of laps.
 * This is called after each lap.
 *
 * @param pFinishTime Final race time
 * @param pBestLap Time of best lap
 * @param pNbLaps Lap we are currently on; -1 denotes that we have finished
 */
void MR_NetworkSession::BroadcastMainElementStats(int pHoverId, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLaps, int pNbSplits, MR_SimulationTime pFinishFirstSplit, MR_SimulationTime pFirstSplitDifference, MR_SimulationTime pFinishSecondSplit, MR_SimulationTime pSecondSplitDifference)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_PLAYER_STATS;
	lMessage.mDataLen = sizeof(MR_PlayerStats);

	MR_PlayerStats *lStats = (MR_PlayerStats *) lMessage.mData;

	lStats->mHoverId = pHoverId;
	lStats->mFinishTime = pFinishTime;
	lStats->mBestLap = pBestLap;
	lStats->mCompletedLaps = pNbLaps;
	lStats->mCompletedSplits = pNbSplits;
	lStats->mFinishFirstSplit = pFinishFirstSplit;
	lStats->mFirstSplitDifference = pFirstSplitDifference;
	lStats->mFinishSecondSplit = pFinishSecondSplit;
	lStats->mSecondSplitDifference = pSecondSplitDifference;

	mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

	TRACE("Sending BroadcastMainElementStats %d, %d, %d, %d, %d, %d, %d, %d, %d\n", pHoverId, pFinishTime, pBestLap, pNbLaps, pNbSplits, pFinishFirstSplit, pFirstSplitDifference, pFinishSecondSplit, pSecondSplitDifference);

	// Add local time
	AddResultEntry(pHoverId, pFinishTime, pBestLap, pNbLaps, pNbSplits, pFinishFirstSplit, pFirstSplitDifference, pFinishSecondSplit, pSecondSplitDifference);
}

/**
 * Broadcast a chat message to all clients.
 *
 * @param pMessage Chat message
 */
void MR_NetworkSession::BroadcastChatMessage(const char *pMessage)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime    = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_CHAT_MESSAGE;
	lMessage.mDataLen = strlen(pMessage);

	if(lMessage.mDataLen > 0) {
		memcpy(lMessage.mData, pMessage, lMessage.mDataLen);

		mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

		// Add locally
		AddChatMessage(-1, pMessage, lMessage.mDataLen);
	}
}

/**
 * Broadcast that we have been hit by a missile or mine.
 *
 * @param pHoverIdSrc Who we have been hit by
 */
void MR_NetworkSession::BroadcastHit(int pVictimHoverId, int pHoverIdSrc, int pElementId)
{
	MR_NetMessageBuffer lMessage;
	MR_HitMessage lHitMessage;

	// lMessage.mSendingTime    = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_HIT_MESSAGE;
	lMessage.mDataLen = sizeof(lHitMessage);
	lHitMessage.mVictimHoverId = (MR_Int16) pVictimHoverId;
	lHitMessage.mHoverIdSrc = (MR_Int16) pHoverIdSrc;
	lHitMessage.mElementId = pElementId;
	memcpy(lMessage.mData, &lHitMessage, sizeof(lHitMessage));

	mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

	// Add locally
	AddHitEntry(pVictimHoverId, pHoverIdSrc);
	NotifyRuleHit(pVictimHoverId, pHoverIdSrc, pElementId);
	DestroyElementByNetworkId(pElementId);
}

void MR_NetworkSession::DestroyElementByNetworkId(int pElementId)
{
	if(pElementId < 0) {
		return;
	}

	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	if(lCurrentLevel == NULL) {
		return;
	}

	for(int lRoom = MR_Level::eNonClassified; lRoom < lCurrentLevel->GetRoomCount(); lRoom++) {
		MR_FreeElementHandle lCurrent = lCurrentLevel->GetFirstFreeElement(lRoom);
		while(lCurrent != NULL) {
			MR_FreeElementHandle lNext = MR_Level::GetNextFreeElement(lCurrent);
			MR_FreeElement *lElement = MR_Level::GetFreeElement(lCurrent);

			if(lElement->GetNetworkId() == pElementId) {
				lCurrentLevel->DeleteElement(lCurrent);
				return;
			}

			lCurrent = lNext;
		}
	}
}

void MR_NetworkSession::AddHitEntry(int pPlayerHoverId, int pPlayerFromId)
{
	// We assume that a result entry exist for both players
	PlayerResult *lEntry;
	PlayerResult **lPtr;

	// Add a point to FromID
	if(pPlayerFromId >= 0) {
		lPtr = &mHitList;

		while(*lPtr != NULL) {
			if((*lPtr)->mPlayerId == pPlayerFromId) {
				if((*lPtr)->mPlayerHoverId != pPlayerHoverId) {
					lEntry = *lPtr;
					*lPtr = lEntry->mNextHitResult;
					lEntry->mNextHitResult = NULL;
					lEntry->mNbGoodShot++;

					InsertHitEntry(lEntry);
				}
				break;
			}
			lPtr = &((*lPtr)->mNextHitResult);
		}
	}
	lPtr = &mHitList;

	while(*lPtr != NULL) {
		if((*lPtr)->mPlayerHoverId == pPlayerHoverId) {
			lEntry = *lPtr;
			*lPtr = lEntry->mNextHitResult;
			lEntry->mNextHitResult = NULL;
			lEntry->mNbTimeHit++;

			InsertHitEntry(lEntry);
			break;
		}
		lPtr = &((*lPtr)->mNextHitResult);
	}
}

/**
 * Insert a PlayerResult object into the list, sorting correctly by hit statistics.
 *
 * @param pEntry PlayerResult object pointer to be inserted into the list
 */
void MR_NetworkSession::InsertHitEntry(PlayerResult *pEntry)
{
	PlayerResult **lPtr;

	lPtr = &mHitList;

	while(*lPtr != NULL) {
		if((pEntry->mNbGoodShot - pEntry->mNbTimeHit) > ((*lPtr)->mNbGoodShot - (*lPtr)->mNbTimeHit)) {
			break;
		}
		lPtr = &((*lPtr)->mNextHitResult);
	}

	pEntry->mNextHitResult = *lPtr;
	*lPtr = pEntry;
}

/**
 * Add or update player result entries for the given player and data.
 *
 * @param pPlayerIndex Index of the player
 * @param pFinishTime Finish time of the player
 * @param pBestLap Best lap time of the player
 * @param pNbLap Lap number that the player is on (-1 means they have finished the race)
 */
void MR_NetworkSession::AddResultEntry(int pPlayerHoverId, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap, int pNbSplits, MR_SimulationTime pFinishFirstSplit, MR_SimulationTime pFirstSplitDifference, MR_SimulationTime pFinishSecondSplit, MR_SimulationTime pSecondSplitDifference)
{
	// If nbLap == -1 that mean that the race is completed
	PlayerResult *lEntry;
	PlayerResult **lPtr;

	// First look if the entry do not already exist; try to make lPtr point to
	// an existing older record of this player
	lPtr = &mResultList;

	while((*lPtr != NULL) && ((*lPtr)->mPlayerHoverId != pPlayerHoverId)) {
		lPtr = &((*lPtr)->mNext);
	}

	if(*lPtr != NULL) {
		lEntry = *lPtr;

		// Remove the entry from the list (we will re-add it later)
		*lPtr = (*lPtr)->mNext;
		const MR_MainCharacter *lPlayer = FindPlayerByHoverId(pPlayerHoverId);
		if(lPlayer != NULL) {
			lEntry->mCraftModel = lPlayer->GetHoverModel();
		}

	}
	else {
		lEntry = new PlayerResult;

		// Fill the fields that are only filled once
		lEntry->mPlayerHoverId = pPlayerHoverId;
		lEntry->mPlayerName = ResolvePlayerName(pPlayerHoverId);

		lEntry->mNbTimeHit = 0;
		lEntry->mNbGoodShot = 0;
		lEntry->mNextHitResult = NULL;

		// add this entry at the end of the hit list
		InsertHitEntry(lEntry);
		lEntry->mPlayerId = pPlayerHoverId;
		const MR_MainCharacter *lPlayer = FindPlayerByHoverId(pPlayerHoverId);
		if(lPlayer != NULL) {
			lEntry->mCraftModel = lPlayer->GetHoverModel();
		}
	}

	lEntry->mPlayerName = ResolvePlayerName(pPlayerHoverId);

	// Update the entry
	lEntry->mNbCompletedLap = pNbLap;
	lEntry->mFinishTime = pFinishTime;			  // finish time of the last lap
	lEntry->mBestLap = pBestLap;
	lEntry->mNbCompletedSplit = pNbSplits;
	lEntry->mFinishFirstSplit = pFinishFirstSplit;
	lEntry->mFirstSplitDifference = pFirstSplitDifference;
	lEntry->mFinishSecondSplit = pFinishSecondSplit;
	lEntry->mSecondSplitDifference = pSecondSplitDifference;

	// Insert the entry in the list
	lPtr = &mResultList;

	while(*lPtr != NULL) {
		if(((unsigned) pNbLap) == ((unsigned) (*lPtr)->mNbCompletedLap)) {
			if(((unsigned) pNbSplits) == ((unsigned) (*lPtr)->mNbCompletedSplit)) {
				if(pNbSplits == 0 && pFinishTime < (*lPtr)->mFinishTime) {
					break;
				} else if(pNbSplits == 1 && pFinishFirstSplit < (*lPtr)->mFinishFirstSplit) {
					break;
				} else if(pNbSplits == 2 && pFinishSecondSplit < (*lPtr)->mFinishSecondSplit) {
					break;
				}
			} else if(((unsigned) pNbSplits) > ((unsigned) (*lPtr)->mNbCompletedSplit)) {
				break;
			}
		}
		else if(((unsigned) pNbLap) > ((unsigned) (*lPtr)->mNbCompletedLap)) {
			break;
		}

		lPtr = &((*lPtr)->mNext);
	}

	lEntry->mNext = *lPtr;

	*lPtr = lEntry;

	NotifyRuleCheckpoint(lEntry->mPlayerId, pNbSplits);
	if(pNbLap > 0) {
		NotifyRuleLapComplete(lEntry->mPlayerId, pNbLap);
	}
}

/**
 * Add a typed character into our message composition display.  If it is '\n' or
 * '\r' the user hit the enter key and we send it, after running it through the
 * Ascii2Simple() function.
 *
 * @param pKey The key the user pressed
 */
void MR_NetworkSession::AddMessageKey(char pKey)
{
	int st = time( NULL );
	TRACE("AddMessageKey pKey %c %dms ", pKey, time( NULL ) - st);
	EnterCriticalSection(&mChatMutex);
	TRACE("EnterCriticalSection %dms ", time( NULL ) - st);

	int lStrLen = strlen(mChatEditBuffer);
	TRACE("strlen(mChatEditBuffer) %dms ", time( NULL ) - st);

	if((pKey == '\n') || (pKey == '\r')) {
		BroadcastChatMessage(mChatEditBuffer);

		mChatEditBuffer[0] = 0;
	}
	else if(pKey == 8) {
		if(lStrLen > 0) {
			mChatEditBuffer[lStrLen - 1] = 0;
		}
	}
	else if((lStrLen + 1) < sizeof(mChatEditBuffer)) {
		pKey = Ascii2Simple(pKey);

		if(pKey != 0) {
			mChatEditBuffer[lStrLen] = pKey;
			mChatEditBuffer[lStrLen + 1] = 0;

		}
	}

	TRACE("After processing %dms ", time( NULL ) - st);

	LeaveCriticalSection(&mChatMutex);
	TRACE("LeaveCriticalSection %dms\n", time( NULL ) - st);
}

/**
 * Copies the currently entered string from the chat buffer into pDest.
 *
 * @param pDest Destination string for the text entered into the chat buffer
 */
void MR_NetworkSession::GetCurrentMessage(char *pDest) const
{
	EnterCriticalSection(&((MR_NetworkSession *) this)->mChatMutex);

	strcpy(pDest, mChatEditBuffer);

	LeaveCriticalSection(&((MR_NetworkSession *) this)->mChatMutex);
} 

/**
 * Add a message to our chat display.
 *
 * @param pPlayerIndex Index of the player this is sent from
 * @param pMessage The message to be added
 * @param pMessageLen Length of the message
 */
void MR_NetworkSession::AddChatMessage(int pPlayerIndex, const char *pMessage, int pMessageLen)
{
	for(int lCounter = MR_CHAT_MESSAGE_STACK - 1; lCounter > 0; lCounter--) {
		mMessageStack[lCounter] = mMessageStack[lCounter - 1];
	}

	mMessageStack[0].mCreationTime = time(NULL);

	mMessageStack[0].mBuffer = Ascii2Simple(mNetInterface.GetPlayerName(pPlayerIndex));
	mMessageStack[0].mBuffer += Ascii2Simple('>');
	mMessageStack[0].mBuffer += CString(pMessage, pMessageLen);
}
