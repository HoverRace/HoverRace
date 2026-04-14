#include "StdAfx.h"

#include "GameRules.h"

#include "ClientSession.h"
#include "../MainCharacter/MainCharacter.h"

#include <math.h>
#include <sstream>

namespace {
	const float DEFAULT_REMOTE_GHOST_OPACITY = 0.90f;
	const float DEFAULT_GHOST_NEAR_OPACITY = 0.50f;
	const float DEFAULT_GHOST_NEAR_DISTANCE = 5.0f;
	const float DEFAULT_GHOST_FADE_DISTANCE = 20.0f;
	const double DISTANCE_UNITS_PER_METER = 1000.0;

	struct MR_PlayerTransparencyCurve
	{
		BOOL mEnabled;
		float mNearDistance;
		float mNearOpacity;
		float mFarDistance;
		float mFarOpacity;

		MR_PlayerTransparencyCurve() :
			mEnabled(FALSE),
			mNearDistance(DEFAULT_GHOST_NEAR_DISTANCE),
			mNearOpacity(DEFAULT_GHOST_NEAR_OPACITY),
			mFarDistance(DEFAULT_GHOST_FADE_DISTANCE),
			mFarOpacity(DEFAULT_REMOTE_GHOST_OPACITY)
		{
		}
	};

	class MR_GameRuleProgramHost;

	class MR_GameRuleSessionApi
	{
		private:
			MR_GameRuleProgramHost &mHost;
			MR_ClientSession &mSession;

		public:
			MR_GameRuleSessionApi(MR_GameRuleProgramHost &pHost,
				MR_ClientSession &pSession);

			const MR_GameRuleSettings &GetSettings() const;
			int GetPlayerCount() const;
			int GetPlayerId(int pPlayerIndex) const;
			BOOL GetPlayerState(int pHoverId, MR_GameRulePlayerState &pState) const;
			const char *GetPlayerName(int pHoverId) const;
			void SetLapCount(int pLapCount);
			void SetSpawnSlot(int pHoverId, int pSpawnSlot);
			void SetPlayerCollisions(int pHoverId, BOOL pEnabled);
			void SetSharedColumnInteraction(int pHoverId, BOOL pEnabled);
			void SetRemotePlayerTransparencyCurve(int pHoverId,
				float pNearDistance, float pNearOpacity, float pFarDistance,
				float pFarOpacity);
			void ClearRemotePlayerTransparency(int pHoverId);
			void EndMatch();
			void AddMessage(const char *pMessage);
	};

	class MR_SessionRuleProgram
	{
		public:
			virtual ~MR_SessionRuleProgram() { }
			virtual void OnInit(MR_GameRuleSessionApi &) { }
			virtual void OnPreSpawn(MR_GameRuleSessionApi &,
				MR_GameRuleSpawnContext &) { }
			virtual void OnCheckpoint(MR_GameRuleSessionApi &, int, int) { }
			virtual void OnLapComplete(MR_GameRuleSessionApi &, int, int) { }
			virtual void OnHit(MR_GameRuleSessionApi &, int, int, int) { }
			virtual void OnProcessTick(MR_GameRuleSessionApi &) { }
			virtual BOOL UsesHitScoring() const { return FALSE; }
			virtual void GetSetupOptions(MR_GameRuleSetupOptions &) const { }
			virtual BOOL IsAvailableInPractice() const { return TRUE; }
			virtual BOOL IsLapCountEditableInSetup() const { return TRUE; }
			virtual BOOL ValidateSettings(const MR_GameRuleSettings &, char *, int) const
				{ return TRUE; }
			virtual void ApplySessionOptions(BOOL &, BOOL &, BOOL &) const { }
			virtual BOOL FormatHudText(const MR_ClientSession &,
				const MR_MainCharacter *, MR_SimulationTime, const MR_GameRuleSettings &,
				char *, int, char *, int) const { return FALSE; }
	};

	class MR_NormalRaceProgram : public MR_SessionRuleProgram
	{
	};

	class MR_GhostRaceProgram : public MR_SessionRuleProgram
	{
		private:
			BOOL mFirstLapOnly;
			int mLapState[MR_NB_MAX_PLAYER];

			void ApplyGhostState(MR_GameRuleSessionApi &pApi)
			{
				const MR_GameRuleSettings &lSettings = pApi.GetSettings();
				const int lPlayerCount = pApi.GetPlayerCount();

				for(int lPlayerIndex = 0; lPlayerIndex < lPlayerCount; lPlayerIndex++) {
					const int lHoverId = pApi.GetPlayerId(lPlayerIndex);
					const BOOL lGhosted =
						!mFirstLapOnly || ((lHoverId >= 0) &&
							(lHoverId < MR_NB_MAX_PLAYER) &&
							(mLapState[lHoverId] < 1));

					pApi.SetPlayerCollisions(lHoverId, !lGhosted);
					pApi.SetSharedColumnInteraction(lHoverId, !lGhosted);
					pApi.SetRemotePlayerTransparencyCurve(lHoverId,
						lSettings.mGhostNearDistance, lSettings.mGhostNearOpacity,
						lSettings.mGhostFadeDistance,
						lSettings.mRemoteCraftOpacity);
				}
			}

		public:
			MR_GhostRaceProgram(BOOL pFirstLapOnly) :
				mFirstLapOnly(pFirstLapOnly)
			{
				memset(mLapState, 0, sizeof(mLapState));
			}

			virtual BOOL IsAvailableInPractice() const
			{
				return FALSE;
			}

			virtual void GetSetupOptions(MR_GameRuleSetupOptions &pOptions) const
			{
				pOptions.mUsesGhostTransparencySettings = TRUE;
			}

			virtual void OnPreSpawn(MR_GameRuleSessionApi &pApi,
				MR_GameRuleSpawnContext &pContext)
			{
				pApi.SetSpawnSlot(pContext.mHoverId, 0);
				pContext.mSpawnSlot = 0;
			}

			virtual void OnLapComplete(MR_GameRuleSessionApi &, int pHoverId,
				int pLapNumber)
			{
				if((pHoverId >= 0) && (pHoverId < MR_NB_MAX_PLAYER) &&
					(pLapNumber > mLapState[pHoverId]))
				{
					mLapState[pHoverId] = pLapNumber;
				}
			}

