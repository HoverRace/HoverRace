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

// Local structures
class MR_PlayerStats
{
	public:

		MR_SimulationTime mFinishTime;
		MR_SimulationTime mBestLap;
		MR_Int8 mCompletedLaps;

};

// Functions implementation

MR_NetworkSession::MR_NetworkSession(BOOL pInternetGame, int pMajorID, int pMinorID, HWND pWindow)
:MR_ClientSession()
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

MR_NetworkSession::~MR_NetworkSession()
{

	if(mInternetGame && (mMainCharacter1 != NULL)) {
		int lCurrentModel = mMainCharacter1->GetHoverModel();

		// Send results to the record server
		if(mMajorID != -1) {
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

				int lTotalLap = mMainCharacter1->GetTotalLap();

				MR_SendRaceResult(mWindow, mSession.GetTitle(), lPlayer->mBestLap, mMajorID, mMinorID, mNetInterface.GetPlayerName(-1), mSession.GetCurrentMazeFile()->GetCheckSum(), mMainCharacter1->GetHoverModel(), (lPlayer->mNbCompletedLap == -1) ? lPlayer->mFinishTime : 0, (lPlayer->mNbCompletedLap == -1) ? lTotalLap : 0, lNbPlayer);

				// Report ladder matchs
				if(lNbPlayer == 2) {
					int lWinnerIndex = mResultList->mPlayerIndex;
					// If you lost and the other player have finished
					if((lWinnerIndex != -1) && (mResultList->mNbCompletedLap == -1)) {
						// Report the Lost
						MR_SendLadderResult(mWindow, mNetInterface.GetPlayerName(lWinnerIndex), mOpponendMajorID, mOpponendMinorID, mNetInterface.GetPlayerName(-1), mMajorID, mMinorID, mSession.GetTitle(), lTotalLap);

					}
				}
			}
		}
	}

	PlayerResult *lCurrent = mResultList;

	mResultList = NULL;

	while(lCurrent != NULL) {
		PlayerResult *lPrev = lCurrent;

		lCurrent = lCurrent->mNext;
		delete lPrev;
	}

}

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

