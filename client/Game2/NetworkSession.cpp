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

#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Util/StrRes.h"

#include "InternetRoom.h"
#include "resource.h"

#include "NetworkSession.h"

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

using namespace HoverRace::Client;

// Local structures
class MR_PlayerStats
{
	public:

		MR_SimulationTime mFinishTime;
		MR_SimulationTime mBestLap;
		MR_Int16 mCompletedLaps;

};

/**
 * Initializes the MR_NetworkSession.  Sets default values; main character
 * creation will be sent at least 5 seconds before the game starts.
 */
MR_NetworkSession::MR_NetworkSession(BOOL pInternetGame, int pMajorID,
                                     int pMinorID, HWND pWindow) :
	SUPER()
{
	mMasterMode = FALSE;
	mInternetGame = pInternetGame;
	mMajorID = pMajorID;
	mMinorID = pMinorID;
	mWindow = pWindow;

	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		mClientCharacter[lCounter] = NULL;
		mLastSendElemStateTime[lCounter] = timeGetTime();
	}
	mLastSendElemStateFuncTime = timeGetTime();

	mResultList = NULL;
	mHitList = NULL;
	mSendedPlayerStats = 0;

	mChatEditBuffer[0] = 0;

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
	if(mInternetGame && (mainCharacter[0] != NULL)) {
		int lCurrentModel = mainCharacter[0]->GetHoverModel();

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
				if(lCurrent->mPlayerIndex == -1) {
					lPlayer = lCurrent;
				}
				lCurrent = lCurrent->mNext;
				lNbPlayer++;
			}

			if(lPlayer != NULL) {

				int lTotalLap = mainCharacter[0]->GetTotalLap();


				MR_SendRaceResult(mWindow, mSession.GetTitle(),
					lPlayer->mBestLap, mMajorID, mMinorID,
					mNetInterface.GetPlayerName(-1),
					mSession.GetCurrentMazeFile()->GetCheckSum(),
					mainCharacter[0]->GetHoverModel(),
					(lPlayer->mNbCompletedLap == -1) ? lPlayer->mFinishTime : 0,
					(lPlayer->mNbCompletedLap == -1) ? lTotalLap : 0,
					lNbPlayer, roomList);

				/*
				// Report ladder matchs
				if(lNbPlayer == 2) {
					int lWinnerIndex = mResultList->mPlayerIndex;
					// If you lost and the other player have finished
					if((lWinnerIndex != -1) && (mResultList->mNbCompletedLap == -1)) {
						// Report the Lost
						MR_SendLadderResult(mWindow, mNetInterface.GetPlayerName(lWinnerIndex), mOpponendMajorID, mOpponendMinorID, mNetInterface.GetPlayerName(-1), mMajorID, mMinorID, mSession.GetTitle(), lTotalLap);

					}
				}
				*/
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
void MR_NetworkSession::GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbLap, MR_SimulationTime &pFinishTime, MR_SimulationTime &pBestLap) const
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
				pPlayerName = mNetInterface.GetPlayerName(lCurrent->mPlayerIndex);
				pId = lCurrent->mPlayerId;
				pConnected = mNetInterface.IsConnected(lCurrent->mPlayerIndex);
				pNbLap = lCurrent->mNbCompletedLap;
				pFinishTime = lCurrent->mFinishTime;
				pBestLap = lCurrent->mBestLap;
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
				pPlayerName = mNetInterface.GetPlayerName(lCurrent->mPlayerIndex);
				pId = lCurrent->mPlayerId;
				pConnected = mNetInterface.IsConnected(lCurrent->mPlayerIndex);
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
	// Return the number of players still playing???
	// return mNetInterface.GetClientCount()+1;
	return ResultAvaillable(); // what an ugly hack, Richard
}

/**
 * Returns an MR_MainCharacter object pointer pointing to the player specified in pPlayerIndex.
 *
 * @param pPlayerIndex Index of the player being located
 */
const MR_MainCharacter *MR_NetworkSession::GetPlayer(int pPlayerIndex) const
{
	const MR_MainCharacter *lReturnValue = NULL;

	if(pPlayerIndex == 0) 
		lReturnValue = mainCharacter[0];
	else if(pPlayerIndex <= MR_NetworkInterface::eMaxClient)
		lReturnValue = mClientCharacter[pPlayerIndex - 1];
	
	return lReturnValue;
}

/**
 * Returns the rank (position) of the main character.  The parameter is ignored if provided, as this function makes the assumption that the position of the main player is the one being found.
 *
 * @param pPlayer Ignored.
 */