			virtual void OnProcessTick(MR_GameRuleSessionApi &pApi)
			{
				ApplyGhostState(pApi);
			}
	};

	class MR_WarProgram : public MR_SessionRuleProgram
	{
		private:
			int mScores[MR_NB_MAX_PLAYER];
			BOOL mMatchFinished;
			int GetPlacement(const MR_ClientSession &pSession, int pHoverId) const
			{
				int lPlacement = 1;
				MR_GameRulePlayerState lPlayerState;
				int lPlayerHitsAgainst = 0;

				if(pSession.GetGameRulePlayerState(pHoverId, lPlayerState)) {
					lPlayerHitsAgainst = lPlayerState.mHitByOtherCount;
				}

				for(int lPlayerIndex = 0; lPlayerIndex < pSession.GetNbPlayers();
					lPlayerIndex++)
				{
					const MR_MainCharacter *lOtherPlayer =
						pSession.GetPlayer(lPlayerIndex);

					if(lOtherPlayer == NULL) {
						continue;
					}

					const int lOtherHoverId = lOtherPlayer->GetHoverId();
					if((lOtherHoverId < 0) || (lOtherHoverId >= MR_NB_MAX_PLAYER) ||
						(lOtherHoverId == pHoverId))
					{
						continue;
					}

					if(mScores[lOtherHoverId] > mScores[pHoverId]) {
						lPlacement++;
					}
					else if(mScores[lOtherHoverId] == mScores[pHoverId]) {
						MR_GameRulePlayerState lOtherState;
						int lOtherHitsAgainst = 0;

						if(pSession.GetGameRulePlayerState(lOtherHoverId, lOtherState)) {
							lOtherHitsAgainst = lOtherState.mHitByOtherCount;
						}

						if((lOtherHitsAgainst < lPlayerHitsAgainst) ||
							((lOtherHitsAgainst == lPlayerHitsAgainst) &&
								(lOtherHoverId < pHoverId)))
						{
							lPlacement++;
						}
					}
				}

				return lPlacement;
			}

			void BroadcastScoreMessage(MR_GameRuleSessionApi &pApi)
			{
				// War now reports the scoring event directly from the rule.
			}

			BOOL CheckForWinner(MR_GameRuleSessionApi &pApi, int pHoverId)
			{
				int lBestOtherScore = 0;
				const int lNbPlayers = pApi.GetPlayerCount();

				for(int lPlayer = 0; lPlayer < lNbPlayers; lPlayer++) {
					const int lOtherHoverId = pApi.GetPlayerId(lPlayer);
					if((lOtherHoverId < 0) || (lOtherHoverId >= MR_NB_MAX_PLAYER) ||
						(lOtherHoverId == pHoverId))
					{
						continue;
					}

					lBestOtherScore = max(lBestOtherScore, mScores[lOtherHoverId]);
				}

				return (mScores[pHoverId] >= pApi.GetSettings().mWarTargetScore) &&
					((mScores[pHoverId] - lBestOtherScore) >=
						pApi.GetSettings().mWarWinBy);
			}

		public:
			MR_WarProgram() :
				mMatchFinished(FALSE)
			{
				memset(mScores, 0, sizeof(mScores));
			}

			virtual BOOL UsesHitScoring() const
			{
				return TRUE;
			}

			virtual BOOL IsAvailableInPractice() const
			{
				return FALSE;
			}

			virtual BOOL IsLapCountEditableInSetup() const
			{
				return FALSE;
			}

			virtual void GetSetupOptions(MR_GameRuleSetupOptions &pOptions) const
			{
				pOptions.mLapCountEditable = FALSE;
				pOptions.mWeaponsEditable = FALSE;
				pOptions.mWeaponsForcedOn = TRUE;
				pOptions.mUsesWarScoreSettings = TRUE;
			}

			virtual BOOL ValidateSettings(const MR_GameRuleSettings &pSettings,
				char *pErrorBuffer, int pErrorBufferLen) const
			{
				if(pSettings.mWarTargetScore < 1) {
					if((pErrorBuffer != NULL) && (pErrorBufferLen > 0)) {
						strncpy(pErrorBuffer, "War target score must be at least 1.",
							pErrorBufferLen - 1);
						pErrorBuffer[pErrorBufferLen - 1] = '\0';
					}
					return FALSE;
				}
				if(pSettings.mWarWinBy < 1) {
					if((pErrorBuffer != NULL) && (pErrorBufferLen > 0)) {
						strncpy(pErrorBuffer, "War win-by must be at least 1.",
							pErrorBufferLen - 1);
						pErrorBuffer[pErrorBufferLen - 1] = '\0';
					}
					return FALSE;
				}
				return TRUE;
			}

			virtual void ApplySessionOptions(BOOL &pAllowWeapons, BOOL &, BOOL &) const
			{
				pAllowWeapons = TRUE;
			}

			virtual void OnHit(MR_GameRuleSessionApi &pApi, int pVictimHoverId,
				int pSourceHoverId, int)
			{
				if(mMatchFinished || (pSourceHoverId < 0) ||
					(pSourceHoverId >= MR_NB_MAX_PLAYER))
				{
					return;
				}
				if(pVictimHoverId == pSourceHoverId) {
					return;
				}

				mScores[pSourceHoverId]++;
				{
					char lBuffer[160];

					sprintf(lBuffer, "%s has hit %s (Total: %d)",
						pApi.GetPlayerName(pSourceHoverId),
						pApi.GetPlayerName(pVictimHoverId),
						mScores[pSourceHoverId]);
					pApi.AddMessage(lBuffer);
				}

				if(CheckForWinner(pApi, pSourceHoverId)) {
					char lBuffer[96];

					sprintf(lBuffer, "War winner: %s",
						pApi.GetPlayerName(pSourceHoverId));
					pApi.AddMessage(lBuffer);
					pApi.EndMatch();
					mMatchFinished = TRUE;
				}
			}