void MR_NetworkSession::GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbLap, MR_SimulationTime & pFinishTime, MR_SimulationTime & pBestLap) const
{
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

int MR_NetworkSession::GetNbPlayers() const
{
	// Return the number of players still playing???
	// return mNetInterface.GetClientCount()+1;
	return ResultAvaillable();
}

const MR_MainCharacter *MR_NetworkSession::GetPlayer(int pPlayerIndex) const
{
	const MR_MainCharacter *lReturnValue = NULL;

	if(pPlayerIndex == 0) {
		lReturnValue = mMainCharacter1;
	}
	else if(pPlayerIndex <= MR_NetworkInterface::eMaxClient) {
		lReturnValue = mClientCharacter[pPlayerIndex - 1];
	}
	return lReturnValue;
}

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

void MR_NetworkSession::Process(int pSpeedFactor)
{
	ReadNet();
	MR_ClientSession::Process(pSpeedFactor);
	WriteNet();
	ReadNet();

}

BOOL MR_NetworkSession::LoadNew(const char *pTitle, MR_RecordFile * pMazeFile, int pNbLap, BOOL pAllowWeapons, MR_VideoBuffer * pVideo)
{
	BOOL lReturnValue = MR_ClientSession::LoadNew(pTitle, pMazeFile, pNbLap, pAllowWeapons, pVideo);

	if(lReturnValue) {
		mSession.GetCurrentLevel()->SetBroadcastHook(ElementCreationHook, PermElementStateHook, this);
	}

	return lReturnValue;
}

void MR_NetworkSession::ElementCreationHook(MR_FreeElement * pElement, int pRoom, void *pThis)
{
	((MR_NetworkSession *) pThis)->BroadcastAutoElementCreation(pElement->GetTypeId(), pElement->GetNetState(), pRoom);
}

void MR_NetworkSession::PermElementStateHook(MR_FreeElement * pElement, int pRoom, int pPermId, void *pThis)
{
	((MR_NetworkSession *) pThis)->BroadcastPermElementState(pPermId, pElement->GetNetState(), pRoom);
}

void MR_NetworkSession::ReadNet()
{
	DWORD lTimeStamp;
	int lMessageType;
	int lMessageLen;
	const MR_UInt8 *lMessage;
	int lClientId;

	lTimeStamp = mSession.GetSimulationTime();

	while(mNetInterface.FetchMessage(lTimeStamp, lMessageType, lMessageLen, lMessage, lClientId)) {
		switch (lMessageType) {
			case MRNM_SET_TIME:
				{
					MR_SimulationTime lNewTime = *(MR_Int32 *) & (lMessage[0]);
					// lNewTime += mNetInterface.GetAvgLag( lClientId ); Done by SetSimulationTime
	
					mSession.SetSimulationTime(lNewTime);
				}
				break;

			case MRNM_CREATE_MAIN_ELEM:
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

			case MRNM_SET_MAIN_ELEM_STATE:
				if(mClientCharacter[lClientId] != NULL) {
					// Drop the message if there was a recent collision on that item
					int lLastCollisionAge = mSession.GetSimulationTime() - mClientCharacter[lClientId]->mLastCollisionTime;

					if(lLastCollisionAge < (mNetInterface.GetAvgLag(lClientId) + 40)) {
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

void MR_NetworkSession::WriteNet()
{
	static unsigned int sClientToCheck = 0;		  //verified ok even if it may seen weird

	if(mMasterMode) {
		if(!mSended8SecClockUpdate) {
			MR_SimulationTime lCurrentTime = mSession.GetSimulationTime();

			if(!mSended12SecClockUpdate) {
				if(lCurrentTime >= -11500) {
					BroadcastTime();
					mSended12SecClockUpdate = TRUE;
				}
			}
			else {
				if(lCurrentTime >= -8500) {
					BroadcastTime();
					mSended8SecClockUpdate = TRUE;
				}
			}
		}

	}

	if(mTimeToSendCharacterCreation != 0) {
		if(mSession.GetSimulationTime() >= mTimeToSendCharacterCreation) {
			BroadcastMainElementCreation(mMainCharacter1->GetTypeId(), mMainCharacter1->GetNetState(), mMainCharacter1->mRoom, mMainCharacter1->GetHoverId());
			mTimeToSendCharacterCreation = 0;
		}
	}
	else if(mMainCharacter1 != NULL) {
		BroadcastMainElementState(mMainCharacter1->GetNetState());

		if(mSendedPlayerStats != -1) {
			if(mMainCharacter1->GetLap() >= mSendedPlayerStats) {
				if(mMainCharacter1->HasFinish()) {
					BroadcastMainElementStats(mMainCharacter1->GetTotalTime(), mMainCharacter1->GetBestLapDuration(), -1);
					mSendedPlayerStats = -1;

					if(mInternetGame) {
						AddMessage(MR_LoadString(IDS_F2_TORETURN));
					}

				}
				else if(mMainCharacter1->GetLap() == 0) {
					BroadcastMainElementStats(mMainCharacter1->GetHoverId(), -1, 0);
					mSendedPlayerStats = 1;
				}
				else {
					mSendedPlayerStats = mMainCharacter1->GetLap();
					BroadcastMainElementStats(mMainCharacter1->GetTotalTime(), mMainCharacter1->GetBestLapDuration(), mSendedPlayerStats);
					mSendedPlayerStats++;
				}
			}
		}
		//Broadcast hits
		while(mMainCharacter1->HitQueueCount() > 0) {
			BroadcastHit(mMainCharacter1->GetHitQueue());
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
		lMessage += MR_LoadString(IDS_HAS_LEFT);

		AddMessage(lMessage);

		// If only one player left, add an other message to indicate it
		if(mNetInterface.GetClientCount() == 0) {
			if(mInternetGame) {
				AddMessage(MR_LoadString(IDS_ALONE_F2));
			}
			else {
				AddMessage(MR_LoadString(IDS_ALONE));
			}
		}
		// Delete the client character
		mSession.GetCurrentLevel()->DeleteElement(mClient[sClientToCheck]);
		mClientCharacter[sClientToCheck] = NULL;
	}

}

void MR_NetworkSession::SetPlayerName(const char *pPlayerName)
{
	mNetInterface.SetPlayerName(pPlayerName);
}

const char *MR_NetworkSession::GetPlayerName() const
{
	return mNetInterface.GetPlayerName();
}

BOOL MR_NetworkSession::WaitConnections(HWND pWindow, const char *pTrackName, BOOL pPromptForPort, unsigned pDefaultPort, HWND * pModalessDlg, int pReturnMessage)
{
	mMasterMode = TRUE;
	mSended12SecClockUpdate = FALSE;
	mSended8SecClockUpdate = FALSE;

	return mNetInterface.MasterConnect(pWindow, pTrackName, pPromptForPort, pDefaultPort, pModalessDlg, pReturnMessage);
}

BOOL MR_NetworkSession::PreConnectToServer(HWND pWindow, CString & pTrackName)
{
	mMasterMode = FALSE;
	return mNetInterface.SlavePreConnect(pWindow, pTrackName);
}

BOOL MR_NetworkSession::ConnectToServer(HWND pWindow, const char *pServerIP, unsigned pPort, const char *pGameName, HWND * pModalessDlg, int pReturnMessage)
{
	mMasterMode = FALSE;

	return mNetInterface.SlaveConnect(pWindow, pServerIP, pPort, pGameName, pModalessDlg, pReturnMessage);
}

void MR_NetworkSession::SetSimulationTime(MR_SimulationTime pTime)
{
	pTime += mNetInterface.GetLagFromServer();

	if(mTimeToSendCharacterCreation != 0) {
		mTimeToSendCharacterCreation = min(pTime + 2000, mTimeToSendCharacterCreation);
	}
	MR_ClientSession::SetSimulationTime(pTime);
}

BOOL MR_NetworkSession::CreateMainCharacter()
{
	// Add a main character on the track

	ASSERT(mMainCharacter1 == NULL);			  // why creating it twice?
	ASSERT(mSession.GetCurrentLevel() != NULL);

	mMainCharacter1 = MR_MainCharacter::New(mNbLap, mAllowWeapons);

	// Insert the character in the current level
	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();

	mMainCharacter1->mPosition = lCurrentLevel->GetStartingPos(mNetInterface.GetId());
	mMainCharacter1->SetOrientation(lCurrentLevel->GetStartingOrientation(mNetInterface.GetId()));
	mMainCharacter1->mRoom = lCurrentLevel->GetStartingRoom(mNetInterface.GetId());
	mMainCharacter1->SetHoverId(mNetInterface.GetId());

	lCurrentLevel->InsertElement(mMainCharacter1, mMainCharacter1->mRoom);

	// BroadcastMainElementCreation( mMainCharacter1->GetTypeId(), mMainCharacter1->GetNetState(), mMainCharacter1->mRoom, mMainCharacter1->GetHoverId() );

	return TRUE;
}

void MR_NetworkSession::BroadcastMainElementCreation(const MR_ObjectFromFactoryId & pId, const MR_ElementNetState & pState, int pRoom, int pHoverId)
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

void MR_NetworkSession::BroadcastAutoElementCreation(const MR_ObjectFromFactoryId & pId, const MR_ElementNetState & pState, int pRoom)
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

	// Init priority level
	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(mClientCharacter[lCounter] != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mMainCharacter1->mPosition.mX) / 8192;
			int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mMainCharacter1->mPosition.mY) / 8192;

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

	// Now to near clients( the second broadcast bring security)
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

void MR_NetworkSession::BroadcastPermElementState(int pPermId, const MR_ElementNetState & pState, int pRoom)
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

	// Init priority level
	for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
		if(mClientCharacter[lCounter] != NULL) {
			lPriorityLevel[lCounter] = 0;

			int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mMainCharacter1->mPosition.mX) / 8192;
			int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mMainCharacter1->mPosition.mY) / 8192;

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

	// Now to near clients( the second broadcast bring security)
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

void MR_NetworkSession::BroadcastMainElementState(const MR_ElementNetState & pState)
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
		if(mMainCharacter1->mNetPriority) {
			lMaxSend += 2;						  // increse that base number.. there is a priority
		}
	}

	if(lMaxSend > 0) {
		mLastSendElemStateFuncTime = lCurrentTime;

		int lPriorityLevel[MR_NetworkInterface::eMaxClient];

		const MR_Level *lLevel = GetCurrentLevel();
		int lVisibleRoomCount;
		const int *lVisibleRoom = lLevel->GetVisibleZones(mMainCharacter1->mRoom, lVisibleRoomCount);

		int lNbEligible = 0;

		// Init priority level
		for(int lCounter = 0; lCounter < MR_NetworkInterface::eMaxClient; lCounter++) {
			if(mClientCharacter[lCounter] != NULL) {
				lPriorityLevel[lCounter] = lCurrentTime - mLastSendElemStateTime[lCounter];

				// Do the visibilitytest
				if(mClientCharacter[lCounter]->mRoom == mMainCharacter1->mRoom) {
					if(mMainCharacter1->mNetPriority) {
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
							if(mMainCharacter1->mNetPriority) {
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

				int lDistanceX = (mClientCharacter[lCounter]->mPosition.mX - mMainCharacter1->mPosition.mX) / 1024;
				int lDistanceY = (mClientCharacter[lCounter]->mPosition.mY - mMainCharacter1->mPosition.mY) / 1024;

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
		mMainCharacter1->mNetPriority = FALSE;
	}
}

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

void MR_NetworkSession::AddHitEntry(int pPlayerIndex, int pPlayerFromId)
{
	//We assume that a result entry exist for both players
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
	//
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

void MR_NetworkSession::InsertHitEntry(PlayerResult * pEntry)
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

void MR_NetworkSession::AddResultEntry(int pPlayerIndex, MR_SimulationTime pFinishTime, MR_SimulationTime pBestLap, int pNbLap)
{
	// If nbLap == -1 that mean that the race is completed

	PlayerResult *lEntry;
	PlayerResult **lPtr;

	// First look if the entry do not already exist

	lPtr = &mResultList;

	while((*lPtr != NULL) && ((*lPtr)->mPlayerIndex != pPlayerIndex)) {
		lPtr = &((*lPtr)->mNext);
	}

	if(*lPtr != NULL) {
		lEntry = *lPtr;

		// Remove the entry from the list
		*lPtr = (*lPtr)->mNext;

		if(pPlayerIndex >= 0) {
			if(mClientCharacter[pPlayerIndex] != NULL) {
				lEntry->mCraftModel = mClientCharacter[pPlayerIndex]->GetHoverModel();
			}
		}
		else {
			if(mMainCharacter1 != NULL) {
				lEntry->mCraftModel = mMainCharacter1->GetHoverModel();
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
			if(mMainCharacter1 != NULL) {
				lEntry->mPlayerId = mMainCharacter1->GetHoverId();
				lEntry->mCraftModel = mMainCharacter1->GetHoverModel();
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

void MR_NetworkSession::GetCurrentMessage(char *pDest) const
{
	EnterCriticalSection(&((MR_NetworkSession *) this)->mChatMutex);

	strcpy(pDest, mChatEditBuffer);

	LeaveCriticalSection(&((MR_NetworkSession *) this)->mChatMutex);
} void MR_NetworkSession::AddChatMessage(int pPlayerIndex, const char *pMessage, int pMessageLen)
{
	for(int lCounter = MR_CHAT_MESSAGE_STACK - 1; lCounter > 0; lCounter--) {
		mMessageStack[lCounter] = mMessageStack[lCounter - 1];
	}

	mMessageStack[0].mCreationTime = time(NULL);

	mMessageStack[0].mBuffer = Ascii2Simple(mNetInterface.GetPlayerName(pPlayerIndex));
	mMessageStack[0].mBuffer += Ascii2Simple('>');
	mMessageStack[0].mBuffer += CString(pMessage, pMessageLen);
}
