#ifndef GAME_RULES_H
#define GAME_RULES_H

#include <string>
#include "../Util/WorldCoordinates.h"

class MR_ClientSession;
class MR_MainCharacter;

enum MR_GameRuleId
{
	MR_GR_NORMAL_RACE = 0,
	MR_GR_NO_COLLISION_RACE,
	MR_GR_FIRST_LAP_GHOST_RACE,
	MR_GR_WAR
};

enum MR_GameRuleClassification
{
	MR_GRC_STANDARD = 0,
	MR_GRC_CUSTOM
};

struct MR_GameRuleSettings
{
	MR_GameRuleId mModeId;
	int mWarTargetScore;
	int mWarWinBy;
	float mRemoteCraftOpacity;
	float mGhostNearOpacity;
	float mGhostNearDistance;
	float mGhostFadeDistance;

	MR_GameRuleSettings();
};

struct MR_GameRulePlayerState
{
	int mHoverId;
	int mLapNumber;
	int mCheckpointIndex;
	int mHitOtherCount;
	int mHitByOtherCount;
	BOOL mHasFinished;
	BOOL mCraftCollisionEnabled;
	BOOL mColumnInteractionEnabled;
	float mBaseOpacity;

	MR_GameRulePlayerState();
};

struct MR_GameRuleSpawnContext
{
	int mHoverId;
	int mSpawnSlot;
};

struct MR_GameRuleSetupOptions
{
	BOOL mAvailableInPractice;
	BOOL mLapCountEditable;
	BOOL mWeaponsEditable;
	BOOL mWeaponsForcedOn;
	BOOL mUsesGhostTransparencySettings;
	BOOL mUsesWarScoreSettings;

	MR_GameRuleSetupOptions();
};

class MR_GameRuleRuntime
{
	protected:
		MR_GameRuleSettings mSettings;
		MR_GameRuleClassification mClassification;

	public:
		MR_GameRuleRuntime(const MR_GameRuleSettings &pSettings);
		virtual ~MR_GameRuleRuntime();

		virtual void OnInit(MR_ClientSession &pSession);
		virtual void OnPreSpawn(MR_ClientSession &pSession,
			MR_GameRuleSpawnContext &pContext);
		virtual void OnCheckpoint(MR_ClientSession &pSession, int pHoverId,
			int pCheckpointIndex);
		virtual void OnLapComplete(MR_ClientSession &pSession, int pHoverId,
			int pLapNumber);
		virtual void OnHit(MR_ClientSession &pSession, int pVictimHoverId,
			int pSourceHoverId, int pElementId);
		virtual void OnProcessTick(MR_ClientSession &pSession);
		virtual BOOL UsesHitScoring() const;
		virtual void GetSetupOptions(MR_GameRuleSetupOptions &pOptions) const;
		virtual BOOL IsAvailableInPractice() const;
		virtual BOOL IsLapCountEditableInSetup() const;
		virtual BOOL ValidateSettings(char *pErrorBuffer, int pErrorBufferLen) const;
		virtual void ApplySessionOptions(BOOL &pAllowWeapons,
			BOOL &pAllowCans, BOOL &pAllowMines) const;
		virtual BOOL TryGetPlayerState(const MR_ClientSession &pSession,
			int pHoverId, MR_GameRulePlayerState &pState) const;
		virtual float GetRemotePlayerOpacityForView(
			const MR_ClientSession &pSession,
			const MR_MainCharacter *pViewingCharacter,
			const MR_MainCharacter *pTargetCharacter,
			float pDefaultOpacity) const;
		virtual BOOL FormatHudText(const MR_ClientSession &pSession,
			const MR_MainCharacter *pViewingCharacter,
			MR_SimulationTime pTime, char *pMainBuffer, int pMainBufferLen,
			char *pSecondaryBuffer, int pSecondaryBufferLen) const;

		const MR_GameRuleSettings &GetSettings() const;
		MR_GameRuleClassification GetClassification() const;
		BOOL IsStandardRule() const;

		static MR_GameRuleRuntime *Create(const MR_GameRuleSettings &pSettings);
};

void MR_NormalizeGameRuleSettings(MR_GameRuleSettings &pSettings);
void MR_GetGameRuleSetupOptions(const MR_GameRuleSettings &pSettings,
	MR_GameRuleSetupOptions &pOptions);
BOOL MR_IsGameRuleAvailableInPractice(const MR_GameRuleSettings &pSettings);
BOOL MR_IsLapCountEditableInSetup(const MR_GameRuleSettings &pSettings);
BOOL MR_ValidateGameRuleSettings(const MR_GameRuleSettings &pSettings,
	char *pErrorBuffer, int pErrorBufferLen);
const char *MR_GetGameRuleToken(MR_GameRuleId pModeId);
const char *MR_GetGameRuleDisplayName(MR_GameRuleId pModeId);
std::string MR_FormatGameRuleConfigSummary(const MR_GameRuleSettings &pSettings,
	int pLapCount);
std::string MR_FormatGameRuleSummary(const MR_GameRuleSettings &pSettings);
std::string MR_FormatGameRulePayload(const MR_GameRuleSettings &pSettings);
bool MR_ParseGameRuleSummary(std::string &pSummary,
	MR_GameRuleSettings &pSettings);
bool MR_ParseGameRulePayload(std::string &pPayload,
	MR_GameRuleSettings &pSettings);

#endif