			virtual BOOL FormatHudText(const MR_ClientSession &pSession,
				const MR_MainCharacter *pViewingCharacter, MR_SimulationTime pTime,
				const MR_GameRuleSettings &pSettings, char *pMainBuffer,
				int pMainBufferLen, char *pSecondaryBuffer,
				int pSecondaryBufferLen) const
			{
				if((pViewingCharacter == NULL) || (pMainBuffer == NULL) ||
					(pSecondaryBuffer == NULL) || (pMainBufferLen <= 0) ||
					(pSecondaryBufferLen <= 0))
				{
					return FALSE;
				}

				MR_GameRulePlayerState lRuleState;
				int lHitsFor = 0;
				int lHitsAgainst = 0;

				if(pSession.GetGameRulePlayerState(pViewingCharacter->GetHoverId(),
					lRuleState))
				{
					lHitsFor = lRuleState.mHitOtherCount;
					lHitsAgainst = lRuleState.mHitByOtherCount;
				}

				if(pTime < 0) {
					pTime = -pTime;
					sprintf(pMainBuffer,
						"Starting in %02d.%02d sec. first to %d win by %d",
						(pTime % 60000) / 1000, (pTime % 1000) / 10,
						pSettings.mWarTargetScore, pSettings.mWarWinBy);
				}
				else if(pSession.IsRuleBasedMatchFinished()) {
					const int lPlacement =
						GetPlacement(pSession, pViewingCharacter->GetHoverId());

					sprintf(pMainBuffer, "Placed %d of %d",
						max(1, lPlacement),
						max(1, pSession.GetNbPlayers()));
				}
				else {
					sprintf(pMainBuffer, "War first to %d, win by %d",
						pSettings.mWarTargetScore, pSettings.mWarWinBy);
				}

				sprintf(pSecondaryBuffer, "Hits %d  Hit by %d",
					lHitsFor, lHitsAgainst);
				return TRUE;
			}
		};

	class MR_LocalViewRuleAdapter
	{
		private:
			MR_PlayerTransparencyCurve mTransparencyByHover[MR_NB_MAX_PLAYER];

			float ClampOpacity(float pOpacity) const
			{
				if(pOpacity < 0.01f) {
					return 0.01f;
				}
				if(pOpacity > 1.0f) {
					return 1.0f;
				}
				return pOpacity;
			}

			float ComputeDistanceOpacity(const MR_MainCharacter *pViewingCharacter,
				const MR_MainCharacter *pTargetCharacter,
				const MR_PlayerTransparencyCurve &pCurve,
				float pDefaultOpacity) const
			{
				double lDeltaX = static_cast<double>(pTargetCharacter->mPosition.mX -
					pViewingCharacter->mPosition.mX);
				double lDeltaY = static_cast<double>(pTargetCharacter->mPosition.mY -
					pViewingCharacter->mPosition.mY);
				double lDeltaZ = static_cast<double>(pTargetCharacter->mPosition.mZ -
					pViewingCharacter->mPosition.mZ);
				double lDistance = sqrt((lDeltaX * lDeltaX) + (lDeltaY * lDeltaY) +
					(lDeltaZ * lDeltaZ));
				const double lNearDistance =
					pCurve.mNearDistance * DISTANCE_UNITS_PER_METER;
				const double lFarDistance =
					pCurve.mFarDistance * DISTANCE_UNITS_PER_METER;
				const double lFadeDistance = lFarDistance - lNearDistance;
				double lFade = (lFadeDistance <= 0.0) ? 1.0 :
					((lDistance - lNearDistance) / lFadeDistance);
				float lFarOpacity = pCurve.mFarOpacity;
				float lNearOpacity = pCurve.mNearOpacity;

				if(pDefaultOpacity < 1.0f) {
					lFarOpacity = min(lFarOpacity, pDefaultOpacity);
					lNearOpacity = min(lNearOpacity, pDefaultOpacity);
				}

				if(lFade < 0.0) {
					lFade = 0.0;
				}
				else if(lFade > 1.0) {
					lFade = 1.0;
				}

				return ClampOpacity(static_cast<float>(lNearOpacity +
					(lFade * (lFarOpacity - lNearOpacity))));
			}

		public:
			void SetRemotePlayerTransparencyCurve(int pHoverId,
				float pNearDistance, float pNearOpacity, float pFarDistance,
				float pFarOpacity)
			{
				if((pHoverId < 0) || (pHoverId >= MR_NB_MAX_PLAYER)) {
					return;
				}

				mTransparencyByHover[pHoverId].mEnabled = TRUE;
				mTransparencyByHover[pHoverId].mNearDistance = pNearDistance;
				mTransparencyByHover[pHoverId].mNearOpacity = pNearOpacity;
				mTransparencyByHover[pHoverId].mFarDistance = pFarDistance;
				mTransparencyByHover[pHoverId].mFarOpacity = pFarOpacity;
			}

			void ClearRemotePlayerTransparency(int pHoverId)
			{
				if((pHoverId < 0) || (pHoverId >= MR_NB_MAX_PLAYER)) {
					return;
				}

				mTransparencyByHover[pHoverId] = MR_PlayerTransparencyCurve();
			}

			float GetRemotePlayerOpacityForView(const MR_ClientSession &pSession,
				const MR_GameRuleProgramHost &pHost,
				const MR_MainCharacter *pViewingCharacter,
				const MR_MainCharacter *pTargetCharacter,
				float pDefaultOpacity) const;
	};

	class MR_GameRuleProgramHost : public MR_GameRuleRuntime
	{
		private:
			MR_SessionRuleProgram *mSessionProgram;
			MR_LocalViewRuleAdapter mLocalViewAdapter;
			int mSpawnSlotOverride[MR_NB_MAX_PLAYER];
			BOOL mCraftCollisionOverrideTracked[MR_NB_MAX_PLAYER];
			BOOL mCraftCollisionOverrideEnabled[MR_NB_MAX_PLAYER];
			BOOL mColumnInteractionOverrideTracked[MR_NB_MAX_PLAYER];
			BOOL mColumnInteractionOverrideEnabled[MR_NB_MAX_PLAYER];

