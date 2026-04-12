// TrackSelect.cpp
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
#include "TrackSelect.h"
#include "Banner.h"
#include "TrackDownloadDialog.h"
#include "../Util/Cursor.h"
#include "resource.h"
#include "io.h"
#include "../MazeCompiler/TrackCommonStuff.h"
#include "../Util/StrRes.h"
#include "../Util/Config.h"
#include "../Util/Net/Agent.h"
#include "../Util/Net/NetExn.h"
#include "../VideoServices/ColorPalette.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {
	const int TRACK_MAP_RECORD = 3;
	const COLORREF TRACK_PREVIEW_BACKGROUND = GetSysColor(COLOR_3DFACE);
	const DWORD TRACK_SEARCH_TIMEOUT = 1500;
	const UINT_PTR REMOTE_TRACK_SEARCH_TIMER = 2401;
	const UINT REMOTE_TRACK_SEARCH_DELAY = 500;
	const size_t REMOTE_TRACK_SEARCH_MIN_CHARS = 2;
	const UINT WM_REMOTE_TRACK_SEARCH_COMPLETE = WM_APP + 201;
	const UINT WM_REMOTE_TRACK_PREVIEW_COMPLETE = WM_APP + 202;
	const char *REMOTE_TRACK_SEARCH_URL =
		"http://www.hoverrace.com/tracks/search.php?json=1&search=";
	const int ALLOWED_CRAFT_MODELS[] = { 0, 1, 2, 7 };
	const size_t ALLOWED_CRAFT_MODEL_COUNT =
		sizeof(ALLOWED_CRAFT_MODELS) / sizeof(ALLOWED_CRAFT_MODELS[0]);

	unsigned BuildDefaultAllowedCraftMask()
	{
		unsigned lReturnValue = 0;

		for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
			lReturnValue |= (1u << ALLOWED_CRAFT_MODELS[lIndex]);
		}

		return lReturnValue;
	}

	std::string TrimCopy(const std::string &value)
	{
		size_t begin = 0;
		size_t end = value.length();

		while((begin < end) && std::isspace((unsigned char) value[begin])) {
			++begin;
		}
		while((end > begin) && std::isspace((unsigned char) value[end - 1])) {
			--end;
		}

		return value.substr(begin, end - begin);
	}

	std::string ToLowerCopy(const std::string &value)
	{
		std::string lReturnValue = value;

		for(size_t lIndex = 0; lIndex < lReturnValue.length(); ++lIndex) {
			lReturnValue[lIndex] = (char) std::tolower((unsigned char) lReturnValue[lIndex]);
		}

		return lReturnValue;
	}

	std::string Utf8ToAnsi(const std::string &value)
	{
		if(value.empty()) {
			return value;
		}

		int wideLen = MultiByteToWideChar(CP_UTF8, 0, value.c_str(),
			(int) value.length(), NULL, 0);
		if(wideLen <= 0) {
			return value;
		}

		std::vector<wchar_t> wide((size_t) wideLen);
		if(MultiByteToWideChar(CP_UTF8, 0, value.c_str(), (int) value.length(),
			&wide[0], wideLen) <= 0)
		{
			return value;
		}

		int ansiLen = WideCharToMultiByte(CP_ACP, 0, &wide[0], wideLen,
			NULL, 0, NULL, NULL);
		if(ansiLen <= 0) {
			return value;
		}

		std::string ansi((size_t) ansiLen, '\0');
		if(WideCharToMultiByte(CP_ACP, 0, &wide[0], wideLen, &ansi[0],
			ansiLen, NULL, NULL) <= 0)
		{
			return value;
		}

		return ansi;
	}

	void AppendUtf8CodePoint(std::string &value, unsigned codePoint)
	{
		if(codePoint <= 0x7f) {
			value += (char) codePoint;
		}
		else if(codePoint <= 0x7ff) {
			value += (char) (0xc0 | ((codePoint >> 6) & 0x1f));
			value += (char) (0x80 | (codePoint & 0x3f));
		}
		else if(codePoint <= 0xffff) {
			value += (char) (0xe0 | ((codePoint >> 12) & 0x0f));
			value += (char) (0x80 | ((codePoint >> 6) & 0x3f));
			value += (char) (0x80 | (codePoint & 0x3f));
		}
		else {
			value += (char) (0xf0 | ((codePoint >> 18) & 0x07));
			value += (char) (0x80 | ((codePoint >> 12) & 0x3f));
			value += (char) (0x80 | ((codePoint >> 6) & 0x3f));
			value += (char) (0x80 | (codePoint & 0x3f));
		}
	}

	bool ParseHexDigit(char ch, unsigned &value)
	{
		if((ch >= '0') && (ch <= '9')) {
			value = (unsigned) (ch - '0');
			return true;
		}
		if((ch >= 'a') && (ch <= 'f')) {
			value = (unsigned) (ch - 'a' + 10);
			return true;
		}
		if((ch >= 'A') && (ch <= 'F')) {
			value = (unsigned) (ch - 'A' + 10);
			return true;
		}
		return false;
	}

	bool IsInternetMeetingRoomWindow(HWND wnd)
	{
		return
			(GetDlgItem(wnd, IDC_GAME_LIST) != NULL) &&
			(GetDlgItem(wnd, IDC_USER_LIST) != NULL);
	}

	void PositionDialogToRightOfOwner(HWND dialog, int gap = 3)
	{
		const int MIN_SIDE_BY_SIDE_WIDTH = 1200;
		const int RIGHT_POSITION_NUDGE = 4;

		HWND owner = GetWindow(dialog, GW_OWNER);
		if(owner == NULL) {
			owner = GetParent(dialog);
		}

		if((owner == NULL) || !IsWindowVisible(owner) || !IsInternetMeetingRoomWindow(owner)) {
			return;
		}

		RECT ownerRect;
		RECT dialogRect;
		if(!GetWindowRect(owner, &ownerRect) || !GetWindowRect(dialog, &dialogRect)) {
			return;
		}

		HMONITOR monitor = MonitorFromRect(&ownerRect, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		if(!GetMonitorInfo(monitor, &monitorInfo)) {
			SystemParametersInfo(SPI_GETWORKAREA, 0, &monitorInfo.rcWork, 0);
		}

		const int workAreaWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
		if(workAreaWidth < MIN_SIDE_BY_SIDE_WIDTH) {
			return;
		}

		const int dialogWidth = dialogRect.right - dialogRect.left;
		const int dialogHeight = dialogRect.bottom - dialogRect.top;

		int x = ownerRect.right + gap - RIGHT_POSITION_NUDGE;
		int y = ownerRect.top;

		if((x + dialogWidth) > monitorInfo.rcWork.right) {
			x = ownerRect.right - dialogWidth - gap;
		}
		if(x < monitorInfo.rcWork.left) {
			x = monitorInfo.rcWork.left;
		}

		if((y + dialogHeight) > monitorInfo.rcWork.bottom) {
			y = monitorInfo.rcWork.bottom - dialogHeight;
		}
		if(y < monitorInfo.rcWork.top) {
			y = monitorInfo.rcWork.top;
		}

		SetWindowPos(dialog, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	}

	struct TrackPreviewData
	{
		int mWidth;
		int mHeight;
		std::vector<MR_UInt8> mBitmap;
		HBITMAP mBitmapHandle;
		bool mLoading;
		bool mSmoothScale;

		TrackPreviewData() :
			mWidth(0), mHeight(0), mBitmapHandle(NULL), mLoading(false),
			mSmoothScale(false) { }

		~TrackPreviewData()
		{
			Reset();
		}

		void Reset()
		{
			if(mBitmapHandle != NULL) {
				DeleteObject(mBitmapHandle);
				mBitmapHandle = NULL;
			}
			mWidth = 0;
			mHeight = 0;
			mLoading = false;
			mSmoothScale = false;
			mBitmap.clear();
		}

		bool IsAvailable() const
		{
			return (mBitmapHandle != NULL) || !mBitmap.empty();
		}
	};

	struct BitmapPixels
	{
		int mWidth;
		int mHeight;
		std::vector<MR_UInt8> mBitmap;

		BitmapPixels() : mWidth(0), mHeight(0) { }
	};
}

class TrackEntry
{
	public:
		TrackEntry() :
			mRegistrationMode(0), mSortingIndex(0), mIsRemote(false)
		{
		}

		std::string mFileName;
		std::string mPath;
		std::string mDescription;
		int mRegistrationMode;
		int mSortingIndex;
		bool mIsRemote;
		std::string mDownloadName;
		std::string mMapGifUrl;
		std::string mPageUrl;

		bool operator<(const TrackEntry &elem2) const
		{
			int diff = mSortingIndex - elem2.mSortingIndex;
			if (diff == 0) {
				return mFileName < elem2.mFileName;
			}
			else {
				return (diff < 0);
			}
		}
};

/// Search path for tracks (not including download path from config).
static const char *TRACK_PATHS[] = {
	".\\track_source\\",
	".\\Tracks\\",
	"..\\Tracks\\",
};
#define NUM_TRACK_PATHS 3

#define TRACK_EXT   ".trk"

// Local functions
static BOOL CALLBACK TrackSelectCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
static BOOL ReadTrackEntry(MR_RecordFile * pRecordFile, TrackEntry * pDest, const char *pFileName);
static bool CompareFunc(const TrackEntry *ent1, const TrackEntry *ent2);
static void ClearTrackPreview();
static void DrawTrackPreview(const DRAWITEMSTRUCT *pDrawItem);
static void DrawTrackListItem(const DRAWITEMSTRUCT *pDrawItem);
static COLORREF GetTrackPreviewColor(MR_UInt8 pColorIndex);
static void InitTrackPreviewPalette();
static bool LoadTrackPreview(const TrackEntry &pEntry);
static void LoadRemoteTrackPreview(HWND pWindow, const TrackEntry &pEntry);
static bool CopyRemotePreviewBitmap(HBITMAP pBitmap, BitmapPixels &pPreview);
static LRESULT CALLBACK TrackListProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
static void SortList();
static void RebuildVisibleTrackList();
static void RefreshTrackList(HWND pWindow);
static void UpdateSelectedTrackInfo(HWND pWindow);
static void SetTrackDescriptionText(HWND pWindow, const char *pText);
static void UpdateTrackDescriptionScrollbar(HWND pWindow);
static bool FinishRemoteTrackSelection(HWND pWindow);
static int HandleTrackListChar(HWND pWindow, UINT pChar);
static void ScheduleRemoteTrackSearch(HWND pWindow);
static void ClearRemoteTrackSearch(HWND pWindow);
static void StartRemoteTrackSearch(HWND pWindow);
static void HandleRemoteTrackSearchComplete(HWND pWindow, LPARAM pLParam);
static void HandleRemoteTrackPreviewComplete(HWND pWindow, LPARAM pLParam);
static void ReadTrackList();
static void ReadTrackListDir(const std::string &dir);
static void CleanList();
static std::string FindTrack(const std::string &name);

// Initial reserve size for track list.
#define INIT_TRACK_ENTRIES 1000

static int gsSelectedEntry = -1;

typedef std::vector<TrackEntry> tracklist_t;
typedef std::vector<TrackEntry*> sorted_t;

static tracklist_t gsTrackList;
static tracklist_t gsRemoteTrackList;
static sorted_t gsSortedTrackList;
static sorted_t gsVisibleTrackList;
static int gsNbLaps;
static BOOL gsAllowWeapons = FALSE;
static BOOL gsAllowCans = FALSE;
static BOOL gsAllowMines = FALSE;
static unsigned gsAllowedCraftMask = 0;
static TrackPreviewData gsTrackPreview;
static COLORREF gsTrackPreviewPalette[MR_NB_COLORS];
static bool gsTrackPreviewPaletteInit = false;
static std::string gsTrackSearchPrefix;
static std::string gsTrackFilter;
static DWORD gsTrackSearchTick = 0;
static WNDPROC gsTrackListWndProc = NULL;
static unsigned gsRemoteSearchSerial = 0;
static unsigned gsRemotePreviewSerial = 0;
static MR_GameRuleSettings gsGameRuleSettings;
static bool gsPracticeMode = false;
static MR_GameRuleId gsVisibleRuleModes[MR_GR_WAR + 1];
static int gsVisibleRuleModeCount = 0;

struct RemoteTrackSearchPayload
{
	unsigned mSerial;
	std::string mFilter;
	tracklist_t mTracks;
};

struct RemoteTrackPreviewPayload
{
	unsigned mSerial;
	BitmapPixels mPreview;

	RemoteTrackPreviewPayload() : mSerial(0) { }
};

/**
 * Open a track file.
 * @param pWindow The window to use as the parent for dialog boxes.
 * @param pFileName The track name (without the ".trk" extension).
 * @return The opened track file, or NULL if the track could not be opened.
 */
MR_RecordFile *MR_TrackOpen(HWND pWindow, const char *pFileName)
{
	MR_RecordFile *lReturnValue = NULL;

	std::string filename = FindTrack(pFileName);

	if (filename.empty()) {
		//MessageBox(pWindow, MR_LoadString(IDS_TRK_NOTFOUND), MR_LoadString(IDS_GAME_NAME), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		lReturnValue = new MR_RecordFile;
		if(!lReturnValue->OpenForRead(filename.c_str(), TRUE)) {
			delete lReturnValue;
			lReturnValue = NULL;
			MessageBox(pWindow, MR_LoadString(IDS_BAD_TRK_FORMAT), MR_LoadString(IDS_GAME_NAME), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			ASSERT(FALSE);
		}
		else {
			TrackEntry lCurrentEntry;

			if(!ReadTrackEntry(lReturnValue, &lCurrentEntry, pFileName)) {
				delete lReturnValue;
				lReturnValue = NULL;

				MessageBox(pWindow, MR_LoadString(IDS_BAD_TRK_FORMAT), MR_LoadString(IDS_GAME_NAME), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}
		}

	}
	return lReturnValue;
}

/**
 * Open the track selection dialog.
 * @param pParentWindow The parent window handle.
 * @param pTrackFile [out] The name of the track.
 * @param pNbLap [out] The number of laps in the race.
 * @param pAllowWeapons [out] Whether weapons are allowed or not.
 * @return @c true if the user selected a track (@p pTrackFile, @p pNbLap, and
 *         @p pAllowWeapons will be filled in), @c false if the user canceled
 *         the dialog.
 */
bool MR_SelectTrack(HWND pParentWindow, std::string &pTrackFile, int &pNbLap,
	bool &pAllowWeapons, bool &pAllowCans, bool &pAllowMines,
	unsigned &pAllowedCraftMask, MR_GameRuleSettings &pGameRuleSettings,
	bool pPracticeMode)
{
	bool lReturnValue = true;
	gsSelectedEntry = -1;
	gsTrackSearchPrefix.clear();
	gsTrackFilter.clear();
	gsTrackSearchTick = 0;

	// Load the entry list
	MR_WAIT_CURSOR ReadTrackList();
	SortList();

	gsNbLaps = 5;								  // Default value
	gsAllowWeapons = TRUE;
	gsAllowCans = TRUE;
	gsAllowMines = TRUE;
	gsAllowedCraftMask = MR_GetDefaultAllowedCraftMask();
	gsGameRuleSettings = MR_GameRuleSettings();
	gsPracticeMode = pPracticeMode;

	if(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TRACK_SELECT), pParentWindow, TrackSelectCallBack) == IDOK) {
		pTrackFile = gsVisibleTrackList[gsSelectedEntry]->mFileName;
		pNbLap = gsNbLaps;
		pAllowWeapons = (gsAllowWeapons != FALSE);
		pAllowCans = (gsAllowCans != FALSE);
		pAllowMines = (gsAllowMines != FALSE);
		pAllowedCraftMask = gsAllowedCraftMask;
		pGameRuleSettings = gsGameRuleSettings;
		lReturnValue = true;
	} else
	lReturnValue = false;
	CleanList();

	return lReturnValue;
}

unsigned MR_GetDefaultAllowedCraftMask()
{
	static const unsigned DEFAULT_ALLOWED_CRAFT_MASK =
		BuildDefaultAllowedCraftMask();

	return DEFAULT_ALLOWED_CRAFT_MASK;
}

unsigned MR_NormalizeAllowedCraftMask(unsigned pAllowedCraftMask)
{
	const unsigned lValidMask = MR_GetDefaultAllowedCraftMask();

	pAllowedCraftMask &= lValidMask;
	return (pAllowedCraftMask != 0) ? pAllowedCraftMask : lValidMask;
}

bool MR_HasAllowedCraft(unsigned pAllowedCraftMask)
{
	return (pAllowedCraftMask & MR_GetDefaultAllowedCraftMask()) != 0;
}

bool MR_IsCraftAllowed(unsigned pAllowedCraftMask, int pCraftId)
{
	if((pCraftId < 0) || (pCraftId >= 32)) {
		return false;
	}

	return (MR_NormalizeAllowedCraftMask(pAllowedCraftMask) &
		(1u << pCraftId)) != 0;
}

int MR_GetFirstAllowedCraft(unsigned pAllowedCraftMask)
{
	const unsigned lMask = MR_NormalizeAllowedCraftMask(pAllowedCraftMask);

	for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
		if((lMask & (1u << ALLOWED_CRAFT_MODELS[lIndex])) != 0) {
			return ALLOWED_CRAFT_MODELS[lIndex];
		}
	}

	return ALLOWED_CRAFT_MODELS[0];
}

int MR_GetNextAllowedCraft(unsigned pAllowedCraftMask, int pCurrentCraftId,
	int pDirection)
{
	const unsigned lMask = MR_NormalizeAllowedCraftMask(pAllowedCraftMask);
	int lCurrentIndex = -1;
	int lStep = (pDirection < 0) ? -1 : 1;

	for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
		if(ALLOWED_CRAFT_MODELS[lIndex] == pCurrentCraftId) {
			lCurrentIndex = (int) lIndex;
			break;
		}
	}

	if(lCurrentIndex == -1) {
		return MR_GetFirstAllowedCraft(lMask);
	}

	for(size_t lOffset = 0; lOffset < ALLOWED_CRAFT_MODEL_COUNT; ++lOffset) {
		lCurrentIndex =
			(lCurrentIndex + lStep + (int) ALLOWED_CRAFT_MODEL_COUNT) %
			(int) ALLOWED_CRAFT_MODEL_COUNT;
		if((lMask & (1u << ALLOWED_CRAFT_MODELS[lCurrentIndex])) != 0) {
			return ALLOWED_CRAFT_MODELS[lCurrentIndex];
		}
	}

	return MR_GetFirstAllowedCraft(lMask);
}

std::string MR_FormatAllowedCraftMask(unsigned pAllowedCraftMask)
{
	std::ostringstream lOutput;
	bool lFirst = true;
	const unsigned lMask = MR_NormalizeAllowedCraftMask(pAllowedCraftMask);

	for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
		const int lCraftId = ALLOWED_CRAFT_MODELS[lIndex];
		if((lMask & (1u << lCraftId)) == 0) {
			continue;
		}

		if(!lFirst) {
			lOutput << ",";
		}
		lOutput << lCraftId;
		lFirst = false;
	}

	return lOutput.str();
}