int MR_NetworkSession::GetRank(const MR_MainCharacter * /*pPlayer */ ) const {
// We assume that we are looking for the main character
	int lReturnValue = 0;

	PlayerResult *lCurrent = mResultList;

	while(lCurrent != NULL) {
		lReturnValue++;
		if(lCurrent->mPlayerIndex < 0) {
			break;
		}
		lCurrent = lCurrent->mNext;
	}

	return lReturnValue;
}

/**
 * The main game loop.  Read, process, write, read.  I wonder why we read twice.
 */
void MR_NetworkSession::Process(int pSpeedFactor)
{
	ReadNet();
	SUPER::Process(pSpeedFactor);
	WriteNet();
	ReadNet();
}

/**
 * Load a new level.  This function calls MR_ClientSession::LoadNew() and then tells the level to notify call ElementCreationHook() and PermElementStateHook() when
 * elements are created.
 */
BOOL MR_NetworkSession::LoadNew(const char *pTitle, MR_RecordFile *pMazeFile, int pNbLap, char pGameOpts, MR_VideoBuffer *pVideo)
{
	BOOL lReturnValue = SUPER::LoadNew(pTitle, pMazeFile, pNbLap, pGameOpts, pVideo);

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
			char lBuffer[100];
			sprintf(lBuffer, "Got message from client %d, message %d\n", lClientId, lMessageType);
			OutputDebugString((LPCTSTR) lBuffer);
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
	
					ASSERT(mClientCharacter[lClientId] == NULL);
	
					lTypeId.mDllId = *(MR_Int16 *) & (lMessage[0]);
					lTypeId.mClassId = *(MR_Int16 *) & (lMessage[2]);
					lRoom = *(MR_Int16 *) & (lMessage[4]);
					lHoverId = *(MR_Int16 *) & (lMessage[6]);
	
					mClientCharacter[lClientId] = (MR_MainCharacter *) MR_DllObjectFactory::CreateObject(lTypeId);
					mClientCharacter[lClientId]->mRoom = lRoom;
					mClientCharacter[lClientId]->SetAsSlave();
					mClientCharacter[lClientId]->SetHoverId(lHoverId);
					mClientCharacter[lClientId]->SetNetState(lMessageLen - 8, lMessage + 8);
					mClientCharacter[lClientId]->SetNbLapForRace(mNbLap);
	
					MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	
					mClient[lClientId] = lCurrentLevel->InsertElement(mClientCharacter[lClientId], lRoom);
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
				if(mClientCharacter[lClientId] != NULL) {
					// Drop the message if there was a recent collision on that item
					int lLastCollisionAge = mSession.GetSimulationTime() - mClientCharacter[lClientId]->mLastCollisionTime;

					// do not drop packets before game starts (craft changing)
					if(lLastCollisionAge < (mNetInterface.GetAvgLag(lClientId) + 40) && mSession.GetSimulationTime() >= 0) {
						// Drop this message
					}
					else {
						int lOldRoom = mClientCharacter[lClientId]->mRoom;

						mClientCharacter[lClientId]->SetNetState(lMessageLen, lMessage);

						// Move element if needed
						if(mClientCharacter[lClientId]->mRoom != lOldRoom) {
							MR_Level *lCurrentLevel = mSession.GetCurrentLevel();

							lCurrentLevel->MoveElement(mClient[lClientId], mClientCharacter[lClientId]->mRoom);
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
	
						if((mainCharacter[0] != NULL) && (lNetworkDelay > 50)) {
							// if the element is in the visible range do the simulation
							BOOL lVisible = (lRoom == mainCharacter[0]->mRoom);
							int lVisibleRoomCount;
							const int *lVisibleRoom = lCurrentLevel->GetVisibleZones(mainCharacter[0]->mRoom, lVisibleRoomCount);
	
							for(int lCounter = 0; !lVisible && (lCounter < lVisibleRoomCount); lCounter++) {
								if(lVisibleRoom[lCounter] == lRoom) {
									lVisible = TRUE;
								}
							}
							// ASSERT( !(!lVisible&&(mainCharacter[0]->mRoom==lRoom)) );
	
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
					AddResultEntry(lClientId, lStats->mFinishTime, lStats->mBestLap, lStats->mCompletedLaps);
				}
				break;

			case MRNM_CHAT_MESSAGE:
				EnterCriticalSection(&mChatMutex);
				AddChatMessage(lClientId, (const char *) lMessage, lMessageLen);
				LeaveCriticalSection(&mChatMutex);
				break;

			case MRNM_HIT_MESSAGE:
				AddHitEntry(lClientId, (char) lMessage[0]);
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
			BroadcastMainElementCreation(mainCharacter[0]->GetTypeId(), mainCharacter[0]->GetNetState(), mainCharacter[0]->mRoom, mainCharacter[0]->GetHoverId());
			mTimeToSendCharacterCreation = 0; // set to 0 so we don't broadcast it again
		}
	}
	else if(mainCharacter[0] != NULL) { // it has already been broadcast
		BroadcastMainElementState(mainCharacter[0]->GetNetState()); // send state

		if(mSendedPlayerStats != -1) { // send statistics if necessary
			if(mainCharacter[0]->GetLap() >= mSendedPlayerStats) {
				if(mainCharacter[0]->HasFinish()) { // race is finished
					BroadcastMainElementStats(mainCharacter[0]->GetTotalTime(), mainCharacter[0]->GetBestLapDuration(), -1);
					mSendedPlayerStats = -1;

					if(mInternetGame) {
						AddMessage(_("Press F2 when ready to return to the Meeting Room"));
					}

				}
				else if(mainCharacter[0]->GetLap() == 0) { // race has not yet started
					BroadcastMainElementStats(mainCharacter[0]->GetHoverId(), -1, 0);
					mSendedPlayerStats = 1;
				}
				else { // send statistics (lap number, total time, best lap)
					mSendedPlayerStats = mainCharacter[0]->GetLap();
					BroadcastMainElementStats(mainCharacter[0]->GetTotalTime(), mainCharacter[0]->GetBestLapDuration(), mSendedPlayerStats);
					mSendedPlayerStats++;
				}
			}
		}

		// Broadcast hits
		while(mainCharacter[0]->HitQueueCount() > 0) {
			BroadcastHit(mainCharacter[0]->GetHitQueue());
		}
	}

	// Remove disconnected opponents
	sClientToCheck++;
	if(sClientToCheck >= MR_NetworkInterface::eMaxClient) {
		sClientToCheck = 0;
	}

	if((mClientCharacter[sClientToCheck] != NULL) && !mNetInterface.IsConnected(sClientToCheck)) {
		CString lMessage;
		// Add a message indicating the the guy disconnected
		lMessage = mNetInterface.GetPlayerName(sClientToCheck);
		lMessage += " ";
		lMessage += _("has left");

		AddMessage(lMessage);

		// If only one player left, add an other message to indicate it
		if(mNetInterface.GetClientCount() == 0) {
			if(mInternetGame) {
				AddMessage(_("Press F2 when ready to return to the Meeting Room"));
			}
			else {
				AddMessage(_("You are now alone"));
			}
		}
		// Delete the client character
		mSession.GetCurrentLevel()->DeleteElement(mClient[sClientToCheck]);
		mClientCharacter[sClientToCheck] = NULL;
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
}

/**
 * Get the local player's name.
 */
const char *MR_NetworkSession::GetPlayerName() const
{
	return mNetInterface.GetPlayerName();
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
 * @param pDefaultPort The default port (@c MR_DEFAULT_NET_PORT)
 * @param pModalessDlg If this is @c NULL, the "TCP Connections" dialog is modal
 * @param pReturnMessage Callback message (see MR_NetworkInterface::ListCallBack);
 */
BOOL MR_NetworkSession::WaitConnections(HWND pWindow, const char *pTrackName, BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg, int pReturnMessage)
{
	mMasterMode = TRUE;
	mSended12SecClockUpdate = FALSE;
	mSended8SecClockUpdate = FALSE;

	return mNetInterface.MasterConnect(pWindow, pTrackName, pPromptForPort, pDefaultPort, pModalessDlg, pReturnMessage);
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
 * @param pReturnMessage Callback message (see MR_NetworkInterface::ListCallBack);
 */
BOOL MR_NetworkSession::ConnectToServer(HWND pWindow, const char *pServerIP, unsigned pPort, const char *pGameName, HWND *pModalessDlg, int pReturnMessage)
{
	mMasterMode = FALSE;

	return mNetInterface.SlaveConnect(pWindow, pServerIP, pPort, pGameName, pModalessDlg, pReturnMessage);
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
	SUPER::SetSimulationTime(pTime);
}

/**
 * Create the main character.  Get the default starting position for our client
 * ID and then place the hovercraft in that position.
 */
BOOL MR_NetworkSession::CreateMainCharacter()
{
	// Add a main character on the track
	ASSERT(mainCharacter[0] == NULL);			  // make sure we are not creating it twice
	ASSERT(mSession.GetCurrentLevel() != NULL);

	mainCharacter[0] = MR_MainCharacter::New(mNbLap, mGameOpts);

	// Insert the character in the current level
	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();

	mainCharacter[0]->mPosition = lCurrentLevel->GetStartingPos(mNetInterface.GetId());
	mainCharacter[0]->SetOrientation(lCurrentLevel->GetStartingOrientation(mNetInterface.GetId()));
	mainCharacter[0]->mRoom = lCurrentLevel->GetStartingRoom(mNetInterface.GetId());
	mainCharacter[0]->SetHoverId(mNetInterface.GetId());

	lCurrentLevel->InsertElement(mainCharacter[0], mainCharacter[0]->mRoom);

	// BroadcastMainElementCreation( mainCharacter[0]->GetTypeId(), mainCharacter[0]->GetNetState(), mainCharacter[0]->mRoom, mainCharacter[0]->GetHoverId() );

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
	int lCounter;

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
	// const int* lVisibleRoom = lLevel->GetVisibleZones( mainCharacter[0]->mRoom, lVisibleRoomCount );

	// Init priority level
	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(mClientCharacter[lCounter] != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mainCharacter[0]->mPosition.mX) / 8192;
			int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mainCharacter[0]->mPosition.mY) / 8192;

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
	int lCounter;

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
	// const int* lVisibleRoom = lLevel->GetVisibleZones( mainCharacter[0]->mRoom, lVisibleRoomCount );

	// Init priority level
	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(mClientCharacter[lCounter] != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mainCharacter[0]->mPosition.mX) / 8192;
			int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mainCharacter[0]->mPosition.mY) / 8192;

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
void MR_NetworkSession::BroadcastMainElementState(const MR_ElementNetState &pState)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_SET_MAIN_ELEM_STATE;
	lMessage.mDataLen = pState.mDataLen;

	memcpy(lMessage.mData, pState.mData, pState.mDataLen);

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

	// DEPRECATED because nobody uses 14K anymore
												  // 14K calibration
	int lMaxSend = (lCurrentTime - mLastSendElemStateFuncTime) * 27 / 1000;

	// Instead let's try refreshing more often
	//int lMaxSend = (lCurrentTime - mLastSendElemStateFuncTime) * (27 / 125);
	// That's eight times more often than Richard's original 14K calibration

	// TRACE( "lMaxSend:%d\n", lMaxSend );

	if(mSession.GetSimulationTime() <= 0) {
		lMaxSend /= 4;							  // relax on refresh before game start
	}
	else {
		if(mainCharacter[0]->mNetPriority) {
			lMaxSend += 2;						  // increse that base number.. there is a priority
		}
	}

	if(lMaxSend > 0) {
		mLastSendElemStateFuncTime = lCurrentTime;

		int lPriorityLevel[MR_NetworkInterface::eMaxClient];

		const MR_Level *lLevel = GetCurrentLevel();
		int lVisibleRoomCount;
		const int *lVisibleRoom = lLevel->GetVisibleZones(mainCharacter[0]->mRoom, lVisibleRoomCount);

		int lNbEligible = 0;

		// Init priority level
		for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
			if(mClientCharacter[lCounter] != NULL) {
				lPriorityLevel[lCounter] = lCurrentTime - mLastSendElemStateTime[lCounter];

				// Do the visibilitytest
				if(mClientCharacter[lCounter]->mRoom == mainCharacter[0]->mRoom) {
					if(mainCharacter[0]->mNetPriority) {
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
						if(lVisibleRoom[lRoom] == mClientCharacter[lCounter]->mRoom) {
							if(mainCharacter[0]->mNetPriority) {
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

				int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mainCharacter[0]->mPosition.mX) / 1024;
				int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mainCharacter[0]->mPosition.mY) / 1024;

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

		TRACE("lNbEligible:%d %d\n", lNbEligible, lMaxSend);

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
				TRACE("SendUDP:%d %d\n", lSelectedClient, lBestPriority);

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
		mainCharacter[0]->mNetPriority = FALSE;
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
void MR_NetworkSession::BroadcastMainElementStats(MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLaps)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime            = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_PLAYER_STATS;
	lMessage.mDataLen = sizeof(MR_PlayerStats);

	MR_PlayerStats *lStats = (MR_PlayerStats *) lMessage.mData;

	lStats->mFinishTime = pFinishTime;
	lStats->mBestLap = pBestLap;
	lStats->mCompletedLaps = pNbLaps;

	mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

	// Add local time
	AddResultEntry(-1, pFinishTime, pBestLap, pNbLaps);
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
void MR_NetworkSession::BroadcastHit(int pHoverIdSrc)
{
	MR_NetMessageBuffer lMessage;

	// lMessage.mSendingTime    = mSession.GetSimulationTime()>>2;
	lMessage.mMessageType = MRNM_HIT_MESSAGE;
	lMessage.mDataLen = 1;
	lMessage.mData[0] = (char) pHoverIdSrc;

	mNetInterface.BroadcastMessage(&lMessage, MR_NET_REQUIRED);

	// Add locally
	AddHitEntry(-1, pHoverIdSrc);
}

/**
 * Record a hit.
 *
 * @param pPlayerIndex Player that was hit
 * @param pPlayerFromId Player who shot him
 */
void MR_NetworkSession::AddHitEntry(int pPlayerIndex, int pPlayerFromId)
{
	// We assume that a result entry exist for both players
	PlayerResult *lEntry;
	PlayerResult **lPtr;

	// Add a point to FromID
	if(pPlayerFromId >= 0) {
		lPtr = &mHitList;

		while(*lPtr != NULL) {
			if((*lPtr)->mPlayerId == pPlayerFromId) {
				if((*lPtr)->mPlayerIndex != pPlayerIndex) {
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
		if((*lPtr)->mPlayerIndex == pPlayerIndex) {
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
void MR_NetworkSession::AddResultEntry(int pPlayerIndex, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap)
{
	// If nbLap == -1 that mean that the race is completed
	PlayerResult *lEntry;
	PlayerResult **lPtr;

	// First look if the entry do not already exist; try to make lPtr point to
	// an existing older record of this player
	lPtr = &mResultList;

	while((*lPtr != NULL) && ((*lPtr)->mPlayerIndex != pPlayerIndex)) {
		lPtr = &((*lPtr)->mNext);
	}

	if(*lPtr != NULL) {
		lEntry = *lPtr;

		// Remove the entry from the list (we will re-add it later)
		*lPtr = (*lPtr)->mNext;

		if(pPlayerIndex >= 0) {
			if(mClientCharacter[pPlayerIndex] != NULL) {
				lEntry->mCraftModel = mClientCharacter[pPlayerIndex]->GetHoverModel();
			}
		}
		else {
			if(mainCharacter[0] != NULL) {
				lEntry->mCraftModel = mainCharacter[0]->GetHoverModel();
			}
		}

	}
	else {
		lEntry = new PlayerResult;

		// Fill the fields that are only filled once
		lEntry->mPlayerIndex = pPlayerIndex;

		lEntry->mNbTimeHit = 0;
		lEntry->mNbGoodShot = 0;
		lEntry->mNextHitResult = NULL;

		// add this entry at the end of the hit list
		InsertHitEntry(lEntry);

		if(pPlayerIndex >= 0) {
			if(mClientCharacter[pPlayerIndex] != NULL) {
				lEntry->mPlayerId = mClientCharacter[pPlayerIndex]->GetHoverId();
				lEntry->mCraftModel = mClientCharacter[pPlayerIndex]->GetHoverModel();
			}
		}
		else {
			if(mainCharacter[0] != NULL) {
				lEntry->mPlayerId = mainCharacter[0]->GetHoverId();
				lEntry->mCraftModel = mainCharacter[0]->GetHoverModel();
			}
		}
	}

	// Update the entry
	lEntry->mNbCompletedLap = pNbLap;
	lEntry->mFinishTime = pFinishTime;			  // finish time of the last lap
	lEntry->mBestLap = pBestLap;

	// Insert the entry in the list
	lPtr = &mResultList;

	while(*lPtr != NULL) {
		if(((unsigned) pNbLap) == ((unsigned) (*lPtr)->mNbCompletedLap)) {
			if(pFinishTime < (*lPtr)->mFinishTime) {
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
}

/**
 * Add a typed character into our message composition display.  If it is @c \\n or
 * @c \\r the user hit the enter key and we send it, after running it through the
 * Ascii2Simple() function.
 *
 * @param pKey The key the user pressed
 */
void MR_NetworkSession::AddMessageKey(char pKey)
{
	EnterCriticalSection(&mChatMutex);

	int lStrLen = strlen(mChatEditBuffer);

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

	LeaveCriticalSection(&mChatMutex);
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
	for(int lCounter = CHAT_MESSAGE_STACK - 1; lCounter > 0; lCounter--) {
		mMessageStack[lCounter] = mMessageStack[lCounter - 1];
	}

	mMessageStack[0].mCreationTime = time(NULL);

	mMessageStack[0].mBuffer = Ascii2Simple(mNetInterface.GetPlayerName(pPlayerIndex));
	mMessageStack[0].mBuffer += Ascii2Simple('>');
	mMessageStack[0].mBuffer += CString(pMessage, pMessageLen);
}