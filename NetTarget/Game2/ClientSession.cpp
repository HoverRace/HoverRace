// ClientSession.cpp
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

#include "ClientSession.h"
#include "TrackSelect.h"
#include "../MazeCompiler/TrackCommonStuff.h"

MR_ClientSession::MR_ClientSession()
:mSession(TRUE)
{
	memset(mMainCharacters, 0, sizeof(mMainCharacters));
	mMainCharacter1 = NULL;
	mMainCharacter2 = NULL;
	mMainCharacter3 = NULL;
	mMainCharacter4 = NULL;
	mBackImage = NULL;
	mMap = NULL;
	mNbLap = 1;
	mAllowWeapons = TRUE;
	mAllowCans = TRUE;
	mAllowMines = TRUE;
	mAllowedCraftMask = MR_GetDefaultAllowedCraftMask();
	mGameRuleRuntime = NULL;
	mRuleBasedMatchFinished = FALSE;
	ResetLocalHitStats();

	InitializeCriticalSection(&mChatMutex);
}

void MR_ClientSession::SyncLegacyMainCharacterPointers()
{
	mMainCharacter1 = mMainCharacters[0];
	mMainCharacter2 = mMainCharacters[1];
	mMainCharacter3 = mMainCharacters[2];
	mMainCharacter4 = mMainCharacters[3];
}

MR_ClientSession::~MR_ClientSession()
{
	delete mGameRuleRuntime;
	delete[]mBackImage;
	delete mMap;

	DeleteCriticalSection(&mChatMutex);
}

BOOL MR_ClientSession::Process(int pSpeedFactor)
{
	BOOL lReturnValue = mSession.Simulate();

	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		MR_MainCharacter *lPlayer = mMainCharacters[i];

		if(lPlayer == NULL) {
			continue;
		}

		while(lPlayer->RaceEventQueueCount() > 0) {
			MR_MainCharacter::RaceEvent lEvent = lPlayer->GetRaceEvent();

			if(lEvent.mType == MR_MainCharacter::RaceEvent::eCheckpoint) {
				NotifyRuleCheckpoint(lPlayer->GetHoverId(), lEvent.mValue);
			}
			else if(lEvent.mType == MR_MainCharacter::RaceEvent::eLapComplete) {
				NotifyRuleLapComplete(lPlayer->GetHoverId(), lEvent.mValue);
			}
		}

		if(ShouldProcessLocalHitQueues()) {
			while(lPlayer->HitQueueCount() > 0) {
				MR_MainCharacter::HitEntry lHit = lPlayer->GetHitQueue();
				NotifyRuleHit(lPlayer->GetHoverId(), lHit.mHoverId,
					lHit.mElementId);
			}
		}
	}

	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->OnProcessTick(*this);
	}

	return lReturnValue;
}

void MR_ClientSession::ReadLevelAttrib(MR_RecordFile * pRecordFile, MR_VideoBuffer * pVideo)
{
	// Read level background palette
	if((pVideo != NULL) && (pRecordFile->GetNbRecords() >= 3)) {
		pRecordFile->SelectRecord(2);

		{
			CArchive lArchive(pRecordFile, CArchive::load | CArchive::bNoFlushOnDelete);

			int lImageType;

			lArchive >> lImageType;

			if(lImageType == MR_RAWBITMAP) {
				MR_UInt8 *lPalette = new MR_UInt8[MR_BACK_COLORS * 3];

				if(mBackImage == NULL) {
					mBackImage = new MR_UInt8[MR_BACK_X_RES * MR_BACK_Y_RES];
				}

				lArchive.Read(lPalette, MR_BACK_COLORS * 3);
				lArchive.Read(mBackImage, MR_BACK_X_RES * MR_BACK_Y_RES);

				pVideo->SetBackPalette(lPalette);
			}
		}
	}
	// Read map section
	if(pRecordFile->GetNbRecords() >= 4) {
		pRecordFile->SelectRecord(3);
		{
			CArchive lArchive(pRecordFile, CArchive::load | CArchive::bNoFlushOnDelete);

			int lX0;
			int lX1;
			int lY0;
			int lY1;

			MR_Sprite *lMapSprite = new MR_Sprite;

			lArchive >> lX0;
			lArchive >> lX1;
			lArchive >> lY0;
			lArchive >> lY1;

			lMapSprite->Serialize(lArchive);

			SetMap(lMapSprite, lX0, lY0, lX1, lY1);
		}
	}
	// Read level midi stream
	if(pRecordFile->GetNbRecords() >= 5) {
		pRecordFile->SelectRecord(4);
		{
			// TODO
		}
	}
}

BOOL MR_ClientSession::LoadNew(const char *pTitle, MR_RecordFile * pMazeFile,
	int pNbLap, BOOL pAllowWeapons, BOOL pAllowCans, BOOL pAllowMines,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings,
	MR_VideoBuffer * pVideo)
{
	BOOL lReturnValue;
	mNbLap = pNbLap;
	mAllowWeapons = pAllowWeapons;
	mAllowCans = pAllowCans;
	mAllowMines = pAllowMines;
	mAllowedCraftMask = MR_NormalizeAllowedCraftMask(pAllowedCraftMask);
	mGameRuleSettings = pGameRuleSettings;
	MR_NormalizeGameRuleSettings(mGameRuleSettings);
	ResetRuleRuntime();
	mGameRuleRuntime = MR_GameRuleRuntime::Create(mGameRuleSettings);
	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->ApplySessionOptions(mAllowWeapons, mAllowCans, mAllowMines);
	}
	mRuleBasedMatchFinished = FALSE;
	ResetLocalHitStats();
	lReturnValue = mSession.LoadNew(pTitle, pMazeFile);

	if(lReturnValue) {
		ReadLevelAttrib(pMazeFile, pVideo);
		ApplyGameOptions();
		if(mGameRuleRuntime != NULL) {
			mGameRuleRuntime->OnInit(*this);
		}
	}

	return lReturnValue;
}

void MR_ClientSession::ApplyGameOptions()
{
	static const MR_UInt16 TRACK_OBJECT_DLL_ID = 1;
	static const MR_UInt16 TRACK_CAN_CLASS_ID = 152;
	static const MR_3DCoordinate HIDDEN_POSITION(0, 0, -32000);

	if(mAllowCans) {
		return;
	}

	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	if(lCurrentLevel == NULL) {
		return;
	}

	const int lRoomCount = lCurrentLevel->GetRoomCount();
	for(int lRoom = 0; lRoom < lRoomCount; lRoom++) {
		MR_FreeElementHandle lCurrent = lCurrentLevel->GetFirstFreeElement(lRoom);
		while(lCurrent != NULL) {
			MR_FreeElementHandle lNext = MR_Level::GetNextFreeElement(lCurrent);
			MR_FreeElement *lElement = MR_Level::GetFreeElement(lCurrent);
			const MR_ObjectFromFactoryId &lTypeId = lElement->GetTypeId();
			const bool lHideCan =
				!mAllowCans &&
				(lTypeId.mDllId == TRACK_OBJECT_DLL_ID) &&
				(lTypeId.mClassId == TRACK_CAN_CLASS_ID);

			if(lHideCan) {
				lElement->mPosition = HIDDEN_POSITION;
				lCurrentLevel->MoveElement(lCurrent, MR_Level::eNonClassified);
			}

			lCurrent = lNext;
		}
	}
}

void MR_ClientSession::SetLapCount(int pNbLap)
{
	if(pNbLap >= 1) {
		mNbLap = pNbLap;
	}
}

const MR_UInt8 *MR_ClientSession::GetBackImage() const
{
	return mBackImage;
}

// Main character controll and interogation