			void ApplyPersistentPlayerOverrides(MR_ClientSession &pSession)
			{
				const int lPlayerCount = pSession.GetNbPlayers();

				for(int lPlayerIndex = 0; lPlayerIndex < lPlayerCount; lPlayerIndex++) {
					const MR_MainCharacter *lPlayer = pSession.GetPlayer(lPlayerIndex);

					if(lPlayer == NULL) {
						continue;
					}

					const int lHoverId = lPlayer->GetHoverId();
					if((lHoverId < 0) || (lHoverId >= MR_NB_MAX_PLAYER)) {
						continue;
					}

					if(mCraftCollisionOverrideTracked[lHoverId]) {
						pSession.SetPlayerCraftCollision(lHoverId,
							mCraftCollisionOverrideEnabled[lHoverId]);
					}
					if(mColumnInteractionOverrideTracked[lHoverId]) {
						pSession.SetPlayerColumnInteraction(lHoverId,
							mColumnInteractionOverrideEnabled[lHoverId]);
					}
				}
			}

		public:
			MR_GameRuleProgramHost(const MR_GameRuleSettings &pSettings,
				MR_SessionRuleProgram *pSessionProgram,
				MR_GameRuleClassification pClassification) :
				MR_GameRuleRuntime(pSettings),
				mSessionProgram(pSessionProgram)
			{
				mClassification = pClassification;
				for(int lCounter = 0; lCounter < MR_NB_MAX_PLAYER; lCounter++) {
					mSpawnSlotOverride[lCounter] = -1;
					mCraftCollisionOverrideTracked[lCounter] = FALSE;
					mCraftCollisionOverrideEnabled[lCounter] = TRUE;
					mColumnInteractionOverrideTracked[lCounter] = FALSE;
					mColumnInteractionOverrideEnabled[lCounter] = TRUE;
				}
			}

			virtual ~MR_GameRuleProgramHost()
			{
				delete mSessionProgram;
			}

			int GetPlayerCount(const MR_ClientSession &pSession) const
			{
				return pSession.GetNbPlayers();
			}

			int GetPlayerId(const MR_ClientSession &pSession, int pPlayerIndex) const
			{
				const MR_MainCharacter *lPlayer = pSession.GetPlayer(pPlayerIndex);
				return (lPlayer != NULL) ? lPlayer->GetHoverId() : -1;
			}

			void SetSpawnSlot(int pHoverId, int pSpawnSlot)
			{
				if((pHoverId < 0) || (pHoverId >= MR_NB_MAX_PLAYER)) {
					return;
				}

				mSpawnSlotOverride[pHoverId] = max(0, pSpawnSlot);
			}

			void SetPlayerCollisions(MR_ClientSession &pSession, int pHoverId,
				BOOL pEnabled)
			{
				if((pHoverId < 0) || (pHoverId >= MR_NB_MAX_PLAYER)) {
					return;
				}

				mCraftCollisionOverrideTracked[pHoverId] = TRUE;
				mCraftCollisionOverrideEnabled[pHoverId] = pEnabled;
				pSession.SetPlayerCraftCollision(pHoverId, pEnabled);
			}

			void SetSharedColumnInteraction(MR_ClientSession &pSession,
				int pHoverId, BOOL pEnabled)
			{
				if((pHoverId < 0) || (pHoverId >= MR_NB_MAX_PLAYER)) {
					return;
				}

				mColumnInteractionOverrideTracked[pHoverId] = TRUE;
				mColumnInteractionOverrideEnabled[pHoverId] = pEnabled;
				pSession.SetPlayerColumnInteraction(pHoverId, pEnabled);
			}

			BOOL GetEffectiveCraftCollisionEnabled(const MR_ClientSession &pSession,
				int pHoverId) const
			{
				const MR_MainCharacter *lPlayer = pSession.FindPlayerByHoverId(pHoverId);

				if((pHoverId >= 0) && (pHoverId < MR_NB_MAX_PLAYER) &&
					mCraftCollisionOverrideTracked[pHoverId])
				{
					return mCraftCollisionOverrideEnabled[pHoverId];
				}

				return (lPlayer != NULL) ? lPlayer->GetCraftCollisionEnabled() : TRUE;
			}

