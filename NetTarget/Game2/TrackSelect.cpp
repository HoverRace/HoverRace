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
#include "../Util/Cursor.h"
#include "resource.h"
#include "io.h"
#include "../MazeCompiler/TrackCommonStuff.h"
#include "../Util/StrRes.h"
#include "../Util/Config.h"
#include "../VideoServices/ColorPalette.h"

#include <algorithm>
#include <cctype>

namespace {
	const int TRACK_MAP_RECORD = 3;
	const COLORREF TRACK_PREVIEW_BACKGROUND = GetSysColor(COLOR_3DFACE);
	const DWORD TRACK_SEARCH_TIMEOUT = 1500;

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

		TrackPreviewData() : mWidth(0), mHeight(0) { }

		void Reset()
		{
			mWidth = 0;
			mHeight = 0;
			mBitmap.clear();
		}

		bool IsAvailable() const
		{
			return !mBitmap.empty();
		}
	};
}

class TrackEntry
{
	public:
		std::string mFileName;
		std::string mPath;
		std::string mDescription;
		int mRegistrationMode;
		int mSortingIndex;

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
static COLORREF GetTrackPreviewColor(MR_UInt8 pColorIndex);
static void InitTrackPreviewPalette();
static bool LoadTrackPreview(const TrackEntry &pEntry);
static LRESULT CALLBACK TrackListProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
static void SortList();
static void RebuildVisibleTrackList();
static void RefreshTrackList(HWND pWindow);
static void UpdateSelectedTrackInfo(HWND pWindow);
static int HandleTrackListChar(HWND pWindow, UINT pChar);
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
static sorted_t gsSortedTrackList;
static sorted_t gsVisibleTrackList;
static int gsNbLaps;
static BOOL gsAllowWeapons = FALSE;
static TrackPreviewData gsTrackPreview;
static COLORREF gsTrackPreviewPalette[MR_NB_COLORS];
static bool gsTrackPreviewPaletteInit = false;
static std::string gsTrackSearchPrefix;
static std::string gsTrackFilter;
static DWORD gsTrackSearchTick = 0;
static WNDPROC gsTrackListWndProc = NULL;

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
bool MR_SelectTrack(HWND pParentWindow, std::string &pTrackFile, int &pNbLap, bool &pAllowWeapons)
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
	gsAllowWeapons = false;

	if(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TRACK_SELECT), pParentWindow, TrackSelectCallBack) == IDOK) {
		pTrackFile = gsVisibleTrackList[gsSelectedEntry]->mFileName;
		pNbLap = gsNbLaps;
		pAllowWeapons = (gsAllowWeapons != FALSE);
		lReturnValue = true;
	} else
	lReturnValue = false;
	CleanList();

	return lReturnValue;
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
			SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_NB_LAP_SPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
			SetDlgItemText(pWindow, IDC_TRACK_FILTER, "");
			SendDlgItemMessage(pWindow, IDC_TRACK_FILTER, EM_SETCUEBANNER, FALSE,
				(LPARAM) L"Search");
			gsTrackSearchPrefix.clear();
			gsTrackFilter.clear();
			gsTrackSearchTick = 0;
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
							RefreshTrackList(pWindow);
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
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;
				case IDOK:
					if(gsSelectedEntry != -1) {
						gsNbLaps = GetDlgItemInt(pWindow, IDC_NB_LAP, NULL, FALSE);
						gsAllowWeapons = (SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_GETCHECK, 0, 0) == BST_CHECKED);

						if(gsNbLaps < 1)
							MessageBox(pWindow, MR_LoadString(IDS_LAP_RANGE), MR_LoadString(IDS_GAME_NAME), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						else
							EndDialog(pWindow, IDOK);
					}
					lReturnValue = TRUE;
					break;
			}
			break;
		case WM_DRAWITEM:
			if ((pWParam == IDC_TRACK_PREVIEW) && (pLParam != 0)) {
				DrawTrackPreview((const DRAWITEMSTRUCT *) pLParam);
				lReturnValue = TRUE;
			}
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

	if(gsTrackPreview.IsAvailable()) {
		BITMAPINFO lBitmapInfo;

		memset(&lBitmapInfo, 0, sizeof(lBitmapInfo));
		lBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lBitmapInfo.bmiHeader.biWidth = gsTrackPreview.mWidth;
		lBitmapInfo.bmiHeader.biHeight = -gsTrackPreview.mHeight;
		lBitmapInfo.bmiHeader.biPlanes = 1;
		lBitmapInfo.bmiHeader.biBitCount = 32;
		lBitmapInfo.bmiHeader.biCompression = BI_RGB;

		SetStretchBltMode(lDc, COLORONCOLOR);
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
			"No preview available";

		SetBkMode(lDc, TRANSPARENT);
		SetTextColor(lDc, GetSysColor(COLOR_GRAYTEXT));
		DrawText(lDc, lMessage, -1, &lInnerRect,
			DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
	}
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
	gsSortedTrackList.clear();
	gsVisibleTrackList.clear();
	gsTrackSearchPrefix.clear();
	gsTrackFilter.clear();
	gsTrackSearchTick = 0;
}

void UpdateSelectedTrackInfo(HWND pWindow)
{
	HWND lPreviewWindow;

	if (gsVisibleTrackList.empty() || (gsSelectedEntry == -1)) {
		SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
		SetDlgItemText(pWindow, IDC_DESCRIPTION, MR_LoadString(IDS_NO_SELECT));
		ClearTrackPreview();
	}
	else {
		SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
		SetDlgItemText(pWindow, IDC_DESCRIPTION,
			gsVisibleTrackList[gsSelectedEntry]->mDescription.c_str());
		LoadTrackPreview(*gsVisibleTrackList[gsSelectedEntry]);
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