BOOL MR_ClientSession::CreateMainCharacter(int pPlayerIndex)
{
	ASSERT(mSession.GetCurrentLevel() != NULL);

	if((pPlayerIndex < 0) || (pPlayerIndex >= MR_MAX_LOCAL_PLAYER)) {
		return FALSE;
	}

	ASSERT(mMainCharacters[pPlayerIndex] == NULL);			  // why creating it twice?

	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();
	if(pPlayerIndex >= lCurrentLevel->GetPlayerCount()) {
		return FALSE;
	}

	mMainCharacters[pPlayerIndex] = MR_MainCharacter::New(mNbLap, mAllowWeapons, mAllowCans,
		mAllowMines, mAllowedCraftMask);

	MR_GameRuleSpawnContext lSpawnContext;
	lSpawnContext.mHoverId = pPlayerIndex;
	lSpawnContext.mSpawnSlot = pPlayerIndex;
	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->OnPreSpawn(*this, lSpawnContext);
	}

	mMainCharacters[pPlayerIndex]->mRoom =
		lCurrentLevel->GetStartingRoom(lSpawnContext.mSpawnSlot);
	mMainCharacters[pPlayerIndex]->mPosition =
		lCurrentLevel->GetStartingPos(lSpawnContext.mSpawnSlot);
	mMainCharacters[pPlayerIndex]->SetOrientation(
		lCurrentLevel->GetStartingOrientation(lSpawnContext.mSpawnSlot));
	mMainCharacters[pPlayerIndex]->SetHoverId(pPlayerIndex);

	lCurrentLevel->InsertElement(mMainCharacters[pPlayerIndex],
		lCurrentLevel->GetStartingRoom(pPlayerIndex));
	SyncLegacyMainCharacterPointers();

	return TRUE;
}

BOOL MR_ClientSession::CreateMainCharacter()
{
	return CreateMainCharacter(0);
}

MR_MainCharacter *MR_ClientSession::GetMainCharacter() const
{
	return GetMainCharacter(0);
}

MR_MainCharacter *MR_ClientSession::GetMainCharacter(int pPlayerIndex) const
{
	if((pPlayerIndex < 0) || (pPlayerIndex >= MR_MAX_LOCAL_PLAYER)) {
		return NULL;
	}

	return mMainCharacters[pPlayerIndex];
}

const MR_GameRuleSettings &MR_ClientSession::GetGameRuleSettings() const
{
	return mGameRuleSettings;
}

BOOL MR_ClientSession::UsesHitResults() const
{
	return (mGameRuleRuntime != NULL) && mGameRuleRuntime->UsesHitScoring();
}

void MR_ClientSession::SetPlayerCraftCollision(int pHoverId, BOOL pEnabled)
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			mMainCharacters[i]->SetCraftCollisionEnabled(pEnabled);
		}
	}
}

void MR_ClientSession::SetPlayerColumnInteraction(int pHoverId, BOOL pEnabled)
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			mMainCharacters[i]->SetColumnInteractionEnabled(pEnabled);
		}
	}
}

void MR_ClientSession::SetPlayerRenderOpacity(int pHoverId, float pOpacity)
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			mMainCharacters[i]->SetRenderOpacity(pOpacity);
		}
	}
}

BOOL MR_ClientSession::IsLocalHoverId(int pHoverId) const
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL MR_ClientSession::GetGameRulePlayerState(int pHoverId,
	MR_GameRulePlayerState &pState) const
{
	const MR_MainCharacter *lPlayer = FindPlayerByHoverId(pHoverId);
	const int lPlayerIndex = GetLocalPlayerIndexByHoverId(pHoverId);

	pState = MR_GameRulePlayerState();
	pState.mHoverId = pHoverId;

	if(lPlayer == NULL) {
		return FALSE;
	}

	pState.mLapNumber = lPlayer->GetLap();
	pState.mCheckpointIndex = lPlayer->GetCurrentCheckpoint();
	pState.mHasFinished = lPlayer->HasFinish();
	pState.mCraftCollisionEnabled = lPlayer->GetCraftCollisionEnabled();
	pState.mColumnInteractionEnabled = lPlayer->GetColumnInteractionEnabled();
	pState.mBaseOpacity = lPlayer->GetRenderOpacity();

	if(lPlayerIndex >= 0) {
		pState.mHitOtherCount = mLocalGoodShots[lPlayerIndex];
		pState.mHitByOtherCount = mLocalHitByOthers[lPlayerIndex];
	}

	return TRUE;
}