			virtual void OnInit(MR_ClientSession &pSession)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnInit(lApi);
				}
				ApplyPersistentPlayerOverrides(pSession);
			}

			virtual void OnPreSpawn(MR_ClientSession &pSession,
				MR_GameRuleSpawnContext &pContext)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnPreSpawn(lApi, pContext);
				}

				if((pContext.mHoverId >= 0) && (pContext.mHoverId < MR_NB_MAX_PLAYER) &&
					(mSpawnSlotOverride[pContext.mHoverId] >= 0))
				{
					pContext.mSpawnSlot = mSpawnSlotOverride[pContext.mHoverId];
				}
			}

			virtual void OnCheckpoint(MR_ClientSession &pSession, int pHoverId,
				int pCheckpointIndex)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnCheckpoint(lApi, pHoverId, pCheckpointIndex);
				}
				ApplyPersistentPlayerOverrides(pSession);
			}

			virtual void OnLapComplete(MR_ClientSession &pSession, int pHoverId,
				int pLapNumber)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnLapComplete(lApi, pHoverId, pLapNumber);
				}
				ApplyPersistentPlayerOverrides(pSession);
			}

			virtual void OnHit(MR_ClientSession &pSession, int pVictimHoverId,
				int pSourceHoverId, int pElementId)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnHit(lApi, pVictimHoverId, pSourceHoverId,
						pElementId);
				}
				ApplyPersistentPlayerOverrides(pSession);
			}

			virtual void OnProcessTick(MR_ClientSession &pSession)
			{
				MR_GameRuleSessionApi lApi(*this, pSession);

				if(mSessionProgram != NULL) {
					mSessionProgram->OnProcessTick(lApi);
				}
				ApplyPersistentPlayerOverrides(pSession);
			}

			virtual BOOL UsesHitScoring() const
			{
				return (mSessionProgram != NULL) &&
					mSessionProgram->UsesHitScoring();
			}

			virtual void GetSetupOptions(MR_GameRuleSetupOptions &pOptions) const
			{
				if(mSessionProgram != NULL) {
					mSessionProgram->GetSetupOptions(pOptions);
				}
			}

			virtual BOOL IsAvailableInPractice() const
			{
				return (mSessionProgram == NULL) ||
					mSessionProgram->IsAvailableInPractice();
			}

			virtual BOOL IsLapCountEditableInSetup() const
			{
				return (mSessionProgram == NULL) ||
					mSessionProgram->IsLapCountEditableInSetup();
			}

			virtual BOOL ValidateSettings(char *pErrorBuffer, int pErrorBufferLen) const
			{
				return (mSessionProgram == NULL) ||
					mSessionProgram->ValidateSettings(mSettings, pErrorBuffer,
						pErrorBufferLen);
			}

			virtual void ApplySessionOptions(BOOL &pAllowWeapons, BOOL &pAllowCans, BOOL &pAllowMines) const
			{
				if(mSessionProgram != NULL) {
					mSessionProgram->ApplySessionOptions(pAllowWeapons, pAllowCans, pAllowMines);
				}
			}

			virtual BOOL TryGetPlayerState(const MR_ClientSession &pSession,
				int pHoverId, MR_GameRulePlayerState &pState) const
			{
				return pSession.GetGameRulePlayerState(pHoverId, pState);
			}

			virtual float GetRemotePlayerOpacityForView(
				const MR_ClientSession &pSession,
				const MR_MainCharacter *pViewingCharacter,
				const MR_MainCharacter *pTargetCharacter,
				float pDefaultOpacity) const
			{
				return mLocalViewAdapter.GetRemotePlayerOpacityForView(pSession, *this,
					pViewingCharacter, pTargetCharacter, pDefaultOpacity);
			}

			virtual BOOL FormatHudText(const MR_ClientSession &pSession,
				const MR_MainCharacter *pViewingCharacter,
				MR_SimulationTime pTime, char *pMainBuffer, int pMainBufferLen,
				char *pSecondaryBuffer, int pSecondaryBufferLen) const
			{
				return (mSessionProgram != NULL) &&
					mSessionProgram->FormatHudText(pSession, pViewingCharacter, pTime,
						mSettings, pMainBuffer, pMainBufferLen, pSecondaryBuffer,
						pSecondaryBufferLen);
			}

			void SetRemotePlayerTransparencyCurve(int pHoverId,
				float pNearDistance, float pNearOpacity, float pFarDistance,
				float pFarOpacity)
			{
				mLocalViewAdapter.SetRemotePlayerTransparencyCurve(pHoverId,
					pNearDistance, pNearOpacity, pFarDistance, pFarOpacity);
			}

			void ClearRemotePlayerTransparency(int pHoverId)
			{
				mLocalViewAdapter.ClearRemotePlayerTransparency(pHoverId);
			}
	};

	MR_GameRuleSessionApi::MR_GameRuleSessionApi(MR_GameRuleProgramHost &pHost,
		MR_ClientSession &pSession) :
		mHost(pHost), mSession(pSession)
	{
	}

	const MR_GameRuleSettings &MR_GameRuleSessionApi::GetSettings() const
	{
		return mHost.GetSettings();
	}

	int MR_GameRuleSessionApi::GetPlayerCount() const
	{
		return mHost.GetPlayerCount(mSession);
	}

	int MR_GameRuleSessionApi::GetPlayerId(int pPlayerIndex) const
	{
		return mHost.GetPlayerId(mSession, pPlayerIndex);
	}

	BOOL MR_GameRuleSessionApi::GetPlayerState(int pHoverId,
		MR_GameRulePlayerState &pState) const
	{
		return mHost.TryGetPlayerState(mSession, pHoverId, pState);
	}

	const char *MR_GameRuleSessionApi::GetPlayerName(int pHoverId) const
	{
		return mSession.GetPlayerDisplayName(pHoverId);
	}

	void MR_GameRuleSessionApi::SetLapCount(int pLapCount)
	{
		mSession.SetLapCount(pLapCount);
	}

	void MR_GameRuleSessionApi::SetSpawnSlot(int pHoverId, int pSpawnSlot)
	{
		mHost.SetSpawnSlot(pHoverId, pSpawnSlot);
	}

	void MR_GameRuleSessionApi::SetPlayerCollisions(int pHoverId, BOOL pEnabled)
	{
		mHost.SetPlayerCollisions(mSession, pHoverId, pEnabled);
	}

	void MR_GameRuleSessionApi::SetSharedColumnInteraction(int pHoverId,
		BOOL pEnabled)
	{
		mHost.SetSharedColumnInteraction(mSession, pHoverId, pEnabled);
	}

	void MR_GameRuleSessionApi::SetRemotePlayerTransparencyCurve(int pHoverId,
		float pNearDistance, float pNearOpacity, float pFarDistance,
		float pFarOpacity)
	{
		mHost.SetRemotePlayerTransparencyCurve(pHoverId, pNearDistance,
			pNearOpacity, pFarDistance, pFarOpacity);
	}

	void MR_GameRuleSessionApi::ClearRemotePlayerTransparency(int pHoverId)
	{
		mHost.ClearRemotePlayerTransparency(pHoverId);
	}

	void MR_GameRuleSessionApi::EndMatch()
	{
		mSession.EndRuleBasedMatch();
	}

	void MR_GameRuleSessionApi::AddMessage(const char *pMessage)
	{
		mSession.AddMessage(pMessage);
	}

	float MR_LocalViewRuleAdapter::GetRemotePlayerOpacityForView(
		const MR_ClientSession &pSession, const MR_GameRuleProgramHost &pHost,
		const MR_MainCharacter *pViewingCharacter,
		const MR_MainCharacter *pTargetCharacter,
		float pDefaultOpacity) const
	{
		const int lViewingHoverId =
			(pViewingCharacter != NULL) ? pViewingCharacter->GetHoverId() : -1;
		const int lTargetHoverId =
			(pTargetCharacter != NULL) ? pTargetCharacter->GetHoverId() : -1;
		const BOOL lHasCurve = (lTargetHoverId >= 0) &&
			(lTargetHoverId < MR_NB_MAX_PLAYER) &&
			mTransparencyByHover[lTargetHoverId].mEnabled;

		if((pViewingCharacter == NULL) || (pTargetCharacter == NULL)) {
			return ClampOpacity(pDefaultOpacity);
		}
		if(lViewingHoverId == lTargetHoverId) {
			return 1.0f;
		}
		if(!lHasCurve) {
			return ClampOpacity(pDefaultOpacity);
		}

		if(!pHost.GetEffectiveCraftCollisionEnabled(pSession, lViewingHoverId) ||
			!pHost.GetEffectiveCraftCollisionEnabled(pSession, lTargetHoverId) ||
			(pDefaultOpacity < 1.0f))
		{
			return ComputeDistanceOpacity(pViewingCharacter, pTargetCharacter,
				mTransparencyByHover[lTargetHoverId], pDefaultOpacity);
		}

		return ClampOpacity(pDefaultOpacity);
	}

	std::string TrimRightSpaces(const std::string &pValue)
	{
		std::string lReturnValue = pValue;

		while(!lReturnValue.empty() &&
			(lReturnValue[lReturnValue.length() - 1] == ' '))
		{
			lReturnValue.erase(lReturnValue.length() - 1);
		}

		return lReturnValue;
	}
}