std::string MR_FormatAllowedCraftDisplayMask(unsigned pAllowedCraftMask)
{
	static const char *CRAFT_DISPLAY_NAMES[] = {
		"Basic",
		"Low CX",
		"Bi-Turbo",
		"EON"
	};

	std::ostringstream lOutput;
	bool lFirst = true;
	const unsigned lMask = MR_NormalizeAllowedCraftMask(pAllowedCraftMask);

	for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
		const int lCraftId = ALLOWED_CRAFT_MODELS[lIndex];
		if((lMask & (1u << lCraftId)) == 0) {
			continue;
		}

		if(!lFirst) {
			lOutput << ", ";
		}
		lOutput << CRAFT_DISPLAY_NAMES[lIndex];
		lFirst = false;
	}

	return lOutput.str();
}

std::string MR_FormatPowerupDisplay(bool pAllowWeapons, bool pAllowCans,
	bool pAllowMines)
{
	std::ostringstream lOutput;
	bool lFirst = true;

	if(pAllowWeapons) {
		lOutput << "Missiles";
		lFirst = false;
	}
	if(pAllowCans) {
		if(!lFirst) {
			lOutput << ", ";
		}
		lOutput << "Cans";
		lFirst = false;
	}
	if(pAllowMines) {
		if(!lFirst) {
			lOutput << ", ";
		}
		lOutput << "Mines";
		lFirst = false;
	}

	if(lFirst) {
		return "None";
	}

	return lOutput.str();
}

static bool IsUrlSafeChar(unsigned char ch)
{
	return std::isalnum(ch) || (ch == '-') || (ch == '_') ||
		(ch == '.') || (ch == '~');
}

static std::string UrlEncode(const std::string &value)
{
	static const char HEX[] = "0123456789ABCDEF";
	std::string lReturnValue;

	for(size_t lIndex = 0; lIndex < value.length(); ++lIndex) {
		unsigned char ch = (unsigned char) value[lIndex];
		if(IsUrlSafeChar(ch)) {
			lReturnValue += (char) ch;
		}
		else {
			lReturnValue += '%';
			lReturnValue += HEX[(ch >> 4) & 0x0f];
			lReturnValue += HEX[ch & 0x0f];
		}
	}

	return lReturnValue;
}

static int GetSelectedRuleMode(HWND pWindow)
{
	int lSelection = (int) SendDlgItemMessage(pWindow, IDC_RULE_MODE,
		CB_GETCURSEL, 0, 0);

	if((lSelection >= 0) && (lSelection < gsVisibleRuleModeCount)) {
		return gsVisibleRuleModes[lSelection];
	}

	return MR_GR_NORMAL_RACE;
}

static void SetSelectedRuleMode(HWND pWindow, MR_GameRuleId pModeId)
{
	int lSelection = 0;

	for(int lIndex = 0; lIndex < gsVisibleRuleModeCount; lIndex++) {
		if(gsVisibleRuleModes[lIndex] == pModeId) {
			lSelection = lIndex;
			break;
		}
	}

	SendDlgItemMessage(pWindow, IDC_RULE_MODE, CB_SETCURSEL, lSelection, 0);
}

static void PopulateRuleModeList(HWND pWindow)
{
	static const MR_GameRuleId ALL_RULE_MODES[] = {
		MR_GR_NORMAL_RACE,
		MR_GR_NO_COLLISION_RACE,
		MR_GR_FIRST_LAP_GHOST_RACE,
		MR_GR_WAR
	};

	gsVisibleRuleModeCount = 0;
	SendDlgItemMessage(pWindow, IDC_RULE_MODE, CB_RESETCONTENT, 0, 0);

	for(size_t lIndex = 0;
		lIndex < (sizeof(ALL_RULE_MODES) / sizeof(ALL_RULE_MODES[0]));
		lIndex++)
	{
		MR_GameRuleSettings lRuleSettings;
		lRuleSettings.mModeId = ALL_RULE_MODES[lIndex];
		MR_NormalizeGameRuleSettings(lRuleSettings);

		if(gsPracticeMode && !MR_IsGameRuleAvailableInPractice(lRuleSettings)) {
			continue;
		}

		gsVisibleRuleModes[gsVisibleRuleModeCount++] = ALL_RULE_MODES[lIndex];
		SendDlgItemMessage(pWindow, IDC_RULE_MODE, CB_ADDSTRING, 0,
			(LPARAM) MR_GetGameRuleDisplayName(ALL_RULE_MODES[lIndex]));
	}

	if(gsVisibleRuleModeCount <= 0) {
		gsVisibleRuleModes[0] = MR_GR_NORMAL_RACE;
		gsVisibleRuleModeCount = 1;
		SendDlgItemMessage(pWindow, IDC_RULE_MODE, CB_ADDSTRING, 0,
			(LPARAM) MR_GetGameRuleDisplayName(MR_GR_NORMAL_RACE));
	}
}

static void UpdateRuleControlState(HWND pWindow)
{
	MR_GameRuleSettings lRuleSettings;
	MR_GameRuleSetupOptions lSetupOptions;

	lRuleSettings.mModeId = (MR_GameRuleId) GetSelectedRuleMode(pWindow);
	MR_NormalizeGameRuleSettings(lRuleSettings);
	MR_GetGameRuleSetupOptions(lRuleSettings, lSetupOptions);

	EnableWindow(GetDlgItem(pWindow, IDC_NB_LAP), lSetupOptions.mLapCountEditable);
	EnableWindow(GetDlgItem(pWindow, IDC_NB_LAP_SPIN),
		lSetupOptions.mLapCountEditable);
	if(lSetupOptions.mWeaponsForcedOn) {
		SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_SETCHECK, BST_CHECKED, 0);
	}
	EnableWindow(GetDlgItem(pWindow, IDC_WEAPONS), lSetupOptions.mWeaponsEditable);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_OPACITY),
		lSetupOptions.mUsesGhostTransparencySettings);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_NEAR_OPACITY),
		lSetupOptions.mUsesGhostTransparencySettings);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_NEAR_DISTANCE),
		lSetupOptions.mUsesGhostTransparencySettings);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_FADE_DISTANCE),
		lSetupOptions.mUsesGhostTransparencySettings);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_WAR_TARGET),
		lSetupOptions.mUsesWarScoreSettings);
	EnableWindow(GetDlgItem(pWindow, IDC_RULE_WAR_WINBY),
		lSetupOptions.mUsesWarScoreSettings);
}

static void SkipJsonWhitespace(const std::string &json, size_t &pos)
{
	while((pos < json.length()) &&
		((json[pos] == ' ') || (json[pos] == '\t') ||
		(json[pos] == '\r') || (json[pos] == '\n')))
	{
		++pos;
	}
}

static bool ParseJsonString(const std::string &json, size_t &pos, std::string &value)
{
	value.clear();
	SkipJsonWhitespace(json, pos);
	if((pos >= json.length()) || (json[pos] != '"')) {
		return false;
	}
	++pos;

	while(pos < json.length()) {
		char ch = json[pos++];
		if(ch == '"') {
			return true;
		}
		if(ch != '\\') {
			value += ch;
			continue;
		}
		if(pos >= json.length()) {
			return false;
		}
		ch = json[pos++];
		switch(ch) {
			case '"':
			case '\\':
			case '/':
				value += ch;
				break;
			case 'b':
				value += '\b';
				break;
			case 'f':
				value += '\f';
				break;
			case 'n':
				value += '\n';
				break;
			case 'r':
				value += '\r';
				break;
			case 't':
				value += '\t';
				break;
			case 'u':
			{
				unsigned codePoint = 0;
				if((pos + 4) > json.length()) {
					return false;
				}
				for(int i = 0; i < 4; ++i) {
					unsigned digit;
					if(!ParseHexDigit(json[pos + i], digit)) {
						return false;
					}
					codePoint = (codePoint << 4) | digit;
				}
				pos += 4;

				if((codePoint >= 0xd800) && (codePoint <= 0xdbff)) {
					unsigned low = 0;
					if((pos + 6) > json.length() ||
						(json[pos] != '\\') || (json[pos + 1] != 'u'))
					{
						return false;
					}
					pos += 2;
					for(int i = 0; i < 4; ++i) {
						unsigned digit;
						if(!ParseHexDigit(json[pos + i], digit)) {
							return false;
						}
						low = (low << 4) | digit;
					}
					pos += 4;
					if((low < 0xdc00) || (low > 0xdfff)) {
						return false;
					}
					codePoint = 0x10000 +
						(((codePoint - 0xd800) << 10) | (low - 0xdc00));
				}
				else if((codePoint >= 0xdc00) && (codePoint <= 0xdfff)) {
					return false;
				}

				AppendUtf8CodePoint(value, codePoint);
				break;
			}
			default:
				return false;
		}
	}

	return false;
}

static bool SkipJsonValue(const std::string &json, size_t &pos)
{
	SkipJsonWhitespace(json, pos);
	if(pos >= json.length()) {
		return false;
	}

	if(json[pos] == '"') {
		std::string ignored;
		return ParseJsonString(json, pos, ignored);
	}

	if((json[pos] == '{') || (json[pos] == '[')) {
		std::vector<char> stack;
		stack.push_back((json[pos] == '{') ? '}' : ']');
		++pos;
		while((pos < json.length()) && !stack.empty()) {
			if(json[pos] == '"') {
				std::string ignored;
				if(!ParseJsonString(json, pos, ignored)) {
					return false;
				}
			}
			else {
				if(json[pos] == '{') {
					stack.push_back('}');
				}
				else if(json[pos] == '[') {
					stack.push_back(']');
				}
				else if(json[pos] == stack.back()) {
					stack.pop_back();
				}
				++pos;
			}
		}
		return stack.empty();
	}

	while((pos < json.length()) && (json[pos] != ',') &&
		(json[pos] != '}') && (json[pos] != ']'))
	{
		++pos;
	}
	return true;
}