const MR_MainCharacter *MR_ClientSession::FindPlayerByHoverId(int pHoverId) const
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			return mMainCharacters[i];
		}
	}

	return NULL;
}

float MR_ClientSession::GetRemotePlayerOpacityForView(
	const MR_MainCharacter *pViewingCharacter,
	const MR_MainCharacter *pTargetCharacter) const
{
	if((pViewingCharacter == NULL) || (pTargetCharacter == NULL)) {
		return 1.0f;
	}

	if(mGameRuleRuntime != NULL) {
		return mGameRuleRuntime->GetRemotePlayerOpacityForView(*this,
			pViewingCharacter, pTargetCharacter,
			pTargetCharacter->GetRenderOpacity());
	}

	return pTargetCharacter->GetRenderOpacity();
}

BOOL MR_ClientSession::FormatRuleHudText(
	const MR_MainCharacter *pViewingCharacter, MR_SimulationTime pTime,
	char *pMainBuffer, int pMainBufferLen, char *pSecondaryBuffer,
	int pSecondaryBufferLen) const
{
	if((pMainBuffer == NULL) || (pMainBufferLen <= 0) ||
		(pSecondaryBuffer == NULL) || (pSecondaryBufferLen <= 0))
	{
		return FALSE;
	}

	pMainBuffer[0] = 0;
	pSecondaryBuffer[0] = 0;

	return (mGameRuleRuntime != NULL) &&
		mGameRuleRuntime->FormatHudText(*this, pViewingCharacter, pTime,
			pMainBuffer, pMainBufferLen, pSecondaryBuffer, pSecondaryBufferLen);
}

BOOL MR_ClientSession::IsStandardGameRule() const
{
	return (mGameRuleRuntime == NULL) || mGameRuleRuntime->IsStandardRule();
}

int MR_ClientSession::GetHitRank(int pHoverId) const
{
	for(int lPosition = 0; lPosition < ResultAvaillable(); lPosition++) {
		const char *lPlayerName;
		int lResultHoverId;
		BOOL lConnected;
		int lNbFor;
		int lNbAgainst;

		GetHitResult(lPosition, lPlayerName, lResultHoverId, lConnected,
			lNbFor, lNbAgainst);
		if(lResultHoverId == pHoverId) {
			return lPosition + 1;
		}
	}

	return 0;
}

void MR_ClientSession::EndRuleBasedMatch()
{
	mRuleBasedMatchFinished = TRUE;
	DisableAllWeapons();
	DestroyMissiles();
}

BOOL MR_ClientSession::IsRuleBasedMatchFinished() const
{
	return mRuleBasedMatchFinished;
}

void MR_ClientSession::SetSimulationTime(MR_SimulationTime pTime)
{
	mSession.SetSimulationTime(pTime);
}

MR_SimulationTime MR_ClientSession::GetSimulationTime() const
{
	return mSession.GetSimulationTime();
}

void MR_ClientSession::SetControlState(const int *pStates, int pStateCount)
{
	if(pStates == NULL) {
		return;
	}

	const int lCount = min(pStateCount, MR_MAX_LOCAL_PLAYER);
	for(int i = 0; i < lCount; ++i) {
		if(mMainCharacters[i] != NULL) {
			mMainCharacters[i]->SetControlState(pStates[i],
				mSession.GetSimulationTime());
		}
	}
}

const MR_Level *MR_ClientSession::GetCurrentLevel() const
{
	MR_GameSession *lSession = (MR_GameSession *) & mSession;

	return lSession->GetCurrentLevel();
}

int MR_ClientSession::ResultAvaillable() const
{
	return GetNbPlayers();
}