MR_GameRuleSettings::MR_GameRuleSettings()
{
	mModeId = MR_GR_NORMAL_RACE;
	mWarTargetScore = 10;
	mWarWinBy = 2;
	mRemoteCraftOpacity = DEFAULT_REMOTE_GHOST_OPACITY;
	mGhostNearOpacity = DEFAULT_GHOST_NEAR_OPACITY;
	mGhostNearDistance = DEFAULT_GHOST_NEAR_DISTANCE;
	mGhostFadeDistance = DEFAULT_GHOST_FADE_DISTANCE;
}

MR_GameRuleSetupOptions::MR_GameRuleSetupOptions()
{
	mAvailableInPractice = TRUE;
	mLapCountEditable = TRUE;
	mWeaponsEditable = TRUE;
	mWeaponsForcedOn = FALSE;
	mUsesGhostTransparencySettings = FALSE;
	mUsesWarScoreSettings = FALSE;
}

MR_GameRulePlayerState::MR_GameRulePlayerState()
{
	mHoverId = -1;
	mLapNumber = 0;
	mCheckpointIndex = 0;
	mHitOtherCount = 0;
	mHitByOtherCount = 0;
	mHasFinished = FALSE;
	mCraftCollisionEnabled = TRUE;
	mColumnInteractionEnabled = TRUE;
	mBaseOpacity = 1.0f;
}

MR_GameRuleRuntime::MR_GameRuleRuntime(const MR_GameRuleSettings &pSettings) :
	mSettings(pSettings),
	mClassification(MR_GRC_STANDARD)
{
}

MR_GameRuleRuntime::~MR_GameRuleRuntime()
{
}

void MR_GameRuleRuntime::OnInit(MR_ClientSession &)
{
}

void MR_GameRuleRuntime::OnPreSpawn(MR_ClientSession &,
	MR_GameRuleSpawnContext &)
{
}

void MR_GameRuleRuntime::OnCheckpoint(MR_ClientSession &, int, int)
{
}

void MR_GameRuleRuntime::OnLapComplete(MR_ClientSession &, int, int)
{
}

void MR_GameRuleRuntime::OnHit(MR_ClientSession &, int, int, int)
{
}

void MR_GameRuleRuntime::OnProcessTick(MR_ClientSession &)
{
}

BOOL MR_GameRuleRuntime::UsesHitScoring() const
{
	return FALSE;
}

void MR_GameRuleRuntime::GetSetupOptions(MR_GameRuleSetupOptions &pOptions) const
{
	pOptions.mAvailableInPractice = IsAvailableInPractice();
	pOptions.mLapCountEditable = IsLapCountEditableInSetup();
}

BOOL MR_GameRuleRuntime::IsAvailableInPractice() const
{
	return TRUE;
}

BOOL MR_GameRuleRuntime::IsLapCountEditableInSetup() const
{
	return TRUE;
}

BOOL MR_GameRuleRuntime::ValidateSettings(char *, int) const
{
	return TRUE;
}

void MR_GameRuleRuntime::ApplySessionOptions(BOOL &, BOOL &, BOOL &) const
{
}

BOOL MR_GameRuleRuntime::TryGetPlayerState(const MR_ClientSession &pSession,
	int pHoverId, MR_GameRulePlayerState &pState) const
{
	return pSession.GetGameRulePlayerState(pHoverId, pState);
}

float MR_GameRuleRuntime::GetRemotePlayerOpacityForView(
	const MR_ClientSession &, const MR_MainCharacter *pViewingCharacter,
	const MR_MainCharacter *pTargetCharacter, float pDefaultOpacity) const
{
	if((pViewingCharacter != NULL) && (pTargetCharacter != NULL) &&
		(pViewingCharacter->GetHoverId() == pTargetCharacter->GetHoverId()))
	{
		return 1.0f;
	}

	if(pDefaultOpacity < 0.01f) {
		return 0.01f;
	}
	if(pDefaultOpacity > 1.0f) {
		return 1.0f;
	}
	return pDefaultOpacity;
}

BOOL MR_GameRuleRuntime::FormatHudText(const MR_ClientSession &,
	const MR_MainCharacter *, MR_SimulationTime, char *, int, char *, int) const
{
	return FALSE;
}

const MR_GameRuleSettings &MR_GameRuleRuntime::GetSettings() const
{
	return mSettings;
}

MR_GameRuleClassification MR_GameRuleRuntime::GetClassification() const
{
	return mClassification;
}

BOOL MR_GameRuleRuntime::IsStandardRule() const
{
	return mClassification == MR_GRC_STANDARD;
}

void MR_GetGameRuleSetupOptions(const MR_GameRuleSettings &pSettings,
	MR_GameRuleSetupOptions &pOptions)
{
	MR_GameRuleRuntime *lRuntime = MR_GameRuleRuntime::Create(pSettings);
	pOptions = MR_GameRuleSetupOptions();

	if(lRuntime != NULL) {
		lRuntime->GetSetupOptions(pOptions);
		delete lRuntime;
	}
}