static bool ParseRemoteTrackObject(const std::string &json, size_t &pos, TrackEntry &entry)
{
	SkipJsonWhitespace(json, pos);
	if((pos >= json.length()) || (json[pos] != '{')) {
		return false;
	}
	++pos;

	entry = TrackEntry();
	entry.mIsRemote = true;
	entry.mSortingIndex = 1000000;

	for(;;) {
		std::string key;
		std::string value;

		SkipJsonWhitespace(json, pos);
		if((pos < json.length()) && (json[pos] == '}')) {
			++pos;
			break;
		}
		if(!ParseJsonString(json, pos, key)) {
			return false;
		}
		SkipJsonWhitespace(json, pos);
		if((pos >= json.length()) || (json[pos] != ':')) {
			return false;
		}
		++pos;

		if((key == "name") || (key == "downloadName") ||
			(key == "description") || (key == "mapGifUrl") ||
			(key == "pageUrl"))
		{
			if(!ParseJsonString(json, pos, value)) {
				return false;
			}
			if(key == "name") {
				entry.mFileName = Utf8ToAnsi(value);
			}
			else if(key == "downloadName") {
				entry.mDownloadName = Utf8ToAnsi(value);
			}
			else if(key == "description") {
				entry.mDescription = Utf8ToAnsi(value);
			}
			else if(key == "mapGifUrl") {
				entry.mMapGifUrl = value;
			}
			else if(key == "pageUrl") {
				entry.mPageUrl = value;
			}
		}
		else if(!SkipJsonValue(json, pos)) {
			return false;
		}

		SkipJsonWhitespace(json, pos);
		if((pos < json.length()) && (json[pos] == ',')) {
			++pos;
			continue;
		}
		if((pos < json.length()) && (json[pos] == '}')) {
			++pos;
			break;
		}
		return false;
	}

	if(entry.mDownloadName.empty()) {
		entry.mDownloadName = entry.mFileName;
	}
	return !entry.mFileName.empty() && !entry.mDownloadName.empty();
}

static bool ParseRemoteTrackSearchJson(const std::string &json, tracklist_t &tracks)
{
	size_t pos = 0;
	tracks.clear();

	SkipJsonWhitespace(json, pos);
	if((pos >= json.length()) || (json[pos] != '{')) {
		return false;
	}
	++pos;

	for(;;) {
		std::string key;

		SkipJsonWhitespace(json, pos);
		if((pos < json.length()) && (json[pos] == '}')) {
			return true;
		}
		if(!ParseJsonString(json, pos, key)) {
			return false;
		}
		SkipJsonWhitespace(json, pos);
		if((pos >= json.length()) || (json[pos] != ':')) {
			return false;
		}
		++pos;

		if(key == "tracks") {
			SkipJsonWhitespace(json, pos);
			if((pos >= json.length()) || (json[pos] != '[')) {
				return false;
			}
			++pos;
			for(;;) {
				TrackEntry entry;
				SkipJsonWhitespace(json, pos);
				if((pos < json.length()) && (json[pos] == ']')) {
					++pos;
					break;
				}
				if(ParseRemoteTrackObject(json, pos, entry)) {
					tracks.push_back(entry);
				}
				else {
					return false;
				}
				SkipJsonWhitespace(json, pos);
				if((pos < json.length()) && (json[pos] == ',')) {
					++pos;
					continue;
				}
				if((pos < json.length()) && (json[pos] == ']')) {
					++pos;
					break;
				}
				return false;
			}
		}
		else if(!SkipJsonValue(json, pos)) {
			return false;
		}

		SkipJsonWhitespace(json, pos);
		if((pos < json.length()) && (json[pos] == ',')) {
			++pos;
			continue;
		}
		if((pos < json.length()) && (json[pos] == '}')) {
			return true;
		}
		return false;
	}
}

unsigned MR_ParseAllowedCraftMask(const char *pAllowedCrafts)
{
	static const char *CRAFT_DISPLAY_NAMES[] = {
		"basic",
		"low cx",
		"bi-turbo",
		"eon"
	};

	std::string lAllowedCrafts =
		(pAllowedCrafts != NULL) ? pAllowedCrafts : "";
	unsigned lMask = 0;
	size_t lStart = 0;

	lAllowedCrafts = TrimCopy(lAllowedCrafts);
	if(lAllowedCrafts.empty()) {
		return MR_GetDefaultAllowedCraftMask();
	}

	while(lStart <= lAllowedCrafts.length()) {
		size_t lEnd = lAllowedCrafts.find(',', lStart);
		std::string lToken = TrimCopy(lAllowedCrafts.substr(lStart,
			(lEnd == std::string::npos) ? std::string::npos : (lEnd - lStart)));

		if(!lToken.empty()) {
			bool lNumeric = true;
			for(size_t lIndex = 0; lIndex < lToken.length(); ++lIndex) {
				if(!std::isdigit((unsigned char) lToken[lIndex])) {
					lNumeric = false;
					break;
				}
			}

			if(lNumeric) {
				const int lCraftId = atoi(lToken.c_str());
				if((lCraftId >= 0) && (lCraftId < 32)) {
					const unsigned lCraftBit = (1u << lCraftId);
					if((MR_GetDefaultAllowedCraftMask() & lCraftBit) != 0) {
						lMask |= lCraftBit;
					}
				}
			}
			else {
				const std::string lLowerToken = ToLowerCopy(lToken);
				for(size_t lIndex = 0; lIndex < ALLOWED_CRAFT_MODEL_COUNT; ++lIndex) {
					if(lLowerToken == CRAFT_DISPLAY_NAMES[lIndex]) {
						lMask |= (1u << ALLOWED_CRAFT_MODELS[lIndex]);
						break;
					}
				}
			}
		}

		if(lEnd == std::string::npos) {
			break;
		}
		lStart = lEnd + 1;
	}

	return (lMask != 0) ? lMask : MR_GetDefaultAllowedCraftMask();
}

static bool StartsWithNoCase(const std::string &value, size_t offset, const std::string &prefix)
{
	if((offset + prefix.length()) > value.length()) {
		return false;
	}

	for(size_t i = 0; i < prefix.length(); ++i) {
		if(std::tolower((unsigned char) value[offset + i]) != std::tolower((unsigned char) prefix[i])) {
			return false;
		}
	}

	return true;
}

static bool ContainsNoCase(const std::string &value, const std::string &needle)
{
	if(needle.empty()) {
		return true;
	}

	if(needle.length() > value.length()) {
		return false;
	}

	for(size_t offset = 0; (offset + needle.length()) <= value.length(); ++offset) {
		if(StartsWithNoCase(value, offset, needle)) {
			return true;
		}
	}

	return false;
}

static bool EqualsNoCase(const std::string &left, const std::string &right)
{
	if(left.length() != right.length()) {
		return false;
	}

	for(size_t i = 0; i < left.length(); ++i) {
		if(std::tolower((unsigned char) left[i]) !=
			std::tolower((unsigned char) right[i]))
		{
			return false;
		}
	}

	return true;
}

static int FindTrackPrefixMatch(const std::string &prefix)
{
	if(prefix.empty()) {
		return -1;
	}

	for(size_t i = 0; i < gsVisibleTrackList.size(); ++i) {
		if(StartsWithNoCase(gsVisibleTrackList[i]->mFileName, 0, prefix)) {
			return (int) i;
		}
	}

	return -1;
}

static int FindVisibleTrackByName(const std::string &name)
{
	for(size_t i = 0; i < gsVisibleTrackList.size(); ++i) {
		if(gsVisibleTrackList[i]->mFileName == name) {
			return (int) i;
		}
	}

	return -1;
}

static std::string GetSelectedTrackName()
{
	if((gsSelectedEntry >= 0) && ((size_t) gsSelectedEntry < gsVisibleTrackList.size())) {
		return gsVisibleTrackList[gsSelectedEntry]->mFileName;
	}

	return std::string();
}

static void SelectTrackEntry(HWND pWindow, int entry)
{
	if((entry >= 0) && ((size_t) entry < gsVisibleTrackList.size())) {
		gsSelectedEntry = entry;
		SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, entry, 0);
		UpdateSelectedTrackInfo(pWindow);
	}
}

static void RebuildVisibleTrackList()
{
	gsVisibleTrackList.clear();

	for(size_t i = 0; i < gsSortedTrackList.size(); ++i) {
		if(ContainsNoCase(gsSortedTrackList[i]->mFileName, gsTrackFilter)) {
			gsVisibleTrackList.push_back(gsSortedTrackList[i]);
		}
	}

	for(size_t i = 0; i < gsRemoteTrackList.size(); ++i) {
		bool duplicate = false;
		for(size_t j = 0; j < gsVisibleTrackList.size(); ++j) {
			if(EqualsNoCase(gsRemoteTrackList[i].mFileName,
				gsVisibleTrackList[j]->mFileName))
			{
				duplicate = true;
				break;
			}
		}

		if(!duplicate) {
			gsVisibleTrackList.push_back(&gsRemoteTrackList[i]);
		}
	}
}