void MR_ClientSession::GetResult(int pPosition, const char *&pPlayerName, int &pId, BOOL &pConnected, int &pNbLap, MR_SimulationTime &pFinishTime, MR_SimulationTime &pBestLap, int &pNbSplit, MR_SimulationTime &pFinishFirstSplit, MR_SimulationTime &pFirstSplitDifference, MR_SimulationTime &pFinishSecondSplit, MR_SimulationTime &pSecondSplitDifference) const
{
	int lSorted[MR_MAX_LOCAL_PLAYER];
	int lSortedCount = 0;

	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if(mMainCharacters[i] != NULL) {
			lSorted[lSortedCount++] = i;
		}
	}

	for(int i = 0; i < lSortedCount; ++i) {
		for(int j = i + 1; j < lSortedCount; ++j) {
			MR_MainCharacter *lLeft = mMainCharacters[lSorted[i]];
			MR_MainCharacter *lRight = mMainCharacters[lSorted[j]];
			const int lLeftLap = lLeft->HasFinish() ? -1 : lLeft->GetLap();
			const int lRightLap = lRight->HasFinish() ? -1 : lRight->GetLap();
			const int lLeftCheckpoint = lLeft->GetCurrentCheckpoint();
			const int lRightCheckpoint = lRight->GetCurrentCheckpoint();
			BOOL lSwap = FALSE;

			if(((unsigned) lRightLap) > ((unsigned) lLeftLap)) {
				lSwap = TRUE;
			}
			else if(lRightLap == lLeftLap) {
				if(lRightCheckpoint > lLeftCheckpoint) {
					lSwap = TRUE;
				}
				else if((lRightCheckpoint == lLeftCheckpoint) &&
					(lRight->GetTotalTime() < lLeft->GetTotalTime()))
				{
					lSwap = TRUE;
				}
			}

			if(lSwap) {
				int lTemp = lSorted[i];
				lSorted[i] = lSorted[j];
				lSorted[j] = lTemp;
			}
		}
	}

	if((pPosition < 0) || (pPosition >= lSortedCount)) {
		pPlayerName = "?";
		pId = -1;
		pConnected = FALSE;
		pNbLap = 0;
		pFinishTime = 0;
		pBestLap = 0;
		pNbSplit = 0;
		pFinishFirstSplit = 0;
		pFirstSplitDifference = 0;
		pFinishSecondSplit = 0;
		pSecondSplitDifference = 0;
		return;
	}

	MR_MainCharacter *lPlayer = mMainCharacters[lSorted[pPosition]];
	static char lNameBuffer[16];

	sprintf(lNameBuffer, "Player %d", lPlayer->GetHoverId() + 1);
	pPlayerName = lNameBuffer;
	pId = lPlayer->GetHoverId();
	pConnected = TRUE;
	pNbLap = lPlayer->HasFinish() ? -1 : lPlayer->GetLap();
	pFinishTime = lPlayer->GetTotalTime();
	pBestLap = lPlayer->GetBestLapDuration();
	pNbSplit = lPlayer->GetCurrentCheckpoint();
	pFinishFirstSplit = lPlayer->GetFirstSplitCompletion();
	pFirstSplitDifference = lPlayer->GetFirstSplitDifference();
	pFinishSecondSplit = lPlayer->GetSecondSplitCompletion();
	pSecondSplitDifference = lPlayer->GetSecondSplitDifference();
}