BOOL MR_IsLapCountEditableInSetup(const MR_GameRuleSettings &pSettings)
{
	BOOL lEditable = TRUE;
	MR_GameRuleRuntime *lRuntime = MR_GameRuleRuntime::Create(pSettings);

	if(lRuntime != NULL) {
		lEditable = lRuntime->IsLapCountEditableInSetup();
		delete lRuntime;
	}

	return lEditable;
}

BOOL MR_IsGameRuleAvailableInPractice(const MR_GameRuleSettings &pSettings)
{
	BOOL lAvailable = TRUE;
	MR_GameRuleRuntime *lRuntime = MR_GameRuleRuntime::Create(pSettings);

	if(lRuntime != NULL) {
		lAvailable = lRuntime->IsAvailableInPractice();
		delete lRuntime;
	}

	return lAvailable;
}

BOOL MR_ValidateGameRuleSettings(const MR_GameRuleSettings &pSettings,
	char *pErrorBuffer, int pErrorBufferLen)
{
	BOOL lValid = TRUE;
	MR_GameRuleRuntime *lRuntime = MR_GameRuleRuntime::Create(pSettings);

	if((pErrorBuffer != NULL) && (pErrorBufferLen > 0)) {
		pErrorBuffer[0] = '\0';
	}

	if(lRuntime != NULL) {
		lValid = lRuntime->ValidateSettings(pErrorBuffer, pErrorBufferLen);
		delete lRuntime;
	}

	return lValid;
}

MR_GameRuleRuntime *MR_GameRuleRuntime::Create(
	const MR_GameRuleSettings &pSettings)
{
	MR_GameRuleSettings lSettings = pSettings;
	MR_NormalizeGameRuleSettings(lSettings);

	switch(lSettings.mModeId) {
		case MR_GR_NO_COLLISION_RACE:
			return new MR_GameRuleProgramHost(lSettings,
				new MR_GhostRaceProgram(FALSE), MR_GRC_STANDARD);

		case MR_GR_FIRST_LAP_GHOST_RACE:
			return new MR_GameRuleProgramHost(lSettings,
				new MR_GhostRaceProgram(TRUE), MR_GRC_STANDARD);

		case MR_GR_WAR:
			return new MR_GameRuleProgramHost(lSettings,
				new MR_WarProgram(), MR_GRC_STANDARD);

		case MR_GR_NORMAL_RACE:
		default:
			return new MR_GameRuleProgramHost(lSettings,
				new MR_NormalRaceProgram(), MR_GRC_STANDARD);
	}
}

void MR_NormalizeGameRuleSettings(MR_GameRuleSettings &pSettings)
{
	if((pSettings.mModeId < MR_GR_NORMAL_RACE) ||
		(pSettings.mModeId > MR_GR_WAR))
	{
		pSettings.mModeId = MR_GR_NORMAL_RACE;
	}

	if(pSettings.mWarTargetScore < 1) {
		pSettings.mWarTargetScore = 1;
	}
	if(pSettings.mWarWinBy < 1) {
		pSettings.mWarWinBy = 1;
	}
	if(pSettings.mRemoteCraftOpacity < 0.05f) {
		pSettings.mRemoteCraftOpacity = 0.05f;
	}
	else if(pSettings.mRemoteCraftOpacity > 1.0f) {
		pSettings.mRemoteCraftOpacity = 1.0f;
	}
	if(pSettings.mGhostNearOpacity < 0.01f) {
		pSettings.mGhostNearOpacity = 0.01f;
	}
	else if(pSettings.mGhostNearOpacity > 1.0f) {
		pSettings.mGhostNearOpacity = 1.0f;
	}
	if(pSettings.mGhostNearDistance < 0.0f) {
		pSettings.mGhostNearDistance = DEFAULT_GHOST_NEAR_DISTANCE;
	}
	if(pSettings.mGhostFadeDistance < 1.0f) {
		pSettings.mGhostFadeDistance = DEFAULT_GHOST_FADE_DISTANCE;
	}
	if(pSettings.mGhostFadeDistance <= pSettings.mGhostNearDistance) {
		pSettings.mGhostFadeDistance = pSettings.mGhostNearDistance + 1.0f;
	}
}

const char *MR_GetGameRuleToken(MR_GameRuleId pModeId)
{
	switch(pModeId) {
		case MR_GR_NO_COLLISION_RACE:
			return "no_collision_race";

		case MR_GR_FIRST_LAP_GHOST_RACE:
			return "first_lap_ghost_race";

		case MR_GR_WAR:
			return "war";

		case MR_GR_NORMAL_RACE:
		default:
			return "normal_race";
	}
}

const char *MR_GetGameRuleDisplayName(MR_GameRuleId pModeId)
{
	switch(pModeId) {
		case MR_GR_NO_COLLISION_RACE:
			return "No-Collision Race";

		case MR_GR_FIRST_LAP_GHOST_RACE:
			return "First-Lap Ghost Race";

		case MR_GR_WAR:
			return "War";

		case MR_GR_NORMAL_RACE:
		default:
			return "Normal Race";
	}
}

std::string MR_FormatGameRuleConfigSummary(const MR_GameRuleSettings &pSettings,
	int pLapCount)
{
	MR_GameRuleSettings lSettings = pSettings;
	CString lSummary;

	MR_NormalizeGameRuleSettings(lSettings);

	if(pLapCount < 1) {
		pLapCount = 1;
	}

	switch(lSettings.mModeId) {
		case MR_GR_NO_COLLISION_RACE:
			lSummary.Format("Ghost Race, %d %s", pLapCount,
				pLapCount == 1 ? "lap" : "laps");
			break;

		case MR_GR_FIRST_LAP_GHOST_RACE:
			lSummary.Format("First-Lap Ghost Race, %d %s", pLapCount,
				pLapCount == 1 ? "lap" : "laps");
			break;

		case MR_GR_WAR:
			lSummary.Format("War, first to %d, win by %d",
				lSettings.mWarTargetScore, lSettings.mWarWinBy);
			break;

		case MR_GR_NORMAL_RACE:
		default:
			lSummary.Format("Race, %d %s", pLapCount,
				pLapCount == 1 ? "lap" : "laps");
			break;
	}

	return (const char *) lSummary;
}