static void RefreshTrackList(HWND pWindow)
{
	std::string selectedTrack = GetSelectedTrackName();

	RebuildVisibleTrackList();

	SendDlgItemMessage(pWindow, IDC_LIST, LB_RESETCONTENT, 0, 0);
	for(sorted_t::iterator iter = gsVisibleTrackList.begin();
		iter != gsVisibleTrackList.end(); ++iter)
	{
		SendDlgItemMessage(pWindow, IDC_LIST, LB_ADDSTRING, 0,
			(LPARAM) ((*iter)->mFileName.c_str()));
	}

	if(!selectedTrack.empty()) {
		gsSelectedEntry = FindVisibleTrackByName(selectedTrack);
	}
	else {
		gsSelectedEntry = -1;
	}

	if((gsSelectedEntry == -1) && !gsVisibleTrackList.empty()) {
		gsSelectedEntry = 0;
	}

	SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, gsSelectedEntry, 0);
	UpdateSelectedTrackInfo(pWindow);
}

static int HandleTrackListChar(HWND pWindow, UINT pChar)
{
	DWORD now = GetTickCount();
	if(now - gsTrackSearchTick > TRACK_SEARCH_TIMEOUT) {
		gsTrackSearchPrefix.clear();
	}
	gsTrackSearchTick = now;

	if(pChar == '\b') {
		if(!gsTrackSearchPrefix.empty()) {
			gsTrackSearchPrefix.erase(gsTrackSearchPrefix.length() - 1);
		}

		int match = FindTrackPrefixMatch(gsTrackSearchPrefix);
		if(match != -1) {
			SelectTrackEntry(pWindow, match);
		}
		return -1;
	}

	if(pChar > 0xff) {
		return -2;
	}

	unsigned char ch = (unsigned char) pChar;
	if(!std::isprint(ch)) {
		return -2;
	}

	std::string search = gsTrackSearchPrefix + (char) ch;
	int match = FindTrackPrefixMatch(search);
	if(match == -1) {
		search.assign(1, (char) ch);
		match = FindTrackPrefixMatch(search);
	}

	if(match != -1) {
		gsTrackSearchPrefix = search;
		SelectTrackEntry(pWindow, match);
	}
	else {
		gsTrackSearchPrefix.clear();
	}

	return -1;
}

bool FinishRemoteTrackSelection(HWND pWindow)
{
	if((gsSelectedEntry < 0) ||
		((size_t) gsSelectedEntry >= gsVisibleTrackList.size()) ||
		!gsVisibleTrackList[gsSelectedEntry]->mIsRemote)
	{
		return true;
	}

	std::string downloadName = gsVisibleTrackList[gsSelectedEntry]->mDownloadName;
	std::string displayName = gsVisibleTrackList[gsSelectedEntry]->mFileName;

	if(downloadName.empty()) {
		downloadName = displayName;
	}

	if(!TrackDownloadDialog(downloadName).ShowModal(GetModuleHandle(NULL), pWindow)) {
		return false;
	}

	std::string currentFilter = gsTrackFilter;
	ReadTrackList();
	SortList();
	gsTrackFilter = currentFilter;
	gsRemoteTrackList.clear();
	RefreshTrackList(pWindow);

	gsSelectedEntry = FindVisibleTrackByName(displayName);
	if(gsSelectedEntry == -1) {
		gsSelectedEntry = FindVisibleTrackByName(downloadName);
	}
	if(gsSelectedEntry != -1) {
		SelectTrackEntry(pWindow, gsSelectedEntry);
	}

	MR_RecordFile *trackFile = MR_TrackOpen(pWindow, downloadName.c_str());
	if(trackFile == NULL) {
		MessageBox(pWindow, MR_LoadString(IDS_TRACK_NOTINSTALL),
			MR_LoadString(IDS_GAME_NAME), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
		return false;
	}
	delete trackFile;

	return gsSelectedEntry != -1;
}

static LRESULT CALLBACK TrackListProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	switch(pMsgId) {
		case WM_CHAR:
		{
			int result = HandleTrackListChar(GetParent(pWindow), (UINT) pWParam);
			if(result == -1) {
				return 0;
			}
			break;
		}
		case WM_NCDESTROY:
			if(gsTrackListWndProc != NULL) {
				SetWindowLongPtr(pWindow, GWLP_WNDPROC, (LONG_PTR) gsTrackListWndProc);
			}
			break;
	}

	return CallWindowProc(gsTrackListWndProc, pWindow, pMsgId, pWParam, pLParam);
}