void MR_ClientSession::GetHitResult(int pPosition, const char *&pPlayerName, int &pId, BOOL & pConnected, int &pNbHitOther, int &pNbHitHimself) const
{
	int lSorted[MR_MAX_LOCAL_PLAYER];
	int lSortedCount = 0;

	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if(mMainCharacters[i] != NULL) {
			lSorted[lSortedCount++] = i;
		}
	}

	for(int i = 0; i < lSortedCount; ++i) {
		for(int j = i + 1; j < lSortedCount; ++j) {
			const int lLeftScore =
				mLocalGoodShots[lSorted[i]] - mLocalHitByOthers[lSorted[i]];
			const int lRightScore =
				mLocalGoodShots[lSorted[j]] - mLocalHitByOthers[lSorted[j]];
			if(lRightScore > lLeftScore) {
				int lTemp = lSorted[i];
				lSorted[i] = lSorted[j];
				lSorted[j] = lTemp;
			}
		}
	}

	if((pPosition < 0) || (pPosition >= lSortedCount)) {
		pPlayerName = "?";
		pId = -1;
		pConnected = FALSE;
		pNbHitOther = 0;
		pNbHitHimself = 0;
		return;
	}

	MR_MainCharacter *lPlayer = mMainCharacters[lSorted[pPosition]];
	static char lNameBuffer[16];

	sprintf(lNameBuffer, "Player %d", lPlayer->GetHoverId() + 1);
	pPlayerName = lNameBuffer;
	pId = lPlayer->GetHoverId();
	pConnected = TRUE;
	pNbHitOther = mLocalGoodShots[lSorted[pPosition]];
	pNbHitHimself = mLocalHitByOthers[lSorted[pPosition]];
}

int MR_ClientSession::GetNbPlayers() const
{
	int lReturnValue = 0;

	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if(mMainCharacters[i] != NULL) {
			lReturnValue++;
		}
	}

	return lReturnValue;
}

int MR_ClientSession::GetRank(const MR_MainCharacter * pPlayer) const
{
	int lReturnValue = 1;

	if((pPlayer != NULL) && pPlayer->HasFinish()) {
		for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
			MR_MainCharacter *lOtherPlayer = mMainCharacters[i];
			if((lOtherPlayer != NULL) && (lOtherPlayer != pPlayer) && lOtherPlayer->HasFinish()) {
				if(lOtherPlayer->GetTotalTime() < pPlayer->GetTotalTime()) {
					lReturnValue++;
				}
			}
		}
	}

	return lReturnValue;
}

void MR_ClientSession::SetMap(MR_Sprite * pMap, int pX0, int pY0, int pX1, int pY1)
{
	delete mMap;

	mMap = pMap;

	mX0Map = pX0;
	mY0Map = pY0;
	mWidthMap = pX1 - pX0;
	mHeightMap = pY1 - pY0;

	mWidthSprite = mMap->GetItemWidth();
	mHeightSprite = mMap->GetItemHeight();

}

const MR_Sprite *MR_ClientSession::GetMap() const
{
	return mMap;
}

void MR_ClientSession::ConvertMapCoordinate(int &pX, int &pY, int pRatio) const
{
	pX = (pX - mX0Map) * mWidthSprite / (mWidthMap * pRatio);
	pY = (mHeightSprite - 1 - (pY - mY0Map) * mHeightSprite / mHeightMap) / pRatio;
}

const MR_MainCharacter *MR_ClientSession::GetPlayer(int pPlayerIndex) const
{
	const MR_MainCharacter *lReturnValue = GetMainCharacter(pPlayerIndex);
	ASSERT(lReturnValue != NULL);

	return lReturnValue;
}

void MR_ClientSession::AddMessageKey(char /*pKey */ )
{

}

void MR_ClientSession::GetCurrentMessage(char *pDest) const
{
	pDest[0] = 0;
}

BOOL MR_ClientSession::GetMessageStack(int pLevel, char *pDest, int pExpiration) const
{
	BOOL lReturnValue = FALSE;

	if(pLevel < MR_CHAT_MESSAGE_STACK) {
		EnterCriticalSection(&((MR_ClientSession *) this)->mChatMutex);

		if(((mMessageStack[pLevel].mCreationTime + pExpiration) > time(NULL)) && (mMessageStack[pLevel].mBuffer.GetLength() > 0)) {
			lReturnValue = TRUE;
			strcpy(pDest, mMessageStack[pLevel].mBuffer);
		}
		LeaveCriticalSection(&((MR_ClientSession *) this)->mChatMutex);
	}

	return lReturnValue;
}