std::string MR_FormatGameRuleSummary(const MR_GameRuleSettings &pSettings)
{
	MR_GameRuleSettings lSettings = pSettings;
	std::ostringstream lOutput;

	MR_NormalizeGameRuleSettings(lSettings);

	lOutput << " mode " << MR_GetGameRuleToken(lSettings.mModeId);

	if(lSettings.mModeId == MR_GR_WAR) {
		lOutput << " war " << lSettings.mWarTargetScore << " "
			<< lSettings.mWarWinBy;
	}

	lOutput << " nearopacity "
		<< (int) (lSettings.mGhostNearOpacity * 100.0f + 0.5f);
	lOutput << " neardist "
		<< (int) (lSettings.mGhostNearDistance + 0.5f);
	lOutput << " fadedist "
		<< (int) (lSettings.mGhostFadeDistance + 0.5f);
	lOutput << " opacity "
		<< (int) (lSettings.mRemoteCraftOpacity * 100.0f + 0.5f);

	return lOutput.str();
}

std::string MR_FormatGameRulePayload(const MR_GameRuleSettings &pSettings)
{
	return MR_FormatGameRuleSummary(pSettings);
}

bool MR_ParseGameRuleSummary(std::string &pSummary,
	MR_GameRuleSettings &pSettings)
{
	const std::string lOpacityTag = " opacity ";
	const std::string lNearOpacityTag = " nearopacity ";
	const std::string lNearDistanceTag = " neardist ";
	const std::string lFadeDistanceTag = " fadedist ";
	const std::string lWarTag = " war ";
	const std::string lModeTag = " mode ";
	bool lParsed = false;

	MR_NormalizeGameRuleSettings(pSettings);
	pSummary = TrimRightSpaces(pSummary);

	const std::string::size_type lOpacityPos = pSummary.rfind(lOpacityTag);
	if((lOpacityPos != std::string::npos) &&
		(lOpacityPos + lOpacityTag.length() < pSummary.length()))
	{
		int lOpacityPercent = atoi(pSummary.c_str() + lOpacityPos +
			lOpacityTag.length());
		pSettings.mRemoteCraftOpacity = lOpacityPercent / 100.0f;
		pSummary.erase(lOpacityPos);
		pSummary = TrimRightSpaces(pSummary);
		lParsed = true;
	}

	const std::string::size_type lFadeDistancePos =
		pSummary.rfind(lFadeDistanceTag);
	if((lFadeDistancePos != std::string::npos) &&
		(lFadeDistancePos + lFadeDistanceTag.length() < pSummary.length()))
	{
		pSettings.mGhostFadeDistance = (float) atoi(pSummary.c_str() +
			lFadeDistancePos + lFadeDistanceTag.length());
		pSummary.erase(lFadeDistancePos);
		pSummary = TrimRightSpaces(pSummary);
		lParsed = true;
	}

	const std::string::size_type lNearDistancePos =
		pSummary.rfind(lNearDistanceTag);
	if((lNearDistancePos != std::string::npos) &&
		(lNearDistancePos + lNearDistanceTag.length() < pSummary.length()))
	{
		pSettings.mGhostNearDistance = (float) atoi(pSummary.c_str() +
			lNearDistancePos + lNearDistanceTag.length());
		pSummary.erase(lNearDistancePos);
		pSummary = TrimRightSpaces(pSummary);
		lParsed = true;
	}

	const std::string::size_type lNearOpacityPos =
		pSummary.rfind(lNearOpacityTag);
	if((lNearOpacityPos != std::string::npos) &&
		(lNearOpacityPos + lNearOpacityTag.length() < pSummary.length()))
	{
		int lOpacityPercent = atoi(pSummary.c_str() + lNearOpacityPos +
			lNearOpacityTag.length());
		pSettings.mGhostNearOpacity = lOpacityPercent / 100.0f;
		pSummary.erase(lNearOpacityPos);
		pSummary = TrimRightSpaces(pSummary);
		lParsed = true;
	}

	const std::string::size_type lWarPos = pSummary.rfind(lWarTag);
	if((lWarPos != std::string::npos) &&
		(lWarPos + lWarTag.length() < pSummary.length()))
	{
		int lTargetScore = 0;
		int lWinBy = 0;

		if(sscanf(pSummary.c_str() + lWarPos + lWarTag.length(), "%d %d",
			&lTargetScore, &lWinBy) == 2)
		{
			pSettings.mWarTargetScore = lTargetScore;
			pSettings.mWarWinBy = lWinBy;
			pSummary.erase(lWarPos);
			pSummary = TrimRightSpaces(pSummary);
			lParsed = true;
		}
	}

	const std::string::size_type lModePos = pSummary.rfind(lModeTag);
	if((lModePos != std::string::npos) &&
		(lModePos + lModeTag.length() < pSummary.length()))
	{
		const char *lModeToken = pSummary.c_str() + lModePos + lModeTag.length();

		if(strcmp(lModeToken, MR_GetGameRuleToken(MR_GR_NO_COLLISION_RACE)) == 0) {
			pSettings.mModeId = MR_GR_NO_COLLISION_RACE;
		}
		else if(strcmp(lModeToken,
			MR_GetGameRuleToken(MR_GR_FIRST_LAP_GHOST_RACE)) == 0)
		{
			pSettings.mModeId = MR_GR_FIRST_LAP_GHOST_RACE;
		}
		else if((strcmp(lModeToken, MR_GetGameRuleToken(MR_GR_WAR)) == 0) ||
			(strcmp(lModeToken, "solo_war") == 0))
		{
			pSettings.mModeId = MR_GR_WAR;
		}
		else {
			pSettings.mModeId = MR_GR_NORMAL_RACE;
		}

		pSummary.erase(lModePos);
		pSummary = TrimRightSpaces(pSummary);
		lParsed = true;
	}

	MR_NormalizeGameRuleSettings(pSettings);
	return lParsed;
}

bool MR_ParseGameRulePayload(std::string &pPayload,
	MR_GameRuleSettings &pSettings)
{
	return MR_ParseGameRuleSummary(pPayload, pSettings);
}