static BOOL CALLBACK TrackSelectCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;
	//int lCounter;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
		{
			PositionDialogToRightOfOwner(pWindow);

			HWND listBox = GetDlgItem(pWindow, IDC_LIST);
			if(listBox != NULL) {
				gsTrackListWndProc = (WNDPROC) SetWindowLongPtr(listBox, GWLP_WNDPROC,
					(LONG_PTR) TrackListProc);
			}

			SetDlgItemInt(pWindow, IDC_NB_LAP, gsNbLaps, FALSE);
			SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_SETCHECK,
				gsAllowWeapons ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_CANS, BM_SETCHECK,
				gsAllowCans ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_MINES, BM_SETCHECK,
				gsAllowMines ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT0, BM_SETCHECK,
				MR_IsCraftAllowed(gsAllowedCraftMask, 0) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT1, BM_SETCHECK,
				MR_IsCraftAllowed(gsAllowedCraftMask, 1) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT2, BM_SETCHECK,
				MR_IsCraftAllowed(gsAllowedCraftMask, 2) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT7, BM_SETCHECK,
				MR_IsCraftAllowed(gsAllowedCraftMask, 7) ? BST_CHECKED : BST_UNCHECKED, 0);
			PopulateRuleModeList(pWindow);
			SetSelectedRuleMode(pWindow, gsGameRuleSettings.mModeId);
			SetDlgItemInt(pWindow, IDC_RULE_OPACITY,
				(int) (gsGameRuleSettings.mRemoteCraftOpacity * 100.0f + 0.5f),
				FALSE);
			SetDlgItemInt(pWindow, IDC_RULE_NEAR_OPACITY,
				(int) (gsGameRuleSettings.mGhostNearOpacity * 100.0f + 0.5f),
				FALSE);
			SetDlgItemInt(pWindow, IDC_RULE_NEAR_DISTANCE,
				(int) (gsGameRuleSettings.mGhostNearDistance + 0.5f), FALSE);
			SetDlgItemInt(pWindow, IDC_RULE_FADE_DISTANCE,
				(int) (gsGameRuleSettings.mGhostFadeDistance + 0.5f), FALSE);
			SetDlgItemInt(pWindow, IDC_RULE_WAR_TARGET,
				gsGameRuleSettings.mWarTargetScore, FALSE);
			SetDlgItemInt(pWindow, IDC_RULE_WAR_WINBY,
				gsGameRuleSettings.mWarWinBy, FALSE);
			UpdateRuleControlState(pWindow);
			SendDlgItemMessage(pWindow, IDC_NB_LAP_SPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
			SetDlgItemText(pWindow, IDC_TRACK_FILTER, "");
			SendDlgItemMessage(pWindow, IDC_TRACK_FILTER, EM_SETCUEBANNER, FALSE,
				(LPARAM) L"Search");
			gsTrackSearchPrefix.clear();
			gsTrackFilter.clear();
			gsTrackSearchTick = 0;
			gsRemoteTrackList.clear();
			++gsRemoteSearchSerial;
			++gsRemotePreviewSerial;
			RefreshTrackList(pWindow);
			SetFocus(GetDlgItem(pWindow, IDC_TRACK_FILTER));
			lReturnValue = FALSE;
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDC_TRACK_FILTER:
					switch (HIWORD(pWParam)) {
						case EN_CHANGE:
						{
							char buffer[256];
							GetDlgItemText(pWindow, IDC_TRACK_FILTER, buffer, sizeof(buffer));
							gsTrackFilter = buffer;
							gsTrackSearchPrefix.clear();
							gsTrackSearchTick = 0;
							gsRemoteTrackList.clear();
							RefreshTrackList(pWindow);
							ScheduleRemoteTrackSearch(pWindow);
							break;
						}
					}
					break;
				case IDC_LIST:
					switch (HIWORD(pWParam)) {
						case LBN_SELCHANGE:
							gsTrackSearchPrefix.clear();
							gsTrackSearchTick = 0;
							gsSelectedEntry = SendDlgItemMessage(pWindow, IDC_LIST, LB_GETCURSEL, 0, 0);
							UpdateSelectedTrackInfo(pWindow);
							break;
					}
					break;
				case IDC_RULE_MODE:
					if(HIWORD(pWParam) == CBN_SELCHANGE) {
						UpdateRuleControlState(pWindow);
					}
					break;
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;
				case IDOK:
					if(gsSelectedEntry != -1) {
						if(!FinishRemoteTrackSelection(pWindow)) {
							lReturnValue = TRUE;
							break;
						}

						gsNbLaps = GetDlgItemInt(pWindow, IDC_NB_LAP, NULL, FALSE);
						gsAllowWeapons = (SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_GETCHECK, 0, 0) == BST_CHECKED);
						gsAllowCans = (SendDlgItemMessage(pWindow, IDC_TRACK_CANS, BM_GETCHECK, 0, 0) == BST_CHECKED);
						gsAllowMines = (SendDlgItemMessage(pWindow, IDC_TRACK_MINES, BM_GETCHECK, 0, 0) == BST_CHECKED);
						gsAllowedCraftMask = 0;
						if(SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT0, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							gsAllowedCraftMask |= (1u << 0);
						}
						if(SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT1, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							gsAllowedCraftMask |= (1u << 1);
						}
						if(SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT2, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							gsAllowedCraftMask |= (1u << 2);
						}
						if(SendDlgItemMessage(pWindow, IDC_TRACK_CRAFT7, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							gsAllowedCraftMask |= (1u << 7);
						}
						const int lWarTargetScore =
							GetDlgItemInt(pWindow, IDC_RULE_WAR_TARGET, NULL, FALSE);
						const int lWarWinBy =
							GetDlgItemInt(pWindow, IDC_RULE_WAR_WINBY, NULL, FALSE);

						gsGameRuleSettings.mModeId =
							(MR_GameRuleId) GetSelectedRuleMode(pWindow);
						gsGameRuleSettings.mRemoteCraftOpacity =
							GetDlgItemInt(pWindow, IDC_RULE_OPACITY, NULL, FALSE) / 100.0f;
						gsGameRuleSettings.mGhostNearOpacity =
							GetDlgItemInt(pWindow, IDC_RULE_NEAR_OPACITY, NULL, FALSE) / 100.0f;
						gsGameRuleSettings.mGhostNearDistance =
							(float) GetDlgItemInt(pWindow, IDC_RULE_NEAR_DISTANCE, NULL, FALSE);
						gsGameRuleSettings.mGhostFadeDistance =
							(float) GetDlgItemInt(pWindow, IDC_RULE_FADE_DISTANCE, NULL, FALSE);
						gsGameRuleSettings.mWarTargetScore = lWarTargetScore;
						gsGameRuleSettings.mWarWinBy = lWarWinBy;
						MR_NormalizeGameRuleSettings(gsGameRuleSettings);
						char lRuleError[128];

						if(gsNbLaps < 1)
							MessageBox(pWindow, MR_LoadString(IDS_LAP_RANGE), MR_LoadString(IDS_GAME_NAME), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						else if(!MR_HasAllowedCraft(gsAllowedCraftMask))
							MessageBox(pWindow, "At least one hovercraft must be enabled.", MR_LoadString(IDS_GAME_NAME), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						else if(!MR_ValidateGameRuleSettings(gsGameRuleSettings,
							lRuleError, sizeof(lRuleError)))
							MessageBox(pWindow, lRuleError, MR_LoadString(IDS_GAME_NAME), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						else
							EndDialog(pWindow, IDOK);
					}
					lReturnValue = TRUE;
					break;
			}
			break;
		case WM_TIMER:
			if(pWParam == REMOTE_TRACK_SEARCH_TIMER) {
				StartRemoteTrackSearch(pWindow);
				lReturnValue = TRUE;
			}
			break;
		case WM_DRAWITEM:
			if(pLParam != 0) {
				if(pWParam == IDC_TRACK_PREVIEW) {
					DrawTrackPreview((const DRAWITEMSTRUCT *) pLParam);
					lReturnValue = TRUE;
				}
				else if(pWParam == IDC_LIST) {
					DrawTrackListItem((const DRAWITEMSTRUCT *) pLParam);
					lReturnValue = TRUE;
				}
			}
			break;
		case WM_REMOTE_TRACK_SEARCH_COMPLETE:
			HandleRemoteTrackSearchComplete(pWindow, pLParam);
			lReturnValue = TRUE;
			break;
		case WM_REMOTE_TRACK_PREVIEW_COMPLETE:
			HandleRemoteTrackPreviewComplete(pWindow, pLParam);
			lReturnValue = TRUE;
			break;
		case WM_DESTROY:
			ClearRemoteTrackSearch(pWindow);
			break;
	}
	return lReturnValue;
}

/**
 * Check if a track is available locally.
 * @param pFileName The track name (without the ".trk" extension).
 * @return @c eTrackAvail if the track can be loaded,
 *         @c eTrackNotFound if the track is unavailable.
 */
MR_TrackAvail MR_GetTrackAvail(const char *pFileName)
{
	MR_TrackAvail lReturnValue = eTrackNotFound;

	std::string path = FindTrack(pFileName);

	if (!path.empty()) {
		MR_RecordFile lFile;

		if(!lFile.OpenForRead(path.c_str()))
			ASSERT(FALSE);
		else {
			TrackEntry lCurrentEntry;

			if(ReadTrackEntry(&lFile, &lCurrentEntry, pFileName))
				lReturnValue = eTrackAvail;
		}
	}
	return lReturnValue;
}

BOOL ReadTrackEntry(MR_RecordFile * pRecordFile, TrackEntry * pDest, const char *pFileName)
{
	BOOL lReturnValue = FALSE;
	int lMagicNumber;

	pRecordFile->SelectRecord(0);

	CArchive lArchive(pRecordFile, CArchive::load | CArchive::bNoFlushOnDelete);
	lArchive >> lMagicNumber;
	if(lMagicNumber == MR_MAGIC_TRACK_NUMBER) {
		int lVersion;

		lArchive >> lVersion;

		if(lVersion == 1) {
			int lMinorID;
			int lMajorID;

			CString cs;
			lArchive >> cs;
			pDest->mDescription = cs;
			lArchive >> lMinorID;
			lArchive >> lMajorID;

			BOOL lIDOk = FALSE;

			if((lMajorID != 0) && (pFileName != NULL)) {
				// Verify that filename fit with userID
				int lMinID = -1;
				int lMajID = -1;

				const char *lStr = strrchr(pFileName, '[');

				if(lStr != NULL) {
					sscanf(lStr + 1, "%d-%d", &lMajID, &lMinID);

					if((lMinID == lMinorID) && (lMajorID == lMajID))
						lIDOk = TRUE;
				}
			} else
			lIDOk = TRUE;

			if(lIDOk) {
				lArchive >> pDest->mSortingIndex;
				lArchive >> pDest->mRegistrationMode;

				if(pDest->mRegistrationMode == MR_FREE_TRACK) {
					lMagicNumber = 1;
					lArchive >> lMagicNumber;

					if(lMagicNumber == MR_MAGIC_TRACK_NUMBER) {
						lReturnValue = TRUE;
					}
				} else
				lReturnValue = TRUE;
			}
		}
	}
	return lReturnValue;
}

// Comparison function for SortList().
bool CompareFunc(const TrackEntry *ent1, const TrackEntry *ent2)
{
	return (*ent1) < (*ent2);
}

void ClearTrackPreview()
{
	gsTrackPreview.Reset();
}

void DrawTrackPreview(const DRAWITEMSTRUCT *pDrawItem)
{
	HDC lDc = pDrawItem->hDC;
	RECT lRect = pDrawItem->rcItem;
	RECT lInnerRect = lRect;
	HBRUSH lWindowBrush = (HBRUSH) (COLOR_WINDOW + 1);

	FillRect(lDc, &lRect, lWindowBrush);
	DrawEdge(lDc, &lRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(&lInnerRect, -2, -2);
	FillRect(lDc, &lInnerRect, lWindowBrush);

	if(gsTrackPreview.mBitmapHandle != NULL) {
		HDC lBitmapDc = CreateCompatibleDC(lDc);
		HBITMAP lOldBitmap = (HBITMAP) SelectObject(lBitmapDc,
			gsTrackPreview.mBitmapHandle);
		BITMAP lBitmap;

		memset(&lBitmap, 0, sizeof(lBitmap));
		GetObject(gsTrackPreview.mBitmapHandle, sizeof(lBitmap), &lBitmap);

		SetStretchBltMode(lDc, COLORONCOLOR);
		StretchBlt(lDc,
			lInnerRect.left, lInnerRect.top,
			lInnerRect.right - lInnerRect.left,
			lInnerRect.bottom - lInnerRect.top,
			lBitmapDc,
			0, 0,
			lBitmap.bmWidth,
			lBitmap.bmHeight,
			SRCCOPY);

		SelectObject(lBitmapDc, lOldBitmap);
		DeleteDC(lBitmapDc);
	}
	else if(!gsTrackPreview.mBitmap.empty()) {
		BITMAPINFO lBitmapInfo;

		memset(&lBitmapInfo, 0, sizeof(lBitmapInfo));
		lBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lBitmapInfo.bmiHeader.biWidth = gsTrackPreview.mWidth;
		lBitmapInfo.bmiHeader.biHeight = -gsTrackPreview.mHeight;
		lBitmapInfo.bmiHeader.biPlanes = 1;
		lBitmapInfo.bmiHeader.biBitCount = 32;
		lBitmapInfo.bmiHeader.biCompression = BI_RGB;

		SetStretchBltMode(lDc, gsTrackPreview.mSmoothScale ? HALFTONE : COLORONCOLOR);
		SetBrushOrgEx(lDc, 0, 0, NULL);
		StretchDIBits(lDc,
			lInnerRect.left, lInnerRect.top,
			lInnerRect.right - lInnerRect.left,
			lInnerRect.bottom - lInnerRect.top,
			0, 0,
			gsTrackPreview.mWidth,
			gsTrackPreview.mHeight,
			&gsTrackPreview.mBitmap[0],
			&lBitmapInfo,
			DIB_RGB_COLORS,
			SRCCOPY);
	}
	else {
		const char *lMessage =
			(gsSelectedEntry == -1) ? MR_LoadString(IDS_NO_SELECT) :
			gsTrackPreview.mLoading ? "Loading preview..." :
			"No preview available";

		SetBkMode(lDc, TRANSPARENT);
		SetTextColor(lDc, GetSysColor(COLOR_GRAYTEXT));
		DrawText(lDc, lMessage, -1, &lInnerRect,
			DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
	}
}

void DrawTrackListItem(const DRAWITEMSTRUCT *pDrawItem)
{
	if(pDrawItem->itemID == (UINT) -1) {
		return;
	}

	HDC lDc = pDrawItem->hDC;
	RECT lRect = pDrawItem->rcItem;
	bool selected = (pDrawItem->itemState & ODS_SELECTED) != 0;
	bool focused = (pDrawItem->itemState & ODS_FOCUS) != 0;
	TrackEntry *entry = NULL;

	if(pDrawItem->itemID < gsVisibleTrackList.size()) {
		entry = gsVisibleTrackList[pDrawItem->itemID];
	}

	COLORREF oldTextColor = SetTextColor(lDc,
		selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) :
		((entry != NULL) && entry->mIsRemote) ? GetSysColor(COLOR_GRAYTEXT) :
		GetSysColor(COLOR_WINDOWTEXT));
	COLORREF oldBkColor = SetBkColor(lDc,
		selected ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW));
	HBRUSH brush = CreateSolidBrush(
		selected ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW));

	FillRect(lDc, &lRect, brush);
	DeleteObject(brush);

	if(entry != NULL) {
		RECT textRect = lRect;
		InflateRect(&textRect, -2, 0);
		DrawText(lDc, entry->mFileName.c_str(), -1, &textRect,
			DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
	}

	if(focused) {
		DrawFocusRect(lDc, &lRect);
	}

	SetTextColor(lDc, oldTextColor);
	SetBkColor(lDc, oldBkColor);
}

COLORREF GetTrackPreviewColor(MR_UInt8 pColorIndex)
{
	if(!gsTrackPreviewPaletteInit) {
		InitTrackPreviewPalette();
	}

	if(pColorIndex < MR_NB_COLORS) {
		return gsTrackPreviewPalette[pColorIndex];
	}
	else {
		return TRACK_PREVIEW_BACKGROUND;
	}
}

void InitTrackPreviewPalette()
{
	int lCounter;
	double lGamma = 1.2;
	double lContrast = 0.95;
	double lBrightness = 0.95;
	MR_Config *lConfig = MR_Config::GetInstance();
	PALETTEENTRY *lOurEntries;

	if(lConfig != NULL) {
		lGamma = lConfig->video.gamma;
		lContrast = lConfig->video.contrast;
		lBrightness = lConfig->video.brightness;
	}

	for(lCounter = 0; lCounter < MR_NB_COLORS; lCounter++) {
		gsTrackPreviewPalette[lCounter] = TRACK_PREVIEW_BACKGROUND;
	}

	lOurEntries = MR_GetColors(
		1.0 / lGamma,
		lContrast * lBrightness,
		lBrightness - (lContrast * lBrightness));

	for(lCounter = 0;
		(lCounter < MR_BASIC_COLORS) &&
		((MR_RESERVED_COLORS_BEGINNING + lCounter) < MR_NB_COLORS);
		lCounter++)
	{
		gsTrackPreviewPalette[MR_RESERVED_COLORS_BEGINNING + lCounter] = RGB(
			lOurEntries[lCounter].peRed,
			lOurEntries[lCounter].peGreen,
			lOurEntries[lCounter].peBlue);
	}

	delete[]lOurEntries;
	gsTrackPreviewPaletteInit = true;
}

bool LoadTrackPreview(const TrackEntry &pEntry)
{
	const long lMaxPreviewPixels = 1024L * 1024L;
	MR_RecordFile lRecordFile;

	ClearTrackPreview();

	if(pEntry.mPath.empty()) {
		return false;
	}

	if(!lRecordFile.OpenForRead(pEntry.mPath.c_str())) {
		return false;
	}

	if(lRecordFile.GetNbRecords() <= TRACK_MAP_RECORD) {
		return false;
	}

	lRecordFile.SelectRecord(TRACK_MAP_RECORD);

	{
		CArchive lArchive(&lRecordFile, CArchive::load | CArchive::bNoFlushOnDelete);
		int lX0;
		int lX1;
		int lY0;
		int lY1;
		int lNbItem;
		int lItemHeight;
		int lTotalHeight;
		int lWidth;
		long lSourceSize;
		std::vector<MR_UInt8> lSource;

		lArchive >> lX0;
		lArchive >> lX1;
		lArchive >> lY0;
		lArchive >> lY1;
		lArchive >> lNbItem;
		lArchive >> lItemHeight;
		lArchive >> lTotalHeight;
		lArchive >> lWidth;

		if((lNbItem < 1) || (lItemHeight <= 0) || (lTotalHeight < lItemHeight) ||
			(lWidth <= 0))
		{
			return false;
		}

		lSourceSize = (long) lWidth * (long) lTotalHeight;
		if((lSourceSize <= 0) || (lSourceSize > lMaxPreviewPixels)) {
			return false;
		}

		lSource.resize((size_t) lSourceSize);
		if(lArchive.Read(&lSource[0], (UINT) lSourceSize) != (UINT) lSourceSize) {
			return false;
		}

		gsTrackPreview.mWidth = lWidth;
		gsTrackPreview.mHeight = lItemHeight;
		gsTrackPreview.mBitmap.resize((size_t) lWidth * (size_t) lItemHeight * 4, 0);

		for(int lY = 0; lY < lItemHeight; lY++) {
			for(int lX = 0; lX < lWidth; lX++) {
				MR_UInt8 lSourceColor = lSource[lY * lWidth + lX];
				COLORREF lColor = TRACK_PREVIEW_BACKGROUND;
				size_t lOffset = ((size_t) lY * (size_t) lWidth + (size_t) lX) * 4;

				if(lSourceColor != 0) {
					lColor = GetTrackPreviewColor(lSourceColor);
				}

				gsTrackPreview.mBitmap[lOffset + 0] = GetBValue(lColor);
				gsTrackPreview.mBitmap[lOffset + 1] = GetGValue(lColor);
				gsTrackPreview.mBitmap[lOffset + 2] = GetRValue(lColor);
				gsTrackPreview.mBitmap[lOffset + 3] = 0;
			}
		}
	}

	return true;
}

static void RemoteTrackSearchThread(HWND pWindow, unsigned serial,
	const std::string filter)
{
	RemoteTrackSearchPayload *payload = new RemoteTrackSearchPayload();
	payload->mSerial = serial;
	payload->mFilter = filter;

	try {
		std::string json;
		HoverRace::Net::Agent agent(std::string(REMOTE_TRACK_SEARCH_URL) +
			UrlEncode(filter));
		agent.Get(json);
		ParseRemoteTrackSearchJson(json, payload->mTracks);
	}
	catch(HoverRace::Net::NetExn&) {
		payload->mTracks.clear();
	}
	catch(std::exception&) {
		payload->mTracks.clear();
	}

	if(!PostMessage(pWindow, WM_REMOTE_TRACK_SEARCH_COMPLETE, 0,
		(LPARAM) payload))
	{
		delete payload;
	}
}

static void RemoteTrackPreviewThread(HWND pWindow, unsigned serial,
	const std::string url)
{
	RemoteTrackPreviewPayload *payload = new RemoteTrackPreviewPayload();
	payload->mSerial = serial;

	try {
		std::string data;
		HoverRace::Net::Agent agent(url);
		agent.Get(data);

		if(!data.empty()) {
			MR_GifDecoder decoder;
			if(decoder.Decode((const unsigned char *) data.data(), (int) data.length())) {
				HBITMAP bitmap = decoder.GetImage(0);
				if(bitmap != NULL) {
					CopyRemotePreviewBitmap(bitmap, payload->mPreview);
				}
			}
		}
	}
	catch(HoverRace::Net::NetExn&) {
	}
	catch(std::exception&) {
	}

	if(!PostMessage(pWindow, WM_REMOTE_TRACK_PREVIEW_COMPLETE, 0,
		(LPARAM) payload))
	{
		delete payload;
	}
}

bool CopyRemotePreviewBitmap(HBITMAP pBitmap, BitmapPixels &pPreview)
{
	BITMAP bitmap;
	HDC dc;
	BITMAPINFO bitmapInfo;

	memset(&bitmap, 0, sizeof(bitmap));
	if(GetObject(pBitmap, sizeof(bitmap), &bitmap) == 0) {
		return false;
	}
	if((bitmap.bmWidth <= 0) || (bitmap.bmHeight <= 0)) {
		return false;
	}

	pPreview.mWidth = bitmap.bmWidth;
	pPreview.mHeight = bitmap.bmHeight;
	pPreview.mBitmap.resize((size_t) pPreview.mWidth *
		(size_t) pPreview.mHeight * 4, 0);

	memset(&bitmapInfo, 0, sizeof(bitmapInfo));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = pPreview.mWidth;
	bitmapInfo.bmiHeader.biHeight = -pPreview.mHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	dc = GetDC(NULL);
	if(dc == NULL) {
		return false;
	}
	if(GetDIBits(dc, pBitmap, 0, pPreview.mHeight, &pPreview.mBitmap[0],
		&bitmapInfo, DIB_RGB_COLORS) == 0)
	{
		ReleaseDC(NULL, dc);
		pPreview.mBitmap.clear();
		pPreview.mWidth = 0;
		pPreview.mHeight = 0;
		return false;
	}
	ReleaseDC(NULL, dc);

	for(size_t offset = 0; offset < pPreview.mBitmap.size(); offset += 4) {
		MR_UInt8 blue = pPreview.mBitmap[offset + 0];
		MR_UInt8 green = pPreview.mBitmap[offset + 1];
		MR_UInt8 red = pPreview.mBitmap[offset + 2];
		if((red >= 248) && (green >= 248) && (blue >= 248)) {
			COLORREF bg = TRACK_PREVIEW_BACKGROUND;
			pPreview.mBitmap[offset + 0] = GetBValue(bg);
			pPreview.mBitmap[offset + 1] = GetGValue(bg);
			pPreview.mBitmap[offset + 2] = GetRValue(bg);
		}
		pPreview.mBitmap[offset + 3] = 0;
	}

	return true;
}

void LoadRemoteTrackPreview(HWND pWindow, const TrackEntry &pEntry)
{
	ClearTrackPreview();

	if(pEntry.mMapGifUrl.empty()) {
		return;
	}

	++gsRemotePreviewSerial;
	gsTrackPreview.mLoading = true;
	boost::thread(boost::bind(&RemoteTrackPreviewThread, pWindow,
		gsRemotePreviewSerial, pEntry.mMapGifUrl)).detach();
}

void ScheduleRemoteTrackSearch(HWND pWindow)
{
	std::string filter = TrimCopy(gsTrackFilter);
	KillTimer(pWindow, REMOTE_TRACK_SEARCH_TIMER);

	++gsRemotePreviewSerial;
	if(filter.length() < REMOTE_TRACK_SEARCH_MIN_CHARS) {
		++gsRemoteSearchSerial;
		gsRemoteTrackList.clear();
		RefreshTrackList(pWindow);
		return;
	}

	SetTimer(pWindow, REMOTE_TRACK_SEARCH_TIMER, REMOTE_TRACK_SEARCH_DELAY, NULL);
}

void ClearRemoteTrackSearch(HWND pWindow)
{
	KillTimer(pWindow, REMOTE_TRACK_SEARCH_TIMER);
	++gsRemoteSearchSerial;
	++gsRemotePreviewSerial;
	gsRemoteTrackList.clear();
}

void StartRemoteTrackSearch(HWND pWindow)
{
	std::string filter = TrimCopy(gsTrackFilter);
	KillTimer(pWindow, REMOTE_TRACK_SEARCH_TIMER);

	if(filter.length() < REMOTE_TRACK_SEARCH_MIN_CHARS) {
		return;
	}

	++gsRemoteSearchSerial;
	boost::thread(boost::bind(&RemoteTrackSearchThread, pWindow,
		gsRemoteSearchSerial, filter)).detach();
}

void HandleRemoteTrackSearchComplete(HWND pWindow, LPARAM pLParam)
{
	RemoteTrackSearchPayload *payload =
		reinterpret_cast<RemoteTrackSearchPayload *>(pLParam);
	if(payload == NULL) {
		return;
	}

	if((payload->mSerial == gsRemoteSearchSerial) &&
		(payload->mFilter == TrimCopy(gsTrackFilter)))
	{
		gsRemoteTrackList = payload->mTracks;
		RefreshTrackList(pWindow);
	}

	delete payload;
}

void HandleRemoteTrackPreviewComplete(HWND pWindow, LPARAM pLParam)
{
	RemoteTrackPreviewPayload *payload =
		reinterpret_cast<RemoteTrackPreviewPayload *>(pLParam);
	HWND lPreviewWindow = GetDlgItem(pWindow, IDC_TRACK_PREVIEW);

	if(payload == NULL) {
		return;
	}

	if(payload->mSerial == gsRemotePreviewSerial) {
		ClearTrackPreview();
		gsTrackPreview.mWidth = payload->mPreview.mWidth;
		gsTrackPreview.mHeight = payload->mPreview.mHeight;
		gsTrackPreview.mSmoothScale = true;
		gsTrackPreview.mBitmap.swap(payload->mPreview.mBitmap);
		if(lPreviewWindow != NULL) {
			InvalidateRect(lPreviewWindow, NULL, TRUE);
		}
	}

	delete payload;
}

void SortList()
{
	// Init pointer list
	if (!gsTrackList.empty()) {
		gsSortedTrackList.resize(gsTrackList.size(), NULL);
		for(unsigned int lCounter = 0; lCounter < gsTrackList.size(); lCounter++)
			gsSortedTrackList[lCounter] = &(gsTrackList[lCounter]);

		std::sort(gsSortedTrackList.begin(), gsSortedTrackList.end(), CompareFunc);
	}
}

/// Read the list of tracks from all search directories.
void ReadTrackList()
{
	CleanList();
	gsTrackList.reserve(INIT_TRACK_ENTRIES);

	for (int i = 0; i < NUM_TRACK_PATHS; ++i) {
		ReadTrackListDir(TRACK_PATHS[i]);
	}
	ReadTrackListDir(MR_Config::GetInstance()->GetTrackPath() + '\\');
}

/**
 * Read the list of tracks from a directory and add them to the global list.
 * @param dir The directory (must end with directory separator) (does not need to exist).
 */
void ReadTrackListDir(const std::string &dir)
{
	long lHandle;
	struct _finddata_t lFileInfo;

	lHandle = _findfirst((dir + "*" TRACK_EXT).c_str(), &lFileInfo);

	if(lHandle != -1) {
		if(lFileInfo.time_access == -1 || lFileInfo.time_create == -1 || lFileInfo.time_write == -1) {
			/* file times are botched, let's fix them */
			HANDLE file = CreateFile((dir + lFileInfo.name).c_str(),
										FILE_WRITE_ATTRIBUTES,
										FILE_SHARE_READ | FILE_SHARE_WRITE,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL);
			/* that was WAY too hard */
			FILETIME ft;
			SYSTEMTIME st;

			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &ft);

			if(SetFileTime(file, &ft, &ft, &ft) == 0) {
				/* error */
				ASSERT(FALSE);
			}

			CloseHandle(file);
		}

		do {
			gsTrackList.push_back(TrackEntry());
			TrackEntry &ent = gsTrackList.back();
			ent.mFileName = std::string(lFileInfo.name, 0, strlen(lFileInfo.name) - strlen(TRACK_EXT));
			ent.mPath = dir + lFileInfo.name;

			// Open the file and read additional info
			MR_RecordFile lRecordFile;

			if(!lRecordFile.OpenForRead(ent.mPath.c_str()))
				gsTrackList.pop_back();
			else {
				if(!ReadTrackEntry(&lRecordFile, &ent, NULL))
					gsTrackList.pop_back();
			}
		}
		while(_findnext(lHandle, &lFileInfo) == 0);

		_findclose(lHandle);
	}
}