void MR_ClientSession::AddMessage(const char *pMessage)
{
	EnterCriticalSection(&mChatMutex);

	for(int lCounter = MR_CHAT_MESSAGE_STACK - 1; lCounter > 0; lCounter--) {
		mMessageStack[lCounter] = mMessageStack[lCounter - 1];
	}

	mMessageStack[0].mCreationTime = time(NULL);

	mMessageStack[0].mBuffer = Ascii2Simple(pMessage);

	LeaveCriticalSection(&mChatMutex);

}

void MR_ClientSession::ResetRuleRuntime()
{
	delete mGameRuleRuntime;
	mGameRuleRuntime = NULL;
}

void MR_ClientSession::ResetLocalHitStats()
{
	memset(mLocalHitByOthers, 0, sizeof(mLocalHitByOthers));
	memset(mLocalGoodShots, 0, sizeof(mLocalGoodShots));
}

void MR_ClientSession::DisableAllWeapons()
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if(mMainCharacters[i] != NULL) {
			mMainCharacters[i]->SetWeaponAvailability(FALSE, mAllowCans, mAllowMines);
		}
	}
}

void MR_ClientSession::DestroyMissiles()
{
	static const MR_UInt16 TRACK_OBJECT_DLL_ID = 1;
	static const MR_UInt16 MISSILE_CLASS_ID = 150;
	MR_Level *lCurrentLevel = mSession.GetCurrentLevel();

	if(lCurrentLevel == NULL) {
		return;
	}

	for(int lRoom = MR_Level::eNonClassified; lRoom < lCurrentLevel->GetRoomCount(); lRoom++) {
		MR_FreeElementHandle lCurrent = lCurrentLevel->GetFirstFreeElement(lRoom);
		while(lCurrent != NULL) {
			MR_FreeElementHandle lNext = MR_Level::GetNextFreeElement(lCurrent);
			MR_FreeElement *lElement = MR_Level::GetFreeElement(lCurrent);
			const MR_ObjectFromFactoryId &lTypeId = lElement->GetTypeId();

			if((lTypeId.mDllId == TRACK_OBJECT_DLL_ID) &&
				(lTypeId.mClassId == MISSILE_CLASS_ID))
			{
				MR_Level::DeleteElement(lCurrent);
			}

			lCurrent = lNext;
		}
	}
}

BOOL MR_ClientSession::ShouldProcessLocalHitQueues() const
{
	return TRUE;
}

int MR_ClientSession::GetLocalPlayerIndexByHoverId(int pHoverId) const
{
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		if((mMainCharacters[i] != NULL) &&
			(mMainCharacters[i]->GetHoverId() == pHoverId))
		{
			return i;
		}
	}

	return -1;
}

void MR_ClientSession::NotifyRuleCheckpoint(int pHoverId, int pCheckpointIndex)
{
	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->OnCheckpoint(*this, pHoverId, pCheckpointIndex);
	}
}

void MR_ClientSession::NotifyRuleLapComplete(int pHoverId, int pLapNumber)
{
	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->OnLapComplete(*this, pHoverId, pLapNumber);
	}
}

void MR_ClientSession::NotifyRuleHit(int pVictimHoverId, int pSourceHoverId,
	int pElementId)
{
	const int lVictimIndex = GetLocalPlayerIndexByHoverId(pVictimHoverId);
	const int lSourceIndex = GetLocalPlayerIndexByHoverId(pSourceHoverId);

	if(lVictimIndex >= 0) {
		mLocalHitByOthers[lVictimIndex]++;
	}
	if((lSourceIndex >= 0) && (lSourceIndex != lVictimIndex)) {
		mLocalGoodShots[lSourceIndex]++;
	}

	if(mGameRuleRuntime != NULL) {
		mGameRuleRuntime->OnHit(*this, pVictimHoverId, pSourceHoverId,
			pElementId);
	}
}