/// Clear the track list.
void CleanList()
{
	ClearTrackPreview();
	gsTrackList.clear();
	gsRemoteTrackList.clear();
	gsSortedTrackList.clear();
	gsVisibleTrackList.clear();
	gsTrackSearchPrefix.clear();
	gsTrackFilter.clear();
	gsTrackSearchTick = 0;
	++gsRemoteSearchSerial;
	++gsRemotePreviewSerial;
}

void SetTrackDescriptionText(HWND pWindow, const char *pText)
{
	std::string text = (pText != NULL) ? pText : "";
	std::string normalized;

	for(size_t i = 0; i < text.length(); ++i) {
		if(text[i] == '\n') {
			if((i == 0) || (text[i - 1] != '\r')) {
				normalized += '\r';
			}
			normalized += '\n';
		}
		else {
			normalized += text[i];
		}
	}

	SetDlgItemText(pWindow, IDC_DESCRIPTION, normalized.c_str());
	SendDlgItemMessage(pWindow, IDC_DESCRIPTION, EM_SETSEL, 0, 0);
	SendDlgItemMessage(pWindow, IDC_DESCRIPTION, EM_SCROLLCARET, 0, 0);
	UpdateTrackDescriptionScrollbar(pWindow);
}

void UpdateTrackDescriptionScrollbar(HWND pWindow)
{
	HWND desc = GetDlgItem(pWindow, IDC_DESCRIPTION);
	if(desc == NULL) {
		return;
	}

	HDC dc = GetDC(desc);
	if(dc == NULL) {
		return;
	}

	TEXTMETRIC tm;
	memset(&tm, 0, sizeof(tm));
	GetTextMetrics(dc, &tm);
	ReleaseDC(desc, dc);

	RECT rect;
	GetClientRect(desc, &rect);

	const int lineHeight = (tm.tmHeight > 0) ? tm.tmHeight : 1;
	int visibleLines = (rect.bottom - rect.top) / lineHeight;
	if(visibleLines < 1) {
		visibleLines = 1;
	}
	const int lineCount = (int) SendMessage(desc, EM_GETLINECOUNT, 0, 0);

	ShowScrollBar(desc, SB_VERT, lineCount > visibleLines);
}

void UpdateSelectedTrackInfo(HWND pWindow)
{
	HWND lPreviewWindow;

	if (gsVisibleTrackList.empty() || (gsSelectedEntry == -1)) {
		SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
		SetTrackDescriptionText(pWindow, MR_LoadString(IDS_NO_SELECT));
		ClearTrackPreview();
	}
	else {
		SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
		SetTrackDescriptionText(pWindow,
			gsVisibleTrackList[gsSelectedEntry]->mDescription.c_str());
		if(gsVisibleTrackList[gsSelectedEntry]->mIsRemote) {
			LoadRemoteTrackPreview(pWindow, *gsVisibleTrackList[gsSelectedEntry]);
		}
		else {
			++gsRemotePreviewSerial;
			LoadTrackPreview(*gsVisibleTrackList[gsSelectedEntry]);
		}
	}

	lPreviewWindow = GetDlgItem(pWindow, IDC_TRACK_PREVIEW);
	if(lPreviewWindow != NULL) {
		InvalidateRect(lPreviewWindow, NULL, TRUE);
	}
}

/**
 * Search the directory list for a filename that matches a track name.
 * @param name The track name (may not be null).
 * @return The file path (may be relative).  Empty string if track not found.
 */
std::string FindTrack(const std::string &name)
{
	std::string path;

	// Check the configurable download dir first (the extension will be
	// added automatically).
	path = MR_Config::GetInstance()->GetTrackPath(name);
	if (_access(path.c_str(), 4) == 0) return path;

	for (int i = 0; i < NUM_TRACK_PATHS; ++i) {
		path = TRACK_PATHS[i];
		path += name;
		path += TRACK_EXT;
		if (_access(path.c_str(), 4) == 0) return path;
	}

	return "";
}

