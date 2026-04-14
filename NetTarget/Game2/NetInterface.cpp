// NetInterface.cpp
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
#include <algorithm>
#include <stdarg.h>
#include <vector>

#include "NetInterface.h"
#include "TrackSelect.h"
#include "resource.h"
#include "../MazeCompiler/TrackCommonStuff.h"
#include "../Util/Config.h"
#include "../Util/StrRes.h"
#include "../VideoServices/ColorPalette.h"

// Private window messages
#define MRM_SERVER_CONNECT (WM_USER + 1)
#define MRM_NEW_CLIENT     (WM_USER + 2)
#define MRM_CLIENT         (WM_USER + 10)		  // 64 next reserved

#define MRNM_GET_GAME_NAME		40
#define MRNM_REMOVE_ENTRY		41
#define MRNM_GAME_NAME			42
#define MRNM_CONN_NAME_GET_SET	43
#define MRNM_CONN_NAME_SET		44
#define MRNM_CLIENT_ADDR_REQ	45
#define MRNM_CLIENT_ADDR		46
#define MRNM_LAG_TEST			47
#define MRNM_LAG_ANSWER			48
#define MRNM_LAG_INFO			49
#define MRNM_CONNECTION_DONE	50
#define MRNM_READY				51
#define MRNM_CANCEL_GAME		52
#define MRNM_SET_PLAYER_ID		53
#define MRNM_PARTY_INFO			58

#define STM_SERVER_CONNECT		54
#define STM_NEW_CLIENT			55
#define STM_CLIENT_CONNECT		56
#define STM_CLIENT_CLOSE		57

#define MR_CONNECTION_TIMEOUT   21000			  // 21 sec

#define MR_PING_RETRY_TIME		750
#define MR_CLIENT_RETRY_TIME	5000

// Local prototypes
static CString GetLocalAddrStr(BOOL lSteamOnly);
static MR_UInt32 GetAddrFromStr(const char *pName);
static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static bool IsInternetMeetingRoomWindow(HWND wnd);
static void PositionDialogToRightOfOwner(HWND dialog, int gap = 3);
static bool GetDialogWorkArea(HWND dialog, RECT &workArea);
static void ExpandTcpConnectionsDialogIfNeeded(HWND dialog, HWND listHandle);
static void ClearTcpTrackPreview();
static void DrawTcpTrackPreview(const DRAWITEMSTRUCT *pDrawItem);
static COLORREF GetTcpTrackPreviewColor(MR_UInt8 pColorIndex);
static void InitTcpTrackPreviewPalette();
static bool LoadTcpTrackPreview(const CString &trackName);
static CString GetTcpTrackPreviewTrackName(HWND dialog);
static CString FormatTcpServerAddrForDialog(const CString &serverAddr);
static void AdjustTcpServerAddrLayout(HWND dialog, const CString &serverAddrText);
static int GetTcpListRowForClient(const MR_NetworkInterface *iface, int clientIdx);
static int GetTcpListRowForClientPartyMember(const MR_NetworkInterface *iface,
	int clientIdx, int partyIdx);
static void UpdateTcpListRowsForClient(HWND listHandle,
	const MR_NetworkInterface *iface, int clientIdx);
static bool GetTcpImrTrackDetails(HWND dialog, CString &trackName, CString &lapText,
	CString &powerupsText, CString &craftsText);
static bool ParseTcpGameSummary(const CString &gameSummary, CString &trackName,
	CString &lapText, CString &powerupsText, CString &craftsText);
static bool GetTcpTrackDetails(HWND dialog, CString &trackName, CString &lapText,
	CString &powerupsText, CString &craftsText);
static void UpdateTcpDialogTrackSummary(HWND dialog);

namespace {
	const int TCP_TRACK_MAP_RECORD = 3;
	const COLORREF TCP_TRACK_PREVIEW_BACKGROUND = GetSysColor(COLOR_3DFACE);

	struct TcpTrackPreviewData
	{
		int mWidth;
		int mHeight;
		std::vector<MR_UInt8> mBitmap;

		TcpTrackPreviewData() : mWidth(0), mHeight(0) { }

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

MR_NetworkInterface *MR_NetworkInterface::mActiveInterface = NULL;
MR_NetMessageBuffer *MR_NetworkInterface::sBuffer = NULL;
CSteamID MR_NetworkInterface::sSteamID = CSteamID();
static TcpTrackPreviewData gsTcpTrackPreview;
static COLORREF gsTcpTrackPreviewPalette[MR_NB_COLORS];

static int GetTcpListRowForClient(const MR_NetworkInterface *iface, int clientIdx)
{
	return GetTcpListRowForClientPartyMember(iface, clientIdx, 0);
}

static int GetTcpListRowForClientPartyMember(const MR_NetworkInterface *iface,
	int clientIdx, int partyIdx)
{
	const int lLocalPartySize = (iface != NULL) ? iface->GetLocalPartySize() : 1;
	return lLocalPartySize + (clientIdx * MR_MAX_LOCAL_PLAYER) + partyIdx;
}

static void UpdateTcpListRowsForClient(HWND listHandle,
	const MR_NetworkInterface *iface, int clientIdx)
{
	if((listHandle == NULL) || (iface == NULL) ||
		(clientIdx < 0) || (clientIdx >= MR_NetworkInterface::eMaxClient)) {
		return;
	}

	const int lPartySize = max(1,
		min(iface->GetRemotePartySize(clientIdx), MR_MAX_LOCAL_PLAYER));
	char lLagText[64] = { 0 };
	char lStatusText[64] = { 0 };
	ListView_GetItemText(listHandle, GetTcpListRowForClient(iface, clientIdx), 1,
		lLagText, sizeof(lLagText));
	ListView_GetItemText(listHandle, GetTcpListRowForClient(iface, clientIdx), 2,
		lStatusText, sizeof(lStatusText));

	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		const int lRow = GetTcpListRowForClientPartyMember(iface, clientIdx, i);
		if(i < lPartySize) {
			ListView_SetItemText(listHandle, lRow, 0,
				(char *) iface->GetRemotePartyName(clientIdx, i));
			ListView_SetItemText(listHandle, lRow, 1, lLagText);
			ListView_SetItemText(listHandle, lRow, 2, lStatusText);
		}
		else {
			ListView_SetItemText(listHandle, lRow, 0, "");
			ListView_SetItemText(listHandle, lRow, 1, "");
			ListView_SetItemText(listHandle, lRow, 2, "");
		}
	}
}
static bool gsTcpTrackPreviewPaletteInit = false;
static HWND gsTcpTrackPreviewWindow = NULL;
static CString gsTcpOriginalGameName;
static CString gsTcpTrackName;
static CString gsTcpLapText;
static CString gsTcpPowerupsText;
static CString gsTcpCraftsText;

static void AppendHandshakeLog(const char *pFormat, ...)
{
	char lMessage[1024];
	va_list lArgs;
	va_start(lArgs, pFormat);
	_vsnprintf(lMessage, sizeof(lMessage) - 1, pFormat, lArgs);
	lMessage[sizeof(lMessage) - 1] = 0;
	va_end(lArgs);

	char lPath[MAX_PATH] = { 0 };
	DWORD lPathLen = GetTempPath(sizeof(lPath), lPath);
	if((lPathLen == 0) || (lPathLen >= sizeof(lPath))) {
		strcpy(lPath, ".\\");
	}
	strcat(lPath, "HoverRaceHandshake.log");

	FILE *lFile = fopen(lPath, "a");
	if(lFile != NULL) {
		fprintf(lFile, "%lu %s\n", (unsigned long) timeGetTime(), lMessage);
		fclose(lFile);
	}
}

static void PositionDialogToRightOfOwner(HWND dialog, int gap)
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
		if(x < ownerRect.left) {
			x = monitorInfo.rcWork.right - dialogWidth - gap;
		}
	}

	if((y + dialogHeight) > monitorInfo.rcWork.bottom) {
		y = monitorInfo.rcWork.bottom - dialogHeight;
	}
	if(y < monitorInfo.rcWork.top) {
		y = monitorInfo.rcWork.top;
	}

	SetWindowPos(dialog, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

static bool IsInternetMeetingRoomWindow(HWND wnd)
{
	return
		(GetDlgItem(wnd, IDC_GAME_LIST) != NULL) &&
		(GetDlgItem(wnd, IDC_USER_LIST) != NULL);
}

static bool GetDialogWorkArea(HWND dialog, RECT &workArea)
{
	HWND owner = GetWindow(dialog, GW_OWNER);
	if(owner == NULL) {
		owner = GetParent(dialog);
	}

	HMONITOR monitor = MonitorFromWindow(owner != NULL ? owner : dialog, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	if(!GetMonitorInfo(monitor, &monitorInfo)) {
		return !!SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	}

	workArea = monitorInfo.rcWork;
	return true;
}

static void ExpandTcpConnectionsDialogIfNeeded(HWND dialog, HWND listHandle)
{
	const int MIN_EXPANDED_DIALOG_WIDTH = 1200;
	const int MIN_PREVIEW_DIALOG_WIDTH = 1200;
	const int TARGET_VISIBLE_PLAYERS = 10;
	const int PREVIEW_MARGIN = 4;
	const int PREVIEW_TOP = 6;
	const int PREVIEW_LIST_GAP = 1;

	if(listHandle == NULL) {
		return;
	}

	RECT workArea;
	if(!GetDialogWorkArea(dialog, workArea)) {
		return;
	}

	if((workArea.right - workArea.left) < MIN_EXPANDED_DIALOG_WIDTH) {
		return;
	}

	int currentVisibleRows = ListView_GetCountPerPage(listHandle);
	if((currentVisibleRows <= 0) || (currentVisibleRows >= TARGET_VISIBLE_PLAYERS)) {
		return;
	}

	RECT itemRect;
	if(!ListView_GetItemRect(listHandle, 0, &itemRect, LVIR_BOUNDS)) {
		return;
	}

	const int rowHeight = itemRect.bottom - itemRect.top;
	if(rowHeight <= 0) {
		return;
	}

	const int extraHeight = (TARGET_VISIBLE_PLAYERS - currentVisibleRows) * rowHeight;
	if(extraHeight <= 0) {
		return;
	}

	RECT dialogRect;
	if(!GetWindowRect(dialog, &dialogRect)) {
		return;
	}

	if(((dialogRect.bottom - dialogRect.top) + extraHeight) > (workArea.bottom - workArea.top)) {
		return;
	}

	RECT listRect;
	if(!GetWindowRect(listHandle, &listRect)) {
		return;
	}
	MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&listRect), 2);

	int previewLayoutExtraHeight = 0;
	if((workArea.right - workArea.left) >= MIN_PREVIEW_DIALOG_WIDTH) {
		int previewSize = (listRect.bottom - listRect.top) + extraHeight;
		HWND craftsWindow = GetDlgItem(dialog, IDC_TCP_CRAFTS);
		if(craftsWindow != NULL) {
			RECT craftsRect;
			if(GetWindowRect(craftsWindow, &craftsRect)) {
				MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&craftsRect), 2);
				const int craftsBottom = craftsRect.bottom;
				if(craftsBottom > PREVIEW_TOP) {
					previewSize = min(previewSize, craftsBottom - PREVIEW_TOP);
				}
			}
		}

		const int previewBottom = PREVIEW_TOP + previewSize + PREVIEW_LIST_GAP;
		if(previewBottom > listRect.top) {
			previewLayoutExtraHeight = previewBottom - listRect.top;
		}
	}
	const int bottomControlShift = extraHeight + previewLayoutExtraHeight;

	for(HWND child = GetWindow(dialog, GW_CHILD); child != NULL; child = GetWindow(child, GW_HWNDNEXT)) {
		if(child == listHandle) {
			continue;
		}

		RECT childRect;
		if(!GetWindowRect(child, &childRect)) {
			continue;
		}
		MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&childRect), 2);

		if(childRect.top >= listRect.bottom) {
			MoveWindow(child, childRect.left, childRect.top + bottomControlShift,
				childRect.right - childRect.left, childRect.bottom - childRect.top, TRUE);
		}
	}

	MoveWindow(listHandle, listRect.left, listRect.top + previewLayoutExtraHeight,
		listRect.right - listRect.left, (listRect.bottom - listRect.top) + extraHeight, TRUE);

	SetWindowPos(dialog, NULL, 0, 0, dialogRect.right - dialogRect.left,
		(dialogRect.bottom - dialogRect.top) + bottomControlShift,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

	if((workArea.right - workArea.left) < MIN_PREVIEW_DIALOG_WIDTH) {
		return;
	}

	RECT updatedListRect;
	if(!GetWindowRect(listHandle, &updatedListRect)) {
		return;
	}
	MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&updatedListRect), 2);

	const int listHeight = updatedListRect.bottom - updatedListRect.top;
	int previewSize = listHeight;
	HWND craftsWindow = GetDlgItem(dialog, IDC_TCP_CRAFTS);
	if(craftsWindow != NULL) {
		RECT craftsRect;
		if(GetWindowRect(craftsWindow, &craftsRect)) {
			MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&craftsRect), 2);
			const int craftsBottom = craftsRect.bottom;
			if(craftsBottom > PREVIEW_TOP) {
				previewSize = min(previewSize, craftsBottom - PREVIEW_TOP);
			}
		}
	}
	if(previewSize <= 0) {
		return;
	}

	RECT updatedDialogRect;
	if(!GetWindowRect(dialog, &updatedDialogRect)) {
		return;
	}
	const int currentDialogWidth = updatedDialogRect.right - updatedDialogRect.left;
	if(currentDialogWidth > (workArea.right - workArea.left)) {
		return;
	}

	const int previewLeft = updatedListRect.right - previewSize;
	const int previewTop = PREVIEW_TOP;

	if(gsTcpTrackPreviewWindow == NULL) {
		gsTcpTrackPreviewWindow = CreateWindow("STATIC", "",
			WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
			previewLeft, previewTop, previewSize, previewSize,
			dialog, (HMENU) IDC_TCP_TRACK_PREVIEW, GetModuleHandle(NULL), NULL);
	}
	else {
		MoveWindow(gsTcpTrackPreviewWindow, previewLeft, previewTop,
			previewSize, previewSize, TRUE);
		ShowWindow(gsTcpTrackPreviewWindow, SW_SHOW);
	}

	const int textRight = previewLeft - PREVIEW_MARGIN;
	const int controlIds[] = {
		IDC_GAME_NAME, IDC_TCP_LAPS, IDC_TCP_WEAPONS, IDC_TCP_CRAFTS
	};

	for(size_t lIndex = 0; lIndex < sizeof(controlIds) / sizeof(controlIds[0]); lIndex++) {
		HWND textWindow = GetDlgItem(dialog, controlIds[lIndex]);
		if(textWindow != NULL) {
			RECT textRect;
			if(GetWindowRect(textWindow, &textRect)) {
				MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&textRect), 2);
				SetWindowLongPtr(textWindow, GWL_STYLE,
					GetWindowLongPtr(textWindow, GWL_STYLE) | SS_ENDELLIPSIS);
				MoveWindow(textWindow, textRect.left, textRect.top,
					textRight - textRect.left,
					textRect.bottom - textRect.top, TRUE);
			}
		}
	}

	HWND serverAddrWindow = GetDlgItem(dialog, IDC_SERVER_ADDR);
	if(serverAddrWindow != NULL) {
		RECT textRect;
		if(GetWindowRect(serverAddrWindow, &textRect)) {
			MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&textRect), 2);
			LONG_PTR textStyle = GetWindowLongPtr(serverAddrWindow, GWL_STYLE);
			textStyle &= ~SS_ENDELLIPSIS;
			SetWindowLongPtr(serverAddrWindow, GWL_STYLE, textStyle);
			MoveWindow(serverAddrWindow, textRect.left, textRect.top,
				textRight - textRect.left,
				textRect.bottom - textRect.top, TRUE);
		}
	}

}

static void ClearTcpTrackPreview()
{
	gsTcpTrackPreview.Reset();
}

static COLORREF GetTcpTrackPreviewColor(MR_UInt8 pColorIndex)
{
	if(!gsTcpTrackPreviewPaletteInit) {
		InitTcpTrackPreviewPalette();
	}

	if(pColorIndex < MR_NB_COLORS) {
		return gsTcpTrackPreviewPalette[pColorIndex];
	}
	else {
		return TCP_TRACK_PREVIEW_BACKGROUND;
	}
}

static void InitTcpTrackPreviewPalette()
{
	int lCounter;
	double lGamma = 1.2;
	double lContrast = 0.95;
	double lBrightness = 0.95;
	int lNbColors = MR_NB_COLORS - MR_RESERVED_COLORS_BEGINNING - MR_RESERVED_COLORS_END;
	MR_Config *lConfig = MR_Config::GetInstance();
	PALETTEENTRY *lOurEntries;

	if(lConfig != NULL) {
		lGamma = lConfig->video.gamma;
		lContrast = lConfig->video.contrast;
		lBrightness = lConfig->video.brightness;
	}

	lOurEntries = MR_GetColors(
		1.0 / lGamma,
		lContrast * lBrightness,
		lBrightness - (lContrast * lBrightness));

	for(lCounter = 0; lCounter < MR_NB_COLORS; lCounter++) {
		gsTcpTrackPreviewPalette[lCounter] = TCP_TRACK_PREVIEW_BACKGROUND;
	}

	for(lCounter = 0; lCounter < lNbColors; lCounter++) {
		gsTcpTrackPreviewPalette[MR_RESERVED_COLORS_BEGINNING + lCounter] = RGB(
			lOurEntries[lCounter].peRed,
			lOurEntries[lCounter].peGreen,
			lOurEntries[lCounter].peBlue);
	}

	delete[] lOurEntries;
	gsTcpTrackPreviewPaletteInit = true;
}

static bool LoadTcpTrackPreview(const CString &trackName)
{
	const long lMaxPreviewPixels = 1024L * 1024L;
	std::string lTrackName((const char *) trackName);
	MR_RecordFile *lTrackFile;
	bool lReturnValue = false;

	ClearTcpTrackPreview();

	if(lTrackName.empty()) {
		return false;
	}

	lTrackFile = MR_TrackOpen(NULL, lTrackName.c_str());
	if(lTrackFile == NULL) {
		return false;
	}

	if(lTrackFile->GetNbRecords() > TCP_TRACK_MAP_RECORD) {
		lTrackFile->SelectRecord(TCP_TRACK_MAP_RECORD);

		CArchive lArchive(lTrackFile, CArchive::load | CArchive::bNoFlushOnDelete);
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

		if((lNbItem >= 1) && (lItemHeight > 0) && (lTotalHeight >= lItemHeight) &&
			(lWidth > 0))
		{
			lSourceSize = (long) lWidth * (long) lTotalHeight;
			if((lSourceSize > 0) && (lSourceSize <= lMaxPreviewPixels)) {
				lSource.resize((size_t) lSourceSize);
				if(lArchive.Read(&lSource[0], (UINT) lSourceSize) == (UINT) lSourceSize) {
					gsTcpTrackPreview.mWidth = lWidth;
					gsTcpTrackPreview.mHeight = lItemHeight;
					gsTcpTrackPreview.mBitmap.resize(
						(size_t) lWidth * (size_t) lItemHeight * 4, 0);

					for(int lY = 0; lY < lItemHeight; lY++) {
						for(int lX = 0; lX < lWidth; lX++) {
							MR_UInt8 lSourceColor = lSource[lY * lWidth + lX];
							COLORREF lColor = TCP_TRACK_PREVIEW_BACKGROUND;
							size_t lOffset =
								((size_t) lY * (size_t) lWidth + (size_t) lX) * 4;

							if(lSourceColor != 0) {
								lColor = GetTcpTrackPreviewColor(lSourceColor);
							}

							gsTcpTrackPreview.mBitmap[lOffset + 0] = GetBValue(lColor);
							gsTcpTrackPreview.mBitmap[lOffset + 1] = GetGValue(lColor);
							gsTcpTrackPreview.mBitmap[lOffset + 2] = GetRValue(lColor);
							gsTcpTrackPreview.mBitmap[lOffset + 3] = 0;
						}
					}

					lReturnValue = true;
				}
			}
		}
	}

	delete lTrackFile;
	return lReturnValue;
}

static void DrawTcpTrackPreview(const DRAWITEMSTRUCT *pDrawItem)
{
	HDC lDc = pDrawItem->hDC;
	RECT lRect = pDrawItem->rcItem;
	RECT lInnerRect = lRect;
	HBRUSH lWindowBrush = (HBRUSH) (COLOR_WINDOW + 1);

	FillRect(lDc, &lRect, lWindowBrush);
	DrawEdge(lDc, &lRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(&lInnerRect, -1, -1);

	if(gsTcpTrackPreview.IsAvailable()) {
		BITMAPINFO lBitmapInfo;

		memset(&lBitmapInfo, 0, sizeof(lBitmapInfo));
		lBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lBitmapInfo.bmiHeader.biWidth = gsTcpTrackPreview.mWidth;
		lBitmapInfo.bmiHeader.biHeight = -gsTcpTrackPreview.mHeight;
		lBitmapInfo.bmiHeader.biPlanes = 1;
		lBitmapInfo.bmiHeader.biBitCount = 32;
		lBitmapInfo.bmiHeader.biCompression = BI_RGB;

		StretchDIBits(lDc,
			lInnerRect.left, lInnerRect.top,
			lInnerRect.right - lInnerRect.left,
			lInnerRect.bottom - lInnerRect.top,
			0, 0,
			gsTcpTrackPreview.mWidth,
			gsTcpTrackPreview.mHeight,
			&gsTcpTrackPreview.mBitmap[0],
			&lBitmapInfo,
			DIB_RGB_COLORS,
			SRCCOPY);
	}
	else {
		const char *lMessage = "Preview unavailable";

		SetBkMode(lDc, TRANSPARENT);
		SetTextColor(lDc, GetSysColor(COLOR_GRAYTEXT));
		DrawText(lDc, lMessage, -1, &lInnerRect,
			DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
	}
}

static CString GetTcpTrackPreviewTrackName(HWND dialog)
{
	char lTrackBuffer[256];
	CString lTrackName;
	CString lLapText;
	CString lPowerupsText;
	CString lCraftsText;

	if(GetTcpTrackDetails(dialog, lTrackName, lLapText, lPowerupsText,
		lCraftsText)) {
		return lTrackName;
	}

	GetDlgItemText(dialog, IDC_GAME_NAME, lTrackBuffer, sizeof(lTrackBuffer));
	lTrackName = lTrackBuffer;
	lTrackName.TrimLeft();
	lTrackName.TrimRight();

	int lInfoStart = lTrackName.Find("  ");
	if(lInfoStart > 0) {
		lTrackName = lTrackName.Left(lInfoStart);
		lTrackName.TrimRight();
	}

	return lTrackName;
}

static CString FormatTcpServerAddrForDialog(const CString &serverAddr)
{
	CString formatted(serverAddr);
	bool sawNewline = (formatted.Find('\n') >= 0) || (formatted.Find('\r') >= 0);

	if(sawNewline || formatted.IsEmpty() ||
		(formatted.CompareNoCase("Peer to Peer via Steam") == 0))
	{
		return formatted;
	}

	int separatorPos = formatted.Find(' ');
	if(separatorPos < 0) {
		return formatted;
	}

	while(separatorPos >= 0) {
		formatted.Delete(separatorPos);
		formatted.Insert(separatorPos, "\r\n");
		separatorPos = formatted.Find(' ', separatorPos + 2);
	}

	return formatted;
}

static void AdjustTcpServerAddrLayout(HWND dialog, const CString &serverAddrText)
{
	HWND serverAddrWindow = GetDlgItem(dialog, IDC_SERVER_ADDR);
	if(serverAddrWindow == NULL) {
		return;
	}

	int lineCount = 1;
	for(int lIndex = 0; lIndex < serverAddrText.GetLength(); lIndex++) {
		if(serverAddrText[lIndex] == '\n') {
			lineCount++;
		}
	}

	if(lineCount <= 1) {
		return;
	}

	RECT serverAddrRect;
	if(!GetWindowRect(serverAddrWindow, &serverAddrRect)) {
		return;
	}
	MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&serverAddrRect), 2);

	const int baseHeight = serverAddrRect.bottom - serverAddrRect.top;
	if(baseHeight <= 0) {
		return;
	}

	const int extraHeight = baseHeight * (lineCount - 1);
	const int originalBottom = serverAddrRect.bottom;

	MoveWindow(serverAddrWindow, serverAddrRect.left, serverAddrRect.top,
		serverAddrRect.right - serverAddrRect.left, baseHeight + extraHeight, TRUE);

	for(HWND child = GetWindow(dialog, GW_CHILD); child != NULL; child = GetWindow(child, GW_HWNDNEXT)) {
		if(child == serverAddrWindow) {
			continue;
		}

		RECT childRect;
		if(!GetWindowRect(child, &childRect)) {
			continue;
		}
		MapWindowPoints(NULL, dialog, reinterpret_cast<LPPOINT>(&childRect), 2);

		if(childRect.top >= originalBottom) {
			MoveWindow(child, childRect.left, childRect.top + extraHeight,
				childRect.right - childRect.left, childRect.bottom - childRect.top, TRUE);
		}
	}

	RECT dialogRect;
	if(GetWindowRect(dialog, &dialogRect)) {
		SetWindowPos(dialog, NULL, 0, 0,
			dialogRect.right - dialogRect.left,
			(dialogRect.bottom - dialogRect.top) + extraHeight,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	}
}

static bool ParseTcpGameSummary(const CString &gameSummary, CString &trackName,
	CString &lapText, CString &powerupsText, CString &craftsText)
{
	std::string summary((const char *) gameSummary);
	std::string::size_type lapsPos;
	std::string::size_type numberStart;
	const std::string craftsTag = " crafts ";
	bool allowWeapons = false;
	bool allowCans = false;
	bool allowMines = false;

	trackName.Empty();
	lapText.Empty();
	powerupsText.Empty();
	craftsText.Empty();

	while(!summary.empty() && (summary[summary.length() - 1] == ' ')) {
		summary.erase(summary.length() - 1);
	}

	if(summary.empty()) {
		return false;
	}

	const std::string minesOn = " mines on";
	const std::string minesOff = " mines off";
	const std::string cansOn = " cans on";
	const std::string cansOff = " cans off";
	const std::string withWeapons = " with weapons";
	const std::string noWeapons = " no weapons";
	const std::string::size_type craftsPos = summary.rfind(craftsTag);

	if((craftsPos != std::string::npos) &&
		(craftsPos + craftsTag.length() < summary.length()))
	{
		craftsText = MR_FormatAllowedCraftDisplayMask(MR_ParseAllowedCraftMask(
			summary.c_str() + craftsPos + craftsTag.length())).c_str();
		summary.erase(craftsPos);
	}

	MR_GameRuleSettings lRuleSettings;
	MR_ParseGameRuleSummary(summary, lRuleSettings);

	if(summary.length() > minesOn.length() &&
		summary.compare(summary.length() - minesOn.length(), minesOn.length(), minesOn) == 0)
	{
		allowMines = true;
		summary.erase(summary.length() - minesOn.length());
	}
	else if(summary.length() > minesOff.length() &&
		summary.compare(summary.length() - minesOff.length(), minesOff.length(), minesOff) == 0)
	{
		summary.erase(summary.length() - minesOff.length());
	}

	if(summary.length() > cansOn.length() &&
		summary.compare(summary.length() - cansOn.length(), cansOn.length(), cansOn) == 0)
	{
		allowCans = true;
		summary.erase(summary.length() - cansOn.length());
	}
	else if(summary.length() > cansOff.length() &&
		summary.compare(summary.length() - cansOff.length(), cansOff.length(), cansOff) == 0)
	{
		summary.erase(summary.length() - cansOff.length());
	}

	if(summary.length() > withWeapons.length() &&
		summary.compare(summary.length() - withWeapons.length(), withWeapons.length(), withWeapons) == 0)
	{
		allowWeapons = true;
		summary.erase(summary.length() - withWeapons.length());
	}
	else if(summary.length() > noWeapons.length() &&
		summary.compare(summary.length() - noWeapons.length(), noWeapons.length(), noWeapons) == 0)
	{
		summary.erase(summary.length() - noWeapons.length());
	}

	powerupsText = MR_GetGameRuleDisplayName(lRuleSettings.mModeId);
	powerupsText += " / ";
	powerupsText += MR_FormatPowerupDisplay(allowWeapons, allowCans, allowMines).c_str();

	lapsPos = summary.rfind(" laps");
	if(lapsPos == std::string::npos) {
		lapsPos = summary.rfind(" lap");
	}

	if(lapsPos != std::string::npos) {
		numberStart = summary.rfind(' ', lapsPos - 1);
		if((numberStart != std::string::npos) && (numberStart + 1 < lapsPos)) {
			lapText = summary.substr(numberStart + 1, lapsPos - numberStart - 1).c_str();
			trackName = summary.substr(0, numberStart).c_str();
			trackName.TrimRight();
			return !trackName.IsEmpty();
		}
	}

	trackName = summary.c_str();
	return !trackName.IsEmpty();
}

static bool GetTcpImrTrackDetails(HWND dialog, CString &trackName, CString &lapText,
	CString &powerupsText, CString &craftsText)
{
	char lTrackBuffer[256];
	char lLapBuffer[64];
	char lPowerupsBuffer[256];
	char lCraftsBuffer[64];
	HWND owner = GetWindow(dialog, GW_OWNER);

	lCraftsBuffer[0] = 0;

	trackName.Empty();
	lapText.Empty();
	powerupsText.Empty();
	craftsText.Empty();

	if(owner == NULL) {
		owner = GetParent(dialog);
	}

	if((owner == NULL) || !IsInternetMeetingRoomWindow(owner)) {
		return false;
	}

	GetDlgItemText(owner, IDC_TRACK_NAME, lTrackBuffer, sizeof(lTrackBuffer));
	GetDlgItemText(owner, IDC_NB_LAP, lLapBuffer, sizeof(lLapBuffer));
	GetDlgItemText(owner, IDC_WEAPONS, lPowerupsBuffer, sizeof(lPowerupsBuffer));
	GetDlgItemText(owner, IDC_CRAFTS, lCraftsBuffer, sizeof(lCraftsBuffer));

	trackName = lTrackBuffer;
	lapText = lLapBuffer;
	powerupsText = lPowerupsBuffer;
	craftsText = lCraftsBuffer;

	trackName.TrimLeft();
	trackName.TrimRight();
	lapText.TrimLeft();
	lapText.TrimRight();
	powerupsText.TrimLeft();
	powerupsText.TrimRight();
	craftsText.TrimLeft();
	craftsText.TrimRight();

	if(trackName.IsEmpty() || (trackName == MR_LoadString(IDS_IMR_NOSELECT))) {
		return false;
	}

	return true;
}

static bool GetTcpTrackDetails(HWND dialog, CString &trackName, CString &lapText,
	CString &powerupsText, CString &craftsText)
{
	if(GetTcpImrTrackDetails(dialog, trackName, lapText, powerupsText, craftsText)) {
		return true;
	}

	trackName = gsTcpTrackName;
	lapText = gsTcpLapText;
	powerupsText = gsTcpPowerupsText;
	craftsText = gsTcpCraftsText;

	if(!trackName.IsEmpty()) {
		return true;
	}

	if(ParseTcpGameSummary(gsTcpOriginalGameName, trackName, lapText, powerupsText,
		craftsText))
	{
		return true;
	}

	return false;
}

static void UpdateTcpDialogTrackSummary(HWND dialog)
{
	CString lTrackName;
	CString lLapText;
	CString lPowerupsText;
	CString lCraftsText;

	if(GetTcpTrackDetails(dialog, lTrackName, lLapText, lPowerupsText,
		lCraftsText)) {
		SetDlgItemText(dialog, IDC_GAME_NAME, lTrackName);
		SetDlgItemText(dialog, IDC_TCP_LAPS, lLapText);
		SetDlgItemText(dialog, IDC_TCP_WEAPONS, lPowerupsText);
		SetDlgItemText(dialog, IDC_TCP_CRAFTS, lCraftsText);
	}
	else {
		SetDlgItemText(dialog, IDC_GAME_NAME, gsTcpOriginalGameName);
		SetDlgItemText(dialog, IDC_TCP_LAPS, "");
		SetDlgItemText(dialog, IDC_TCP_WEAPONS, "");
		SetDlgItemText(dialog, IDC_TCP_CRAFTS, "");
	}
}

/**
 * Set up the MR_NetworkInterface.  Set up the client list and information, create and initialize both UDPOut ports, set them as non-blocking, and
 * set the player name (mPlayer) correctly.
 */
MR_NetworkInterface::MR_NetworkInterface()
{
	ASSERT(MR_NET_HEADER_LEN == 5);

	WORD lVersionRequested = MAKEWORD(1, 1);
	WSADATA lWsaData;

	if(WSAStartup(lVersionRequested, &lWsaData))
		ASSERT(FALSE);

	MR_Config *cfg = MR_Config::GetInstance();
	mPlayer = (cfg != NULL && !cfg->player.nickName.empty()) ?
		cfg->player.nickName.c_str() : "Player";
	mLocalPartySize = 1;
	mLocalPartyNames[0] = mPlayer;
	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mLocalPartyNames[i] = "";
	}

	mId = MR_ID_NOT_SET; // this denotes that it has not been set yet
	mServerMode = FALSE;
	mRegistrySocket = INVALID_SOCKET;
	mServerPort = 0;
	mSteamID = CSteamID();
	mSteamOnly = !MR_Config::GetInstance()->misc.directConnect;
	mTrackName = "";
	mNbLap = -1;
	mHasGameLaps = FALSE;
	mAllowWeapons = FALSE;
	mHasGameWeapons = FALSE;
	mAllowCans = FALSE;
	mHasGameCans = FALSE;
	mAllowMines = FALSE;
	mHasGameMines = FALSE;
	mAllowedCraftMask = MR_GetDefaultAllowedCraftMask();
	mHasGameCrafts = FALSE;
	mGameRuleSettings = MR_GameRuleSettings();

	mAllPreLoguedRecv = FALSE;
	mWaitGameNameConnected = FALSE;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		mPreLoguedClient[lCounter] = FALSE;
		mConnected[lCounter] = FALSE;
		mCanBePreLogued[lCounter] = FALSE;
		mClientPartySize[lCounter] = 1;
		mClientPartyNames[lCounter][0] = "";
		for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
			mClientPartyNames[lCounter][i] = "";
		}
	}

	// Init the UDP Output ports
	mUDPOutShortPort = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPOutShortPort != INVALID_SOCKET);

	mUDPOutLongPort = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPOutLongPort != INVALID_SOCKET);

	// Set these port as non-blocking
	int lCode;
	unsigned long lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPOutShortPort, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPOutLongPort, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// reduce buffer size to avoid data accumulation
	int lQueueSize = 120;
	lCode = setsockopt(mUDPOutShortPort, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));
	ASSERT(lCode != SOCKET_ERROR);

	lQueueSize = 800;
	lCode = setsockopt(mUDPOutLongPort, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));
	ASSERT(lCode != SOCKET_ERROR);

	// Bind to any avail addr
	SOCKADDR_IN lAddr;

	lAddr.sin_family = AF_INET;
	lAddr.sin_addr.s_addr = INADDR_ANY;
	lAddr.sin_port = 0;

	lCode = bind(mUDPOutShortPort, (LPSOCKADDR) & lAddr, sizeof(lAddr));
	ASSERT(lCode != SOCKET_ERROR);

	mUDPRecvPort = MR_Config::GetInstance()->net.udpRecvPort;
	mTCPRecvPort = MR_Config::GetInstance()->net.tcpRecvPort;
}

/**
 * Destroy the MR_NetworkInterface.  Close the UDP out sockets, disconnect, and call WSACleanup() to clean up WSA-related cruft.
 */
MR_NetworkInterface::~MR_NetworkInterface()
{
	if(mUDPOutShortPort != INVALID_SOCKET)
		closesocket(mUDPOutShortPort);

	if(mUDPOutLongPort != INVALID_SOCKET)
		closesocket(mUDPOutLongPort);

	Disconnect();
	WSACleanup();
}

/**
 * Get the player name of the specified index (pIndex).
 *
 * @param pIndex The index of the player whose name we are seeking
 */
const char *MR_NetworkInterface::GetPlayerName(int pIndex) const
{
	const char *lReturnValue = NULL;

	if(pIndex < 0)
		lReturnValue = mPlayer;
	else {
		if(pIndex < eMaxClient)
			lReturnValue = mClientName[pIndex];
	}
	return lReturnValue;
}

/**
 * Check if we are connected to the client with the specified index (pIndex).
 *
 * @param pIndex The index of the player whose connection status we are seeking
 */
BOOL MR_NetworkInterface::IsConnected(int pIndex) const
{
	ASSERT(pIndex < eMaxClient);

	if(pIndex == -1)
		return TRUE;
	else
		return mClient[pIndex].IsConnected();
}

void MR_NetworkInterface::CleanupClientState(int pClient, HWND pWindow)
{
	if((pClient < 0) || (pClient >= eMaxClient)) {
		return;
	}

	mClient[pClient].DisconnectSteam();
	mClient[pClient].Disconnect();
	mCanBePreLogued[pClient] = FALSE;
	mPreLoguedClient[pClient] = FALSE;
	mConnected[pClient] = FALSE;
	mClientPartySize[pClient] = 1;
	mClientPartyNames[pClient][0] = "";
	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mClientPartyNames[pClient][i] = "";
	}

	if((pWindow != NULL) && IsWindow(pWindow)) {
		HWND lListHandle = GetDlgItem(pWindow, IDC_LIST);

		if(lListHandle != NULL) {
			for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
				const int lRow = GetTcpListRowForClientPartyMember(this, pClient, i);
				ListView_SetItemText(lListHandle, lRow, 0, "");
				ListView_SetItemText(lListHandle, lRow, 1, "");
				ListView_SetItemText(lListHandle, lRow, 2,
					(i == 0) ? (char *) MR_LoadStringBuffered(IDS_DISCONNECTED) : "");
			}
		}
	}
}

int MR_NetworkInterface::GetRemotePartySize(int pClient) const
{
	if((pClient < 0) || (pClient >= eMaxClient)) {
		return 0;
	}

	return max(1, mClientPartySize[pClient]);
}

const char *MR_NetworkInterface::GetRemotePartyName(int pClient, int pPartyIndex) const
{
	if((pClient < 0) || (pClient >= eMaxClient)) {
		return "";
	}
	if((pPartyIndex < 0) || (pPartyIndex >= GetRemotePartySize(pClient))) {
		return "";
	}

	if((pPartyIndex == 0) && mClientPartyNames[pClient][0].IsEmpty()) {
		return mClientName[pClient];
	}

	return mClientPartyNames[pClient][pPartyIndex];
}

int MR_NetworkInterface::GetMachineIdForClient(int pClient) const
{
	if((pClient < 0) || (pClient >= eMaxClient)) {
		return MR_ID_NOT_SET;
	}

	return (pClient >= mId) ? (pClient + 1) : pClient;
}

int MR_NetworkInterface::GetClientForMachineId(int pMachineId) const
{
	if((pMachineId < 0) || (pMachineId > eMaxClient) || (pMachineId == mId)) {
		return -1;
	}

	return (pMachineId > mId) ? (pMachineId - 1) : pMachineId;
}

void MR_NetworkInterface::NotifyClientRemoved(int pClient)
{
	if(!mServerMode || (pClient < 0) || (pClient >= eMaxClient)) {
		return;
	}

	MR_NetMessageBuffer lAnswer;
	lAnswer.mMessageType = MRNM_REMOVE_ENTRY;
	lAnswer.mClient = mId;
	lAnswer.mDataLen = 1;
	lAnswer.mData[0] = pClient + 1;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if((lCounter != pClient) && mClient[lCounter].IsConnected()) {
			mClient[lCounter].Send(&lAnswer, MR_NET_REQUIRED);
		}
	}
}

/**
 * Close the registry socket and reset all variables to their "disconnected" state.
 */
void MR_NetworkInterface::Disconnect(BOOL pDisconnectSteam)
{
	if(mRegistrySocket != INVALID_SOCKET) {
		closesocket(mRegistrySocket);
		mRegistrySocket = INVALID_SOCKET;
	}
	mServerMode = FALSE;
	mId = MR_ID_NOT_SET;
	mServerPort = 0;
	mServerAddr = "";
	mGameName = "";
	mTrackName = "";
	mNbLap = -1;
	mHasGameLaps = FALSE;
	mAllowWeapons = FALSE;
	mHasGameWeapons = FALSE;
	mAllowCans = FALSE;
	mHasGameCans = FALSE;
	mAllowMines = FALSE;
	mHasGameMines = FALSE;
	mAllowedCraftMask = MR_GetDefaultAllowedCraftMask();
	mHasGameCrafts = FALSE;
	mGameRuleSettings = MR_GameRuleSettings();

	sSteamID = CSteamID();
	sBuffer = NULL;

	mAllPreLoguedRecv = FALSE;
	mWaitGameNameConnected = FALSE;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		mClient[lCounter].DisconnectSteam(pDisconnectSteam);
		mClient[lCounter].Disconnect();
		mPreLoguedClient[lCounter] = FALSE;
		mConnected[lCounter] = FALSE;
		mCanBePreLogued[lCounter] = FALSE;
		mClientPartySize[lCounter] = 1;
		mClientPartyNames[lCounter][0] = "";
		for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
			mClientPartyNames[lCounter][i] = "";
		}
	}

	// disconnect UDP recv socket
	if(mUDPRecvSocket != INVALID_SOCKET) {
		closesocket(mUDPRecvSocket);
		mUDPRecvSocket = INVALID_SOCKET;
	}
}

/**
 * Returns the number of connected clients (checks with IsConnected()).
 */
int MR_NetworkInterface::GetClientCount() const
{
	int lReturnValue = 0;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if(mClient[lCounter].IsConnected())
			lReturnValue++;
	} return lReturnValue;
}

/**
 * Returns the player id of this interface ("what player are we?").
 */
void MR_NetworkInterface::SetId(const int id)
{
	mId = id;
}

/**
 * Returns the player id of this interface ("what player are we?").
 */
int MR_NetworkInterface::GetId() const
{
	ASSERT((mId != 0) || mServerMode);
	return mId;
}

/**
 * Returns the player Steam ID.
 */
CSteamID MR_NetworkInterface::GetSteamId() const
{
	return mSteamID;
}

/**
 * Returns the player Steam ID.
 */
BOOL MR_NetworkInterface::GetSteamOnly() const
{
	return mSteamOnly;
}

/**
 * Returns the calculated lag to the server.
 */
int MR_NetworkInterface::GetLagFromServer() const
{
	if(mServerMode)
		return 0;
	else
		return mClient[0].GetMinLag();
}

/**
 * Create the UDP receive port, set the port to be non-blocking, and bind it.  Returns TRUE on success (and FALSE on error).
 *
 * @param pPort The port to use
 */
BOOL MR_NetworkInterface::CreateUDPRecvSocket(int pPort)
{
	// this assert can be removed; early debugging artifact
	ASSERT(mUDPRecvSocket == INVALID_SOCKET);

	mUDPRecvSocket = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPRecvSocket != INVALID_SOCKET);

	// make it non-blocking
	int lCode;
	unsigned long lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPRecvSocket, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// Bind the socket to the default port
	SOCKADDR_IN lLocalAddr;
	lLocalAddr.sin_family = AF_INET;
	lLocalAddr.sin_addr.s_addr = INADDR_ANY;
	lLocalAddr.sin_port = pPort;
	lCode = bind(mUDPRecvSocket, (LPSOCKADDR) &lLocalAddr, sizeof(lLocalAddr));
	ASSERT(lCode != SOCKET_ERROR);

	// bind() success code is 0
	return (lCode == 0);
}

/**
 * Return the smallest lag sample to the given client.
 *
 * @param pClient Index of the client
 */
int MR_NetworkInterface::GetMinLag(int pClient) const
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	return mClient[pClient].GetMinLag();
}

/**
 * Unlike its name says, this actually does the exact same thing as GetMinLag().  It returns the minimum, not the average like its name implies.  Hooray
 * for lying!
 *
 * @param pClient Index of the client
 */
int MR_NetworkInterface::GetAvgLag(int pClient) const
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	return mClient[pClient].GetMinLag();
}

/**
 * Send a message to the given client.
 *
 * @param pClient Index of the client
 * @param pMessage MR_NetMessageBuffer structure containing the message to be sent
 * @param pLongPort TRUE uses the UDP out long port, FALSE uses the UDP out short port
 * @param pResendLast Should be set to TRUE if we are resending pMessage
 */
BOOL MR_NetworkInterface::UDPSend(int pClient, MR_NetMessageBuffer *pMessage, BOOL pLongPort, BOOL pResendLast)
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	pMessage->mClient = mId;
	return mClient[pClient].UDPSend(pLongPort ? mUDPOutLongPort : mUDPOutShortPort, pMessage, pLongPort ? 0 : 1, pResendLast, mId);
}

/**
 * Send a message to all clients.  Assumes long port for UDP out and that the message is not being resent.
 *
 * @param pMessage MR_NetMessageBuffer structure containing the message to be sent
 * @param pReqLevel If set to MR_NET_DATAGRAM UDP will be used; otherwise, TCP will be used
 */
BOOL MR_NetworkInterface::BroadcastMessage(MR_NetMessageBuffer *pMessage, int pReqLevel)
{
	pMessage->mClient = mId; // must ensure this
	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if(pReqLevel == MR_NET_DATAGRAM)
			mClient[lCounter].UDPSend(mUDPOutLongPort, pMessage, 0, FALSE, mId);
		else
			mClient[lCounter].Send(pMessage, pReqLevel, mId + eMaxClient + 1);
	}
	return TRUE;
}

/*
BOOL MR_NetworkInterface::BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage )
{
	MR_NetMessageBuffer lMessage;

	lMessage.mSendingTime = pTimeStamp;
	lMessage.mMessageType = pMessageType;
	lMessage.mDataLen     = pMessageLen;

	if( pMessageLen > 0 )
	{
		memcpy( lMessage.mData, pMessage, pMessageLen );
	}

	return BroadcastMessage( &lMessage );

}
*/

/**
 * Fetches a message out of the queue.  Writes the message's data into the references passed as parameters.
 *
 * @param pTimeStamp Timestamp of the message
 * @param pMessageType ID type of the message
 * @param pMessageLen Length of the message
 * @param pMessage MR_UInt8 buffer containing the message
 * @param pClientId ID of the client
 */
BOOL MR_NetworkInterface::FetchMessage(DWORD &pTimeStamp, int &pMessageType, int &pMessageLen, const MR_UInt8 *&pMessage, int &pClientId)
{
	BOOL lReturnValue = FALSE;
	static int sLastClient = 0;
	
	for(int lCounter = 0; !lReturnValue && (lCounter < eMaxClient); lCounter++) {
		int lClient = (lCounter + sLastClient + 1) % eMaxClient;
		const MR_NetMessageBuffer *lMessage;

		lMessage = mClient[lClient].Poll((lClient >= mId) ? lClient + 1 : lClient, TRUE);

		if(lMessage != NULL) {
			lReturnValue = TRUE;
			sLastClient = lMessage->mClient;

			if(&pMessage != NULL) {
				pMessage = lMessage->mData;
			}
			pMessageLen = lMessage->mDataLen;
			pMessageType = lMessage->mMessageType;

			/*
			   DWORD lOtherSideEval = pTimeStamp-mClient[ lClient ].GetLag();

			   DWORD lOtherSideTime = lOtherSideEval&~4095 + lMessage->mSendingTime<<2;

			   int lDiff = lOtherSideTime-lOtherSideEval;

			   if( lDiff > 2048 )
			   {
			   lOtherSideTime -= 4096;
			   }
			   else if(lDiff < -2048 )
			   {
			   lOtherSideTime += 4096;
			   }

			   pTimeStamp = lOtherSideTime;
			 */
			pTimeStamp = 0;

			pClientId = lMessage->mClient;

			// We need to modify pClientId.  If we are player #1, player #2 should be client 0, player #3 should be client 1,
			// and so on.  If we are player #2, player #1 is client 0, player #3 is client 1, and so on.
			// What this boils down to is that if pClientId is greater than our own ID, we have to decrement it
			if(pClientId > mId)
				pClientId--;
		}
	}
	return lReturnValue;
}

/**
 * Set the player name.
 *
 * @param pPlayerName The player name
 */
void MR_NetworkInterface::SetPlayerName(const char *pPlayerName)
{
	mPlayer = pPlayerName;
	mLocalPartyNames[0] = mPlayer;
}

void MR_NetworkInterface::SetLocalParty(int pPartySize,
	const std::string *pPartyNames)
{
	mLocalPartySize = max(1, min(pPartySize, MR_MAX_LOCAL_PLAYER));
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		mLocalPartyNames[i] = (pPartyNames != NULL) ? pPartyNames[i].c_str() : "";
	}
	if(mLocalPartyNames[0].IsEmpty()) {
		mLocalPartyNames[0] = mPlayer;
	}
	mPlayer = mLocalPartyNames[0];
}

int MR_NetworkInterface::GetLocalPartySize() const
{
	return mLocalPartySize;
}

const char *MR_NetworkInterface::GetLocalPartyName(int pIndex) const
{
	if((pIndex < 0) || (pIndex >= MR_MAX_LOCAL_PLAYER)) {
		return "";
	}
	return mLocalPartyNames[pIndex];
}

/**
 * Get the player name.
 */
const char *MR_NetworkInterface::GetPlayerName() const
{
	return mPlayer;
} 

void MR_NetworkInterface::SetGameDetails(const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask, const MR_GameRuleSettings *pGameRuleSettings)
{
	mTrackName = (pTrackName != NULL) ? pTrackName : "";
	mNbLap = pNbLap;
	mHasGameLaps = (pNbLap >= 0);
	mHasGameWeapons = pHasWeapons;
	mAllowWeapons = pHasWeapons ? pAllowWeapons : FALSE;
	mHasGameCans = pHasCans;
	mAllowCans = pHasCans ? pAllowCans : FALSE;
	mHasGameMines = pHasMines;
	mAllowMines = pHasMines ? pAllowMines : FALSE;
	mHasGameCrafts = pHasCrafts;
	mAllowedCraftMask = pHasCrafts ?
		MR_NormalizeAllowedCraftMask(pAllowedCraftMask) :
		MR_GetDefaultAllowedCraftMask();
	mGameRuleSettings = (pGameRuleSettings != NULL) ? *pGameRuleSettings :
		MR_GameRuleSettings();
	MR_NormalizeGameRuleSettings(mGameRuleSettings);
}

/**
 * Set up the network interface if we are the server.  Clear existing connections, set up the server socket, and set up the interface to wait
 * for incoming connections.
 *
 * @param pWindow Parent window
 * @param pGameName Track name
 * @param pPromptForPort Should we use the default port or choose our own?
 * @param pDefaultPort The default port
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkInterface::MasterConnect(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines)
{
	return MasterConnect(pWindow, pGameName, pPromptForPort, pDefaultPort,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		FALSE, MR_GetDefaultAllowedCraftMask(), MR_GameRuleSettings());
}

BOOL MR_NetworkInterface::MasterConnect(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask)
{
	return MasterConnect(pWindow, pGameName, pPromptForPort, pDefaultPort,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		pHasCrafts, pAllowedCraftMask, MR_GameRuleSettings());
}

BOOL MR_NetworkInterface::MasterConnect(HWND pWindow, const char *pGameName,
	BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg,
	int pReturnMessage, const char *pTrackName, int pNbLap,
	BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans, BOOL pAllowCans,
	BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings)
{
	BOOL lReturnValue = FALSE;
	mActiveInterface = this;

	Disconnect();

	mGameName = pGameName;
	SetGameDetails(pTrackName, pNbLap, pHasWeapons, pAllowWeapons,
		pHasCans, pAllowCans, pHasMines, pAllowMines, pHasCrafts,
		pAllowedCraftMask, &pGameRuleSettings);
	mServerMode = TRUE;
	mId = 0; // we are client 0 (the host)

	// Create the server Socket
	mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

	if(mRegistrySocket == INVALID_SOCKET && !mSteamID.IsValid()) {
		MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		unsigned int lOpt = 1;
		setsockopt(mRegistrySocket, SOL_SOCKET, SO_REUSEADDR,
			(const char *) &lOpt, sizeof(lOpt));

		// Ask server port number
		HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

		mServerPort = pDefaultPort;

		if(pPromptForPort) {
			lReturnValue = (DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_SERVER_PORT), pWindow, ServerPortCallBack) == IDOK);
		}
		else {
			SOCKADDR_IN lAddr;

			lAddr.sin_family = AF_INET;
			lAddr.sin_addr.s_addr = INADDR_ANY;
			lAddr.sin_port = htons(mServerPort);

			if(bind(mRegistrySocket, (LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0) {
				lReturnValue = FALSE;
				MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}
			else {
				lReturnValue = TRUE;
			}
		}

		if(lReturnValue) {
			// set up UDP receive port
			lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

			if(!lReturnValue) {
				MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_UDP_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}

			if(lReturnValue) {
				// Determine server addr
				mServerAddr = GetLocalAddrStr(false);

				// Pop the Bit dialog
				if(pModalessDlg == NULL) {
					mReturnMessage = 0;

					// Normal mode (Modal)
					if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_SERVER), pWindow, ListCallBack) != IDOK) {
						lReturnValue = FALSE;
					}
				}
				else {
					mServerAddr = GetLocalAddrStr(mSteamOnly);

					ASSERT(pReturnMessage != 0);

					mReturnMessage = pReturnMessage;

					// Modaless mode
					*pModalessDlg = CreateDialog(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_SERVER), pWindow, ListCallBack);

					if(*pModalessDlg == NULL) {
						lReturnValue = FALSE;
					}
				}
			}
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}
	return lReturnValue;
}

/**
 * This function is called to get parameters about a server.  It is called as a result of the "Connect to TCP Server..." menu option being
 * selected.  When the dialog box is filled out, MR_NetworkInterface::ServerAddrCallBack is called.
 *
 * @param pWindow Parent window
 * @param pGameName CString to store the name of the game into
 */
BOOL MR_NetworkInterface::SlavePreConnect(HWND pWindow, CString &pGameName)
{
	BOOL lReturnValue = FALSE;
	mActiveInterface = this;

	Disconnect();

	pGameName = "";
	mGameName = "";

	// Create the registry Socket
	mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

	if(mRegistrySocket == INVALID_SOCKET && !mSteamID.IsValid()) {
		MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		// set up UDP receive socket
		lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

		if(lReturnValue) {
			// Ask server port addr and number
			HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

			if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_SERVER_ADDR), pWindow, ServerAddrCallBack) == IDOK) {
				lReturnValue = TRUE;

				pGameName = mGameName;
			} else {
				lReturnValue = FALSE;
			}
		}
		else {
			MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_UDP_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}

	return lReturnValue;
}

/**
 * This function is called when a user connects to an existing game from the IMR, or, after the SlavePreConnect phase completes (which is called
 * when a user connects to a came from the "Connect to TCP Server..." option.  This phase sets up a dialog to get server information (if the PreConnect
 * phase never happened) which calls MR_NetworkInterface::ServerAddrCallBack() and later a dialog (the "TCP Connections" waiting screen) that calls
 * MR_NetworkInterface::ListCallBack().
 *
 * @param pWindow Parent window
 * @param pServerIP IP of the server
 * @param pDefaultPort Default port for the server (MR_DEFAULT_NET_PORT)
 * @param pGameName Name of the game (title of the track)
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkInterface::SlaveConnect(HWND pWindow, const char *pServerIP,
	unsigned pDefaultPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines)
{
	return SlaveConnect(pWindow, pServerIP, pDefaultPort, pSteamID, pGameName,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		FALSE, MR_GetDefaultAllowedCraftMask(), MR_GameRuleSettings());
}

BOOL MR_NetworkInterface::SlaveConnect(HWND pWindow, const char *pServerIP,
	unsigned pDefaultPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask)
{
	return SlaveConnect(pWindow, pServerIP, pDefaultPort, pSteamID, pGameName,
		pModalessDlg, pReturnMessage, pTrackName, pNbLap, pHasWeapons,
		pAllowWeapons, pHasCans, pAllowCans, pHasMines, pAllowMines,
		pHasCrafts, pAllowedCraftMask, MR_GameRuleSettings());
}

BOOL MR_NetworkInterface::SlaveConnect(HWND pWindow, const char *pServerIP,
	unsigned pDefaultPort, uint64 pSteamID, const char *pGameName,
	HWND *pModalessDlg, int pReturnMessage, const char *pTrackName,
	int pNbLap, BOOL pHasWeapons, BOOL pAllowWeapons, BOOL pHasCans,
	BOOL pAllowCans, BOOL pHasMines, BOOL pAllowMines, BOOL pHasCrafts,
	unsigned pAllowedCraftMask,
	const MR_GameRuleSettings &pGameRuleSettings)
{
	ASSERT(!mServerMode);

	BOOL lReturnValue = TRUE;

	HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

	if(pGameName != NULL) {
		mGameName = pGameName;
	}
	if((pTrackName != NULL) || (pNbLap >= 0) || pHasWeapons || pHasCans ||
		pHasMines || pHasCrafts)
	{
		SetGameDetails(pTrackName, pNbLap, pHasWeapons, pAllowWeapons,
			pHasCans, pAllowCans, pHasMines, pAllowMines, pHasCrafts,
			pAllowedCraftMask, &pGameRuleSettings);
	}

	if(pServerIP != NULL) {
		Disconnect();
		ASSERT(!mServerMode);

		mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);
		mSteamID = CSteamID(pSteamID);

		if(mRegistrySocket == INVALID_SOCKET && !mSteamID.IsValid()) {
			MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			lReturnValue = FALSE;
		}
		else {
			// There was no pre-connect phase, do the preconnection now
			mServerAddr = mSteamID.IsValid() ? "Peer to Peer via Steam" : pServerIP;
			mServerPort = pDefaultPort;
			mSteamID = CSteamID(pSteamID);
			mActiveInterface = this;

			lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

			if(!lReturnValue) {
				MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_UDP_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}

			// figure out the game information
			lReturnValue = (DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK);
		}
	}

	ASSERT(mRegistrySocket != INVALID_SOCKET);
	ASSERT(mActiveInterface == this);

	// set up a dialog that will call ListCallBack
	if(lReturnValue) {
		if(pModalessDlg == NULL) {
			mReturnMessage = 0;

			// Normal mode (Modal)
			if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_CLIENT), pWindow, ListCallBack) != IDOK) {
				lReturnValue = FALSE;
			}
		}
		else {
			ASSERT(pReturnMessage != 0);

			mReturnMessage = pReturnMessage;

			// Modaless mode (Modal)
			*pModalessDlg = CreateDialog(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_CLIENT), pWindow, ListCallBack);

			if(*pModalessDlg == NULL) {
				lReturnValue = FALSE;
			}
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}

	return lReturnValue;
}

/**
 * This callback is used by the "Port selection" dialog (IDD_SERVER_PORT).  This is where the user sets their player name and the port to be
 * used by the server.  Once the user selects a port the socket is initialized.
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ServerPortCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);

			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					// retrieve player name
					char lNameBuffer[80];
					GetDlgItemText(pWindow, IDC_PLAYER_NAME, lNameBuffer, sizeof(lNameBuffer));
					mActiveInterface->mPlayer = lNameBuffer;

					// Try to bind the socket to the addr
					mActiveInterface->mServerPort = MR_Config::GetInstance()->net.tcpServPort;

					if(mActiveInterface->mServerPort <= 0) { // luser check
						MessageBox(pWindow, MR_LoadString(IDS_PORT_RANGE), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
					}
					else {
						SOCKADDR_IN lAddr;

						lAddr.sin_family = AF_INET;
						lAddr.sin_addr.s_addr = INADDR_ANY;
						lAddr.sin_port = htons(mActiveInterface->mServerPort);

						if(bind(mActiveInterface->mRegistrySocket, (LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0) {
							MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
						}
						else {
							EndDialog(pWindow, IDOK);
						}
					}
					lReturnValue = TRUE;
					break;
			}
	}

	return lReturnValue;
}

/**
 * Callback for the "Server selection" dialog box (IDD_SERVER_ADDR).  If the user hit OK, the entered player name, server address, and
 * port are saved.  Then, a new modal dialog is created (IDD_NET_PROGRESS) which calls WaitGameNameCallBack().
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ServerAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);
			SetDlgItemInt(pWindow, IDC_SERVER_PORT, 9530, FALSE); // 9530 is the default port
			lReturnValue = TRUE;
			break;

		case WM_COMMAND: // command was given
			switch (LOWORD(pWParam)) {
				case IDC_DEFAULT: // set the default port
					SetDlgItemInt(pWindow, IDC_SERVER_PORT, 9530, FALSE);
					lReturnValue = TRUE;
					break;

				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					{
						// retrieve player name
						char lNameBuffer[80];
						GetDlgItemText(pWindow, IDC_PLAYER_NAME, lNameBuffer, sizeof(lNameBuffer));
						mActiveInterface->mPlayer = lNameBuffer;
	
						char lBuffer[80];
						HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );
	
						lBuffer[0] = 0;
						GetDlgItemText(pWindow, IDC_SERVER_ADDR, lBuffer, sizeof(lBuffer));
	
						mActiveInterface->mServerAddr = lBuffer;
						mActiveInterface->mServerPort = GetDlgItemInt(pWindow, IDC_SERVER_PORT, NULL, FALSE);
	
						if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK) {
							EndDialog(pWindow, IDOK);
						}
						lReturnValue = TRUE;
					}
					break;
			}
			break;
	}

	return lReturnValue;
}

/**
 * This is the callback for the "Connecting to server" dialog box (IDD_NET_PROGRESS).  It actually gets called three times:
 *
 *   -#  When the dialog is initialized.  Set up the new socket to connect to the server.
 *   -#  When the new socket is connected to the server.  Ask the server for the game name.
 *   -#  When the server responds.  Save the game name, 
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::WaitGameNameCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	// static socket is to be setup by various callbacks
	static SOCKET sNewSocket = INVALID_SOCKET;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			{	
				mActiveInterface->mConnectModal = pWindow;
				mActiveInterface->mWaitGameNameConnected = FALSE;
				PositionDialogToRightOfOwner(pWindow);

				TRACE("\n%s: [WaitGameNameCallBack] WM_INITDIALOG ", mActiveInterface->GetPlayerName());

				// set up the static socket used to connect to the server
				sNewSocket = socket(PF_INET, SOCK_STREAM, 0);

				if(sNewSocket == INVALID_SOCKET) { // socket creation failed
					TRACE("INVALID_SOCKET ");

					if(!mActiveInterface->mSteamID.IsValid()) {
						SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CANT_CREATE_SOCK));
					}
				}
				else {
					TRACE(mActiveInterface->mSteamOnly ? "STEAM_SOCKET_CONNECT " : "SOCKET_CONNECT ");

					int lCode;
					int lTrue = 1;

					unsigned long lNonBlock = TRUE;
					lCode = ioctlsocket(sNewSocket, FIONBIO, &lNonBlock);

					// "Connecting to server..."
					SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CONNECTING_SERV));

					SOCKADDR_IN lAddr;
					memset(&lAddr, 0, sizeof(lAddr));

					lAddr.sin_family = AF_INET;
					lAddr.sin_addr.s_addr = GetAddrFromStr(mActiveInterface->mServerAddr);
					lAddr.sin_port = htons(mActiveInterface->mServerPort);

					mActiveInterface->mClientAddr[0] = GetAddrFromStr(mActiveInterface->mServerAddr);
					mActiveInterface->mClientBkAddr[0] = GetAddrFromStr(mActiveInterface->mServerAddr);
					mActiveInterface->mClientPort[0] = htons(mActiveInterface->mServerPort);

					if(!mActiveInterface->mSteamOnly) {
						// call this callback again when the socket successfully connects
						WSAAsyncSelect(sNewSocket, pWindow, MRM_SERVER_CONNECT, FD_CONNECT);
					}

					// Disable Nagle's algorithm: we don't want to accumulate packets and send as larger ones,
					// but instead send packets as quickly as they are ready -- screw TCP/IP efficiency

					ASSERT(lCode != SOCKET_ERROR);

					lCode = setsockopt(sNewSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &lTrue, sizeof(int));

					ASSERT(lCode != SOCKET_ERROR);

					// Reduce output queue to 512 bytes
					int lQueueSize = 512;

					lCode = setsockopt(sNewSocket, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));

					ASSERT(lCode != SOCKET_ERROR);

					// next instance of this function will be with the MRM_SERVER_CONNECT message
					mActiveInterface->Connect(sNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr), mActiveInterface->mSteamID, STM_NEW_CLIENT, 0);
				}
			}
			break;

		// sNewSocket has connected to the server successfully, now let's get the game information
		case MRM_SERVER_CONNECT:
			{
				TRACE("\n%s: [WaitGameNameCallBack] MRM_SERVER_CONNECT ", mActiveInterface->GetPlayerName());
				AppendHandshakeLog("WaitGameName MRM_SERVER_CONNECT steamOnly=%d waitConnected=%d steamId=%u",
					mActiveInterface->mSteamOnly, mActiveInterface->mWaitGameNameConnected,
					mActiveInterface->mSteamID.GetAccountID());
				if(mActiveInterface->mWaitGameNameConnected) {
					lReturnValue = TRUE;
					break;
				}

				MR_NetMessageBuffer lOutputBuffer;
				SOCKADDR_IN lAddr;
				memset(&lAddr, 0, sizeof(lAddr));
				lAddr.sin_family = AF_INET;
				lAddr.sin_addr.s_addr = INADDR_ANY;
				lAddr.sin_port = htons(mActiveInterface->mTCPRecvPort);
	
				if(WSAGETSELECTERROR(pLParam) == 0) {
					mActiveInterface->mWaitGameNameConnected = TRUE;
					TRACE("CONNECTING ");
					AppendHandshakeLog("WaitGameName connected ok -> send MRNM_GET_GAME_NAME to steamId=%u",
						mActiveInterface->mSteamID.GetAccountID());

					SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_GET_GAMEINFO));
	
					// mClient[0] is the server (if we're not the server)
					mActiveInterface->mClient[0].Connect(sNewSocket, mActiveInterface->mUDPRecvSocket, mActiveInterface->mSteamID, mActiveInterface->mSteamOnly);

					if(!mActiveInterface->mSteamOnly) {
						// callback with MRM_CLIENT message once the socket reads data
						WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, FD_READ);

						// bind the registry socket to the server
						int lSize = sizeof(lAddr);

						if(mActiveInterface->mClient[0].GetSocket() != INVALID_SOCKET) {
							SOCKADDR_IN lBoundAddr;
							memset(&lBoundAddr, 0, sizeof(lBoundAddr));
							lBoundAddr.sin_family = AF_INET;

							if(getsockname(mActiveInterface->mClient[0].GetSocket(),
								(struct sockaddr *) &lBoundAddr, &lSize) == 0)
							{
								lAddr.sin_addr.s_addr = lBoundAddr.sin_addr.s_addr;
							}
						}

						unsigned int lOpt = 1;
						setsockopt(mActiveInterface->mRegistrySocket, SOL_SOCKET,
							SO_REUSEADDR, (const char *) &lOpt, sizeof(lOpt));

						if(bind(mActiveInterface->mRegistrySocket,
							(LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0)
						{
							SOCKADDR_IN lFallbackAddr;
							memset(&lFallbackAddr, 0, sizeof(lFallbackAddr));
							lFallbackAddr.sin_family = AF_INET;
							lFallbackAddr.sin_addr.s_addr = INADDR_ANY;
							lFallbackAddr.sin_port = htons(mActiveInterface->mTCPRecvPort);

							if(bind(mActiveInterface->mRegistrySocket,
								(LPSOCKADDR) &lFallbackAddr,
								sizeof(lFallbackAddr)) != 0 && !mActiveInterface->mSteamID.IsValid())
							{
								TRACE("IDS_CANT_CREATE_SOCK ");

								MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);
							}
						}

						lSize = sizeof(lAddr);
						lAddr.sin_family = AF_INET;

						getsockname(mActiveInterface->mRegistrySocket, (struct sockaddr *) &lAddr, &lSize);
					}
	
					// send a message asking for the game name
					lOutputBuffer.mMessageType = MRNM_GET_GAME_NAME;
					lOutputBuffer.mClient = mActiveInterface->mId;
					lOutputBuffer.mDataLen = 8;
					*(int *) &(lOutputBuffer.mData[0]) = lAddr.sin_addr.s_addr;
					*(int *) &(lOutputBuffer.mData[4]) = lAddr.sin_port;
	
					mActiveInterface->mClient[0].Send(&lOutputBuffer, MR_NET_REQUIRED);
					AppendHandshakeLog("Sent MRNM_GET_GAME_NAME addr=%u port=%u", lAddr.sin_addr.s_addr, (unsigned) ntohs((u_short) lAddr.sin_port));
				}
				else if(!mActiveInterface->mSteamID.IsValid()) {
					TRACE("IDS_CANT_CONNECT ");

					SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CANT_CONNECT));
				}
			}
			lReturnValue = TRUE;
			break;

		case MRM_CLIENT:
			{
				TRACE("\n%s: [WaitGameNameCallBack] MRM_CLIENT ", mActiveInterface->GetPlayerName());
				AppendHandshakeLog("WaitGameName MRM_CLIENT event");

				const MR_NetMessageBuffer *lBuffer = NULL;

				int param = !mActiveInterface->mClient[0].mSteamOnly ? WSAGETSELECTEVENT(pLParam) : pLParam;
	
				switch (param) {
					case FD_READ:
						TRACE("FD_READ ");

						// disable reception of this message
						WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, 0);
						if (!mActiveInterface->mClient[0].mSteamOnly) {
							lBuffer = mActiveInterface->mClient[0].Poll(0, TRUE);
						} else {
							lBuffer = sBuffer;
						}
	
						if((lBuffer != NULL) && (lBuffer->mMessageType == MRNM_GAME_NAME)) {
							TRACE("MRNM_GAME_NAME ");
							AppendHandshakeLog("Received MRNM_GAME_NAME len=%d", lBuffer->mDataLen);

							mActiveInterface->mGameName = CString((const char *) lBuffer->mData, lBuffer->mDataLen);
							if(mActiveInterface->mTrackName.IsEmpty()) {
								CString lTrackName;
								CString lLapText;
								CString lPowerupsText;
								CString lCraftsText;

				if(ParseTcpGameSummary(mActiveInterface->mGameName, lTrackName,
					lLapText, lPowerupsText, lCraftsText))
				{
					std::string lSummary((const char *) mActiveInterface->mGameName);

					MR_ParseGameRuleSummary(lSummary,
						mActiveInterface->mGameRuleSettings);
					mActiveInterface->mTrackName = lTrackName;
									if(!lLapText.IsEmpty()) {
										mActiveInterface->mNbLap = atoi((const char *) lLapText);
										mActiveInterface->mHasGameLaps = TRUE;
									}
									if(!lPowerupsText.IsEmpty()) {
										mActiveInterface->mAllowWeapons =
											(lPowerupsText.Find("Missiles") != -1);
										mActiveInterface->mHasGameWeapons = TRUE;
										mActiveInterface->mHasGameCans = TRUE;
										mActiveInterface->mHasGameMines = TRUE;
										mActiveInterface->mAllowCans =
											(lPowerupsText.Find("Cans") != -1);
										mActiveInterface->mAllowMines =
											(lPowerupsText.Find("Mines") != -1);
									}
									if(!lCraftsText.IsEmpty()) {
										mActiveInterface->mAllowedCraftMask =
											MR_ParseAllowedCraftMask(lCraftsText);
										mActiveInterface->mHasGameCrafts = TRUE;
									}
								}
							}
							mActiveInterface->mConnectModal = NULL;
							EndDialog(pWindow, IDOK);
						}
						else {
							TRACE("BAD_MESSAGE ");

							// Bad message, reenable reception
							WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, FD_READ);
						}
	
						break;
				}
			}
			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {	
				case IDCANCEL: // user canceled connection
					TRACE("\n%s: [WaitGameNameCallBack] IDCANCEL ", mActiveInterface->GetPlayerName());

					lReturnValue = TRUE;
					mActiveInterface->mConnectModal = NULL;
					mActiveInterface->mWaitGameNameConnected = FALSE;
					closesocket(sNewSocket);

					if(mActiveInterface->mSteamOnly && mActiveInterface->mSteamID.IsValid()) {
						MR_NetMessageBuffer lCancel;
						lCancel.mMessageType = STM_CLIENT_CLOSE;
						lCancel.mClient = mActiveInterface->mId;
						lCancel.mDataLen = 0;

						if(mActiveInterface->mClient[0].GetSteamId().IsValid()) {
							mActiveInterface->mClient[0].Send(&lCancel, MR_NET_REQUIRED);
						}
						else {
							SteamNetworking()->SendP2PPacket(mActiveInterface->mSteamID, &lCancel, MR_NET_HEADER_LEN, k_EP2PSendReliable, STM_IMR_CHANNEL);
						}
					}

					mActiveInterface->CleanupClientState(0);

					EndDialog(pWindow, IDCANCEL);
					break;
			}
	}

	return lReturnValue;
}

/**
 * This method is called when modifications to the "TCP Connections" dialog (IDD_TCP_CLIENT or IDD_TCP_SERVER) occur.  Four basic sequences of packet
 * reception occur through this method -- one for the server, one for a client connecting to the server, one for a client connecting to another client,
 * and another for a client being connected to by another client.  A basic flowchart:
 *
 * Server:
 *
 *  -#  Dialog is initialized.  Set up the main socket for reception and wait for a new client to connect (MRM_NEW_CLIENT).
 *  -#  A new client has connected: find a place in the list for them, add them to the list, and wait for another message (MRNM_CONN_NAME_GET_SET)
 *  -#  Client has responded (MRNM_CONN_NAME_GET_SET) with player name and UDP recv port; add it to the list and respond with our name and UDP
 *      receive port (MRNM_CONN_NAME_SET) and then send the list of other client IPs and names (MRNM_CLIENT_ADDR).  Wait for client to initiate lag
 *      test (MRNM_LAG_TEST).
 *  -#  Lag test initiation received (MRNM_LAG_TEST); respond with current time (MRNM_LAG_ANSWER).  The client will repeat the lag test 5 times, then
 *      inform us of the lag info.
 *  -#  Lag info received (MRNM_LAG_INFO).  Wait for more connections (go to step 2) or user input to start the race.
 *  -#  User started the race.  If some connections aren't done, ask the user if they really want to continue.  If they do, send an MRNM_READY
 *      message to all clients.  Remove the dialog and send mReturnMessage to the parent window (pWindow).
 *
 * Client connecting to server:
 *
 *  -#  Dialog is initialized.  Set up sockets for reception and ask the server for the game name (MRNM_CONN_NAME_GET_SET), sending the player name
 *      and UDP receive port.
 *  -#  Server responded with their name and UDP receive port (MRNM_CONN_NAME_SET); initiate a lag test (MRNM_LAG_TEST), sending the current time.
 *      Wait for the response.
 *  -#  Server responded with lag (MRNM_LAG_ANSWER).  Do 4 more lag tests (totaling 5 tests).
 *  -#  Send the server lag info (MRNM_LAG_INFO).  Mark this connection as done, update the dialog box as such.
 *  -#  Wait for the MRNM_READY message from the server to start the game.
 *
 * Client connecting to another client:
 *
 *  -#  The server gave us the info to connect to the new client with the MRNM_CLIENT_ADDR message.  Set up the socket and try to connect to the new
 *      client.
 *  -#  (jump to the first step of the "Client connecting to server:" section)
 *  -#  When the lag tests are done, call SendConnectionDoneIfNeeded() to send the MRNM_CONNECTION_DONE message to the server if we are connected to
 *      all the other clients.  
 *  -#  Wait for the MRNM_READY message.
 *
 * Client being connected to by another client:
 *
 *  -#  We received a connection request from a client.
 *  -#  (jump to "Server:", step 2, follow until step 5.
 *  -#  Wait for the MRNM_READY message.
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;
	HWND lListHandle = NULL;
	MR_NetMessageBuffer lAnswer;
	lAnswer.mClient = mActiveInterface->GetId();

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the window
			{
				mActiveInterface->mGameModal = pWindow;
				gsTcpTrackPreviewWindow = NULL;
				gsTcpOriginalGameName = "";
				gsTcpTrackName = "";
				gsTcpLapText = "";
				gsTcpPowerupsText = "";
				gsTcpCraftsText = "";
				ClearTcpTrackPreview();

				TRACE("\n%s: [ListCallBack] WM_INITDIALOG ", mActiveInterface->GetPlayerName());

				RECT lRect;
	
				lListHandle = GetDlgItem(pWindow, IDC_LIST);
	
				if(!GetClientRect(lListHandle, &lRect)) {
					ASSERT(FALSE);
				}
				else {
					// Create list columns
					int lWidth = lRect.right;
					LV_COLUMN lSpec;
	
					lSpec.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	
					CString lLagLabel;
					lLagLabel.LoadString(IDS_LAG);
					CString lStatusLabel;
					lStatusLabel.LoadString(IDS_STATUS);
					SIZE lLagSize = {0, 0};
					SIZE lStatusSize = {0, 0};
					HDC lDc = GetDC(lListHandle);
					if(lDc != NULL) {
						HFONT lFont = (HFONT) SendMessage(lListHandle, WM_GETFONT, 0, 0);
						HGDIOBJ lOldFont = NULL;
						if(lFont != NULL) {
							lOldFont = SelectObject(lDc, lFont);
						}
						GetTextExtentPoint32(lDc, lLagLabel, lLagLabel.GetLength(), &lLagSize);
						GetTextExtentPoint32(lDc, lStatusLabel, lStatusLabel.GetLength(), &lStatusSize);
						if(lOldFont != NULL) {
							SelectObject(lDc, lOldFont);
						}
						ReleaseDC(lListHandle, lDc);
					}

					const int lScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
					const int lLagWidth = max(lWidth / 5, lLagSize.cx + 20);
					int lStatusWidth = max(lWidth / 4, lStatusSize.cx + 18);
					int lPlayerWidth = lWidth - lLagWidth - lStatusWidth - lScrollbarWidth;
					const int lMinPlayerWidth = 110;
					if(lPlayerWidth < lMinPlayerWidth) {
						const int lMinStatusWidth = lStatusSize.cx + 18;
						const int lStatusReduction = min(lMinPlayerWidth - lPlayerWidth,
							lStatusWidth - lMinStatusWidth);
						lStatusWidth -= lStatusReduction;
						lPlayerWidth = lWidth - lLagWidth - lStatusWidth - lScrollbarWidth;
					}

					CString lPlayerLabel;
					lPlayerLabel.LoadString(IDS_PLAYER);
					lSpec.fmt = LVCFMT_LEFT;
					lSpec.cx = lPlayerWidth;
					lSpec.pszText = (char *) (const char *) lPlayerLabel;
					lSpec.iSubItem = 0;

					ListView_InsertColumn(lListHandle, 0, &lSpec);

					lSpec.fmt = LVCFMT_RIGHT;
					lSpec.cx = lLagWidth;
					lSpec.pszText = (char *) (const char *) lLagLabel;
					lSpec.iSubItem = 1;

					ListView_InsertColumn(lListHandle, 1, &lSpec);

					lSpec.fmt = LVCFMT_LEFT;
					lSpec.cx = lStatusWidth;
					lSpec.pszText = (char *) (const char *) lStatusLabel;
					lSpec.iSubItem = 2;
	
					ListView_InsertColumn(lListHandle, 2, &lSpec);
				}
	
				// set game information in dialog
				SetDlgItemInt(pWindow, IDC_SERVER_PORT, mActiveInterface->mServerPort, FALSE);
				CString lServerAddrText =
					FormatTcpServerAddrForDialog(mActiveInterface->mServerAddr);
				SetDlgItemText(pWindow, IDC_SERVER_ADDR, lServerAddrText);
				AdjustTcpServerAddrLayout(pWindow, lServerAddrText);
				gsTcpOriginalGameName = mActiveInterface->mGameName;
				gsTcpTrackName = mActiveInterface->mTrackName;
				if(mActiveInterface->mHasGameLaps) {
					gsTcpLapText = MR_FormatGameRuleConfigSummary(
						mActiveInterface->mGameRuleSettings,
						mActiveInterface->mNbLap).c_str();
				}
				if(mActiveInterface->mHasGameWeapons || mActiveInterface->mHasGameCans ||
					mActiveInterface->mHasGameMines)
				{
					gsTcpPowerupsText = MR_FormatPowerupDisplay(
						mActiveInterface->mHasGameWeapons && (mActiveInterface->mAllowWeapons != FALSE),
						mActiveInterface->mHasGameCans && (mActiveInterface->mAllowCans != FALSE),
						mActiveInterface->mHasGameMines && (mActiveInterface->mAllowMines != FALSE)).c_str();
				}
				if(mActiveInterface->mHasGameCrafts) {
					gsTcpCraftsText = MR_FormatAllowedCraftDisplayMask(
						mActiveInterface->mAllowedCraftMask).c_str();
				}
				UpdateTcpDialogTrackSummary(pWindow);
	
				// Add the current local party to the list
				LV_ITEM lItem;
	
				lItem.mask = LVIF_TEXT;
				lItem.iItem = 0;
				lItem.iSubItem = 0;
				lItem.pszText = "";
	
				for(int lCounter = 0; lCounter < mActiveInterface->GetLocalPartySize(); lCounter++) {
					lItem.iItem = lCounter;
					ListView_InsertItem(lListHandle, &lItem);
					ListView_SetItemText(lListHandle, lCounter, 0,
						(char *) mActiveInterface->GetLocalPartyName(lCounter));
					ListView_SetItemText(lListHandle, lCounter, 1,
						(char *) MR_LoadStringBuffered(IDS_LOCAL));
					ListView_SetItemText(lListHandle, lCounter, 2,
						(char *) MR_LoadStringBuffered(IDS_CONNECTED));
				}
	
				for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
					for(int lPartyIndex = 0; lPartyIndex < MR_MAX_LOCAL_PLAYER; ++lPartyIndex) {
						lItem.iItem = GetTcpListRowForClientPartyMember(
							mActiveInterface, lCounter, lPartyIndex);
						lItem.pszText = "";
						ListView_InsertItem(lListHandle, &lItem);
					}
				}

				ExpandTcpConnectionsDialogIfNeeded(pWindow, lListHandle);
				LoadTcpTrackPreview(GetTcpTrackPreviewTrackName(pWindow));
				if(gsTcpTrackPreviewWindow != NULL) {
					InvalidateRect(gsTcpTrackPreviewWindow, NULL, TRUE);
				}
				PositionDialogToRightOfOwner(pWindow);
	
				// Put the registry socket in listen mode, for the MRM_NEW_CLIENT message
				listen(mActiveInterface->mRegistrySocket, 5);
				WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_NEW_CLIENT, FD_ACCEPT);

				TRACE("WSAAsyncSelect/MRM_NEW_CLIENT ");
	
				if(!mActiveInterface->mServerMode) {
					TRACE("!mServerMode ");

					// allow reception of messages with id MRM_CLIENT + 0, MRM_CLIENT + 1
					WSAAsyncSelect(mActiveInterface->mClient[0].GetSocket(), pWindow, MRM_CLIENT + 0, FD_READ);
					WSAAsyncSelect(mActiveInterface->mClient[0].GetUDPSocket(), pWindow, MRM_CLIENT + 1, FD_READ);
	
					// Request server name to start sequence
					// also include UDP port number in the request
					lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
					lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
					*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);
					memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);

					mActiveInterface->mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
					mActiveInterface->SendPartyInfo(0);
				}
			}
	
			break;

		case WM_DRAWITEM:
			if((pWParam == IDC_TCP_TRACK_PREVIEW) && (pLParam != 0)) {
				DrawTcpTrackPreview((const DRAWITEMSTRUCT *) pLParam);
				lReturnValue = TRUE;
			}
			break;

		case WM_DESTROY:
			ClearTcpTrackPreview();
			gsTcpTrackPreviewWindow = NULL;
			gsTcpOriginalGameName = "";
			gsTcpTrackName = "";
			gsTcpLapText = "";
			gsTcpPowerupsText = "";
			break;

		case WM_COMMAND:
			TRACE("\n%s: [ListCallBack] WM_COMMAND ", mActiveInterface->GetPlayerName());

			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					TRACE("IDCANCEL\n");

					lAnswer.mMessageType = mActiveInterface->mServerMode ? MRNM_CANCEL_GAME : STM_CLIENT_CLOSE;
					lAnswer.mDataLen = 1;
					mActiveInterface->BroadcastMessage(&lAnswer, MR_NET_REQUIRED); // tell everyone

					mActiveInterface->Disconnect(FALSE);
					lReturnValue = TRUE;

					if(mActiveInterface->mReturnMessage == 0) {
						EndDialog(pWindow, IDCANCEL);
					}
					else {
						SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDCANCEL, 0);
						DestroyWindow(pWindow);
					}
					break;

				case IDOK:
					{
						TRACE("IDOK\n");

						ASSERT(mActiveInterface->mServerMode);
	
						int lCounter;
						BOOL lOk = TRUE;
						// Verify that lag have been computed for all connections
						for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
							if(mActiveInterface->mClient[lCounter].IsConnected() && !mActiveInterface->mConnected[lCounter]) {
								lOk = FALSE;
							}
						}
	
						if(!lOk) {
							HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );
	
							if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_WAIT_ALL), pWindow, DialogProc) == IDOK) {
								lOk = TRUE;
							}
						}
	
						if(lOk) {
							MR_NetMessageBuffer lMessage;
	
							lMessage.mMessageType = MRNM_READY;
							lMessage.mClient = mActiveInterface->mId;
							lMessage.mDataLen = 1;
	
							// Send a READY message to all the clients
							for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
								lMessage.mData[0] = lCounter + 1;
	
								mActiveInterface->mClient[lCounter].Send(&lMessage, MR_NET_REQUIRED);
							}
	
							// Disable all callbacks
							for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
								if(mActiveInterface->mClient[lCounter].IsConnected()) {
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetSocket(), pWindow, MRM_CLIENT + lCounter, 0);
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetUDPSocket(), pWindow, MRM_CLIENT + lCounter, 0);
								}
							}
							WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_CLIENT, 0);


	
							if(mActiveInterface->mReturnMessage == 0) {
								EndDialog(pWindow, IDOK);
							}
							else {
								SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
								DestroyWindow(pWindow);
							}
						}
						else {
							// MessageBox( pWindow, "Please wait until all clients have all successfully connected", "TCP Server", MB_ICONINFORMATION|MB_OK|MB_APPLMODAL );
						}
					}
					break;
			}
			break;

		case MRM_NEW_CLIENT: // a new client has connected
			{
				TRACE("\n%s: [ListCallBack] MRM_NEW_CLIENT ", mActiveInterface->GetPlayerName());

				int lNewSlot = -1;
	
				// figure out what slot we're going to put them in
				for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
					if(!mActiveInterface->mClient[lCounter].IsConnected()) {

						lNewSlot = lCounter;
						break;
					}
				}

				if(lNewSlot != -1) {
					// accept our new connection
					SOCKET lNewSocket = mActiveInterface->Accept(mActiveInterface->mRegistrySocket, NULL, 0, sSteamID, mActiveInterface->mServerMode ? STM_SERVER_CONNECT : STM_CLIENT_CONNECT, FD_CONNECT, lNewSlot);

					TRACE(" -- MRM_NEW_CLIENT --\n");
					TRACE("lNewSlot = %d / Valid? %s / SteamID %d\n", lNewSlot, (sSteamID.IsValid() ? "Yes" : "No"), sSteamID.GetAccountID());

					if(lNewSocket != INVALID_SOCKET && mActiveInterface->mClient[lNewSlot].mSocketConnected || sSteamID.IsValid()) {
						int lCode;
						int lTrue = 1;

						lCode = setsockopt(lNewSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &lTrue, sizeof(int));

						if(!sSteamID.IsValid())
						{
							ASSERT(lCode != SOCKET_ERROR);
						}

						// Reduce output queue to 512 bytes
						int lQueueSize = 512;

						lCode = setsockopt(lNewSocket, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));

						if(!sSteamID.IsValid())
						{
							ASSERT(lCode != SOCKET_ERROR);
						}

						mActiveInterface->mClient[lNewSlot].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket, sSteamID, mActiveInterface->mSteamOnly);

						// wait for new message (MRM_CLIENT + lNewSlot)
						WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);
						WSAAsyncSelect(mActiveInterface->mClient[lNewSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);

						TRACE("WSAAsyncSelect/MRM_CLIENT+lNewSlot ");
					}
				}
			}
			break;

		case WM_TIMER: // used for lag tests
			{
				TRACE("\n%s: [ListCallBack] WM_TIMER ", mActiveInterface->GetPlayerName());

				if(pWParam - 10 > 0) { // client connect attempt timed out
					// retry
					KillTimer(pWindow, pWParam);

					TRACE("CLIENT_CONNECT_TIMED_OUT ");

					int lClient = pWParam - 20;

					// try to connect again
					TRACE("attempting reconnect with client %d\n", lClient);
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, 0);
					
					mActiveInterface->mClient[lClient].Disconnect();
					SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

					mActiveInterface->mClient[lClient].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket, mActiveInterface->mClient[lClient].GetSteamId(), mActiveInterface->mSteamOnly);
					mActiveInterface->mPreLoguedClient[lClient] = TRUE;

					SOCKADDR_IN lAddr;

					lAddr.sin_family = AF_INET;
					lAddr.sin_addr.s_addr = mActiveInterface->mClientAddr[lClient];
					lAddr.sin_port = mActiveInterface->mClientPort[lClient];

					// connect to the new client
					WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);
					int lCode = mActiveInterface->Connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr), mActiveInterface->mClient[lClient].GetSteamId(), MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);

					// set timeout timer
					SetTimer(pWindow, lClient + 20, MR_CLIENT_RETRY_TIME, NULL);

				} else {
					KillTimer(pWindow, pWParam);
	
					int lClient = pWParam - 10;
	
					if((lClient >= 0) && (lClient < eMaxClient)) {
						if(mActiveInterface->mClient[lClient].LagDone()) {
							// That is a late time-out.. probably stuck in the queue
							TRACE("Late ping message A %d\n", lClient);
						}
						else {
							TRACE("MR_PING_RETRY_TIME ");

							// Start a time-out timer because the request may fail
							SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);
	
							// send a new request
							lAnswer.mMessageType = MRNM_LAG_TEST;
							lAnswer.mDataLen = 4;
							*(int *) &(lAnswer.mData[0]) = timeGetTime();
	
							mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
						}
					}
				}
			}
			break;
	}

	// client message

	/**
	 * Here is how I want to change this:
	 *
	 * 1.  All messages become MRM_CLIENT.
	 * 2.  lClient is not determined until the message is read.
	 * 3.  The next message in the queue is always dealt with (no ignoring).
	 *
	 * But what if lClient is MR_ID_NOT_SET?  How do we differentiate the clients?
	 */
	if((pMsgId >= MRM_CLIENT) && (pMsgId < (MRM_CLIENT + eMaxClient))) {
		int lClient = pMsgId - MRM_CLIENT;
		const MR_NetMessageBuffer *lBuffer = NULL;

		lListHandle = GetDlgItem(pWindow, IDC_LIST);

		int param = mActiveInterface->mClient[lClient].mSocketConnected && !mActiveInterface->mClient[lClient].mSteamOnly ? WSAGETSELECTEVENT(pLParam) : pLParam;

		switch (param) {
			case FD_CLOSE:
				TRACE("\n%s: [ListCallBack] FD_CLOSE ", mActiveInterface->GetPlayerName());

				// client quit this game
				TRACE("Client %d disconnected\n", lClient);
				{
					const BOOL lWasPreLogued = mActiveInterface->mCanBePreLogued[lClient];
					mActiveInterface->CleanupClientState(lClient, pWindow);

					if(lWasPreLogued) {
						mActiveInterface->NotifyClientRemoved(lClient);
					}
				}

				break;

			case FD_READ:		
				TRACE("\n%s: [ListCallBack] FD_READ ", mActiveInterface->GetPlayerName());

				// get our message
				if (mActiveInterface->mClient[lClient].mSocketConnected && !mActiveInterface->mClient[lClient].mSteamOnly) {
					lBuffer = mActiveInterface->mClient[lClient].Poll(0, FALSE);
				} else {
					// TRACE(" --- ListCallBack --- \n");
					lBuffer = sBuffer;
					// TRACE("FD_READ type %d from client %d \n", lBuffer->mMessageType, lBuffer->mClient);
				}

				if(lBuffer != NULL) {
					// we must make sure this message is from the right client
					if(lBuffer->mClient != MR_ID_NOT_SET) {
						// We need to modify mClient.  If we are player #1, player #2 should be client 0, player #3 should be client 1,
						// and so on.  If we are player #2, player #1 is client 0, player #3 is client 1, and so on.
						// What this boils down to is that if pClientId is greater than our own ID, we have to decrement it
						if(lBuffer->mClient > mActiveInterface->mId)
							lClient = lBuffer->mClient - 1;
						else
							lClient = lBuffer->mClient;
					}

					TRACE("lClient %d ", lClient);

					// disable further reception of this message
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, 0);
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, 0);
					// TRACE("Packet processing: lClient is %d, ID is %d\n", lClient, lBuffer->mMessageType);

					TRACE("DISABLING MRM_CLIENT + lClient ");

					switch (lBuffer->mMessageType) {
						case MRNM_GET_GAME_NAME: // can only occur in server mode: client asked for game name
							TRACE("MRNM_GET_GAME_NAME ");

							ASSERT(mActiveInterface->mServerMode);

							// we want to get the IP ourselves instead of from the client
							// but we want to save what they said
							mActiveInterface->mClientBkAddr[lClient] = *(int *) &(lBuffer->mData[0]);
							{
								SOCKADDR_IN lClientAddr;
								int lSize = sizeof(lClientAddr);
								if(getpeername(mActiveInterface->mClient[lClient].GetSocket(), (SOCKADDR *) &lClientAddr, &lSize) != 0 && !mActiveInterface->mClient[lClient].GetSteamId().IsValid()) {
									ASSERT(FALSE);
								} else {
									mActiveInterface->mClientAddr[lClient] = *(int *) &(lClientAddr.sin_addr);
								}
								// TRACE("Client addr: %08x (id %d)\n", mActiveInterface->mClientAddr[lClient], lClient);
							}
							mActiveInterface->mClientPort[lClient] = *(int *) &(lBuffer->mData[4]);

							lAnswer.mMessageType = MRNM_GAME_NAME;
							lAnswer.mDataLen = mActiveInterface->mGameName.GetLength();
							memcpy(lAnswer.mData, (const char *) mActiveInterface->mGameName, lAnswer.mDataLen);

							mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

							break;

						case MRNM_CONN_NAME_GET_SET: // client has given us their name and their UDP recv port
							{
								TRACE("MRNM_CONN_NAME_GET_SET ");
								const int lPartySize = (lBuffer->mDataLen >= 5) ?
									max(1, min((int) lBuffer->mData[4], MR_MAX_LOCAL_PLAYER)) : 1;
								const int lNameOffset = (lBuffer->mDataLen >= 5) ? 5 : 4;
								AppendHandshakeLog("slot=%d recv MRNM_CONN_NAME_GET_SET size=%d name=%.*s", lClient,
									lPartySize, max(0, lBuffer->mDataLen - lNameOffset),
									(const char *) (lBuffer->mData + lNameOffset));

								// Add the item int the list
								LV_ITEM lItem;
								CString lConnectionName((const char *) (lBuffer->mData + lNameOffset),
									lBuffer->mDataLen - lNameOffset);
	
								mActiveInterface->mClient[lClient].SetRemoteUDPPort(*(unsigned int *) (lBuffer->mData));
								mActiveInterface->mClientName[lClient] = lConnectionName;
								mActiveInterface->mClientPartySize[lClient] = lPartySize;
								mActiveInterface->mClientPartyNames[lClient][0] = lConnectionName;
								for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
									mActiveInterface->mClientPartyNames[lClient][i] = "";
								}
	
								lItem.mask = LVIF_TEXT;
								lItem.iItem = GetTcpListRowForClient(mActiveInterface, lClient);
								lItem.iSubItem = 0;
								lItem.pszText = (char *) ((const char *) lConnectionName);

								if(ListView_GetItemCount(lListHandle) > lItem.iItem) {
									ListView_SetItem(lListHandle, &lItem);
								}
								else {
									ListView_InsertItem(lListHandle, &lItem);
								}
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 1,
									(char *) MR_LoadStringBuffered(IDS_COMPUTING));
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 2,
									(char *) MR_LoadStringBuffered(IDS_CONNECTING));
								UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);
	
								// return local name as an answer
								// also include UDP port number in the request
								lAnswer.mMessageType = MRNM_CONN_NAME_SET;
								lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 5;
								*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);
								lAnswer.mData[4] = (MR_UInt8) max(1,
									min(mActiveInterface->mLocalPartySize, MR_MAX_LOCAL_PLAYER));
								memcpy(lAnswer.mData + 5, mActiveInterface->mPlayer, lAnswer.mDataLen - 5);
	
								mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
								mActiveInterface->SendPartyInfo(lClient);

								// Tell client their ID and send client list if server
								if(mActiveInterface->mServerMode) {
									// send player ID
									lAnswer.mMessageType = MRNM_SET_PLAYER_ID;
									lAnswer.mDataLen = 1;
									lAnswer.mData[0] = lClient + 1;
	
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
									
									// now send the client list
									lAnswer.mMessageType = MRNM_CLIENT_ADDR;
									lAnswer.mClient = 0;
									lAnswer.mDataLen = 21;
	
									// Send the actual client list
									for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
										if((lCounter != lClient)
											&& (mActiveInterface->mClient[lCounter].IsConnected())
											&& (mActiveInterface->mCanBePreLogued[lCounter])) {
											*(int *) &(lAnswer.mData[0]) = mActiveInterface->mClientAddr[lCounter];
											*(int *) &(lAnswer.mData[4]) = mActiveInterface->mClientBkAddr[lCounter];
											*(int *) &(lAnswer.mData[8]) = mActiveInterface->mClientPort[lCounter];
											*(uint64 *) &(lAnswer.mData[12]) = mActiveInterface->mClient[lCounter].GetSteamId().ConvertToUint64();
											*(BOOL *) &(lAnswer.mData[20]) = mActiveInterface->mClient[lCounter].mSteamOnly;
	
											mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
	
										}
									}
									lAnswer.mDataLen = 0;
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
	
									mActiveInterface->mCanBePreLogued[lClient] = TRUE;
								}
	
							}
							break;

						case MRNM_CONN_NAME_SET: // client returned information on their name and UDP recv port
							{
								TRACE("MRNM_CONN_NAME_SET ");
								const int lPartySize = (lBuffer->mDataLen >= 5) ?
									max(1, min((int) lBuffer->mData[4], MR_MAX_LOCAL_PLAYER)) : 1;
								const int lNameOffset = (lBuffer->mDataLen >= 5) ? 5 : 4;
								AppendHandshakeLog("slot=%d recv MRNM_CONN_NAME_SET size=%d name=%.*s", lClient,
									lPartySize, max(0, lBuffer->mDataLen - lNameOffset),
									(const char *) (lBuffer->mData + lNameOffset));

								LV_ITEM lItem;
								CString lConnectionName((const char *) (lBuffer->mData + lNameOffset),
									lBuffer->mDataLen - lNameOffset);
	
								mActiveInterface->mClient[lClient].SetRemoteUDPPort(*(unsigned int *) (lBuffer->mData));
								mActiveInterface->mClientName[lClient] = lConnectionName;
								mActiveInterface->mClientPartySize[lClient] = lPartySize;
								mActiveInterface->mClientPartyNames[lClient][0] = lConnectionName;
								for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
									mActiveInterface->mClientPartyNames[lClient][i] = "";
								}
	
								lItem.mask = LVIF_TEXT;
								lItem.iItem = GetTcpListRowForClient(mActiveInterface, lClient);
								lItem.iSubItem = 0;
								lItem.pszText = (char *) ((const char *) lConnectionName);

								// Add the client into the list
								if(ListView_GetItemCount(lListHandle) > lItem.iItem) {
									ListView_SetItem(lListHandle, &lItem);
								}
								else {
									ListView_InsertItem(lListHandle, &lItem);
								}

								// Begin lag test
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 1,
									(char *) MR_LoadStringBuffered(IDS_COMPUTING));
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 2,
									(char *) MR_LoadStringBuffered(IDS_CONNECTING));
								UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);
								mActiveInterface->SendPartyInfo(lClient);
	
								// Start lag test with that connection
								lAnswer.mMessageType = MRNM_LAG_TEST;
								lAnswer.mDataLen = 4;
								*(DWORD *) lAnswer.mData = timeGetTime();

								mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
								AppendHandshakeLog("slot=%d send MRNM_LAG_TEST stamp=%u", lClient,
									*(DWORD *) lAnswer.mData);
								// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );
	
								// Start a time-out timer because the request may fail
								SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);	
							}
							break;

						case MRNM_REMOVE_ENTRY:
							TRACE("MRNM_REMOVE_ENTRY ");

							if(lBuffer->mDataLen >= 1) {
								int lRemovedClient = lBuffer->mData[0];
								int lRemovedSlot;

								if(lRemovedClient > mActiveInterface->mId) {
									lRemovedSlot = lRemovedClient - 1;
								}
								else {
									lRemovedSlot = lRemovedClient;
								}

								if((lRemovedSlot >= 0) && (lRemovedSlot < eMaxClient)) {
									mActiveInterface->CleanupClientState(lRemovedSlot, pWindow);
									mActiveInterface->SendConnectionDoneIfNeeded();
								}
							}
							break;

						case MRNM_CLIENT_ADDR: // server sent us a list of other clients and addresses
							{
								TRACE("MRNM_CLIENT_ADDR ");

								if(lBuffer->mDataLen == 0) {
									TRACE("END_OF_LIST ");

									// end of list
									mActiveInterface->mAllPreLoguedRecv = TRUE;
									mActiveInterface->mPreLoguedClient[0] = TRUE;
	
									// Start a lag test with the server
									// * Already done *
	
									// Report that all timing tests have been done.. if needed
									mActiveInterface->SendConnectionDoneIfNeeded();
								}
								else {
									// The server sent us a new client address
									// Create a connection to that port
									SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

									ASSERT(lNewSocket != INVALID_SOCKET);

									SOCKADDR_IN lAddr;
	
									int lSlot = -1;
	
									for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
										if(!mActiveInterface->mClient[lCounter].IsConnected()) {
											lSlot = lCounter;
											break;
										}
									}
									ASSERT(lSlot != -1);

									TRACE("MORE_CLIENTS lSlot %d ", lSlot);

									CSteamID lSteamID = CSteamID(*(uint64 *) &(lBuffer->mData[12]));

									mActiveInterface->mClient[lSlot].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket, lSteamID, *(BOOL *) &(lBuffer->mData[20]));
									mActiveInterface->mPreLoguedClient[lSlot] = TRUE;

									lAddr.sin_family = AF_INET;
									lAddr.sin_addr.s_addr = *(int *) &(lBuffer->mData[0]);
									lAddr.sin_port = *(int *) &(lBuffer->mData[8]);
									
									// save info
									// backup address will be attempted to be used if connection fails
									mActiveInterface->mClientAddr[lSlot] = *(int *) &(lBuffer->mData[0]);
									mActiveInterface->mClientBkAddr[lSlot] = *(int *) &(lBuffer->mData[4]);
									mActiveInterface->mClientPort[lSlot] = *(int *) &(lBuffer->mData[8]);

									// allow messages from the new client (under FD_CONNECT)
									WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lSlot, FD_CONNECT | FD_READ | FD_CLOSE);
									WSAAsyncSelect(mActiveInterface->mClient[lSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lSlot, FD_READ | FD_CLOSE);

									// connect to the new client
									int lCode = mActiveInterface->Connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr), lSteamID, STM_NEW_CLIENT, FD_CONNECT | FD_READ | FD_CLOSE);

									// set timeout timer
									SetTimer(pWindow, lSlot + 20, MR_CLIENT_RETRY_TIME, NULL);
								}
							}
							break;

						case MRNM_LAG_TEST: // a client we are connecting to is conducting a lag test
							TRACE("MRNM_LAG_TEST ");
							AppendHandshakeLog("slot=%d recv MRNM_LAG_TEST stamp=%d", lClient,
								*(int *) &(lBuffer->mData[0]));

							// return the message and add the current time
							lAnswer.mMessageType = MRNM_LAG_ANSWER;
							lAnswer.mDataLen = 4;
							*(int *) &(lAnswer.mData[0]) = *(int *) &(lBuffer->mData[0]);

							// send the request
							mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
							// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );

							break;

						case MRNM_LAG_ANSWER: // lag test returned
							TRACE("MRNM_LAG_ANSWER ");
							AppendHandshakeLog("slot=%d recv MRNM_LAG_ANSWER stamp=%d", lClient,
								*(int *) &(lBuffer->mData[0]));

							// Desactivate time-out
							KillTimer(pWindow, lClient + 10);

							// Verify that it is not a late message
							if(mActiveInterface->mClient[lClient].LagDone()) {
								// That is a late message.. ignore it
								TRACE("Late ping message B %d\n", lClient);
							}
							else {
								// Add the sample to the list
								if(!mActiveInterface->mClient[lClient].AddLagSample(timeGetTime() - *(int *) &(lBuffer->mData[0]))) {
									// Start a time-out timer because the request may fail
									SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);

									// send a new request
							lAnswer.mMessageType = MRNM_LAG_TEST;
							lAnswer.mDataLen = 4;
							*(int *) &(lAnswer.mData[0]) = timeGetTime();
	
							mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
							AppendHandshakeLog("slot=%d retry MRNM_LAG_TEST stamp=%d", lClient,
								*(int *) &(lAnswer.mData[0]));
									// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );

								}
								else { // we have 5 lag samples, that's all we need
									// ASSERT( mActiveInterface->mAllPreLoguedRecv ); it is not an important assert.. it only mean that the client list is not completely received
									ASSERT(!mActiveInterface->mServerMode);

									// Send result to remote side
									lAnswer.mMessageType = MRNM_LAG_INFO;
									lAnswer.mDataLen = 8;
									*(int *) &(lAnswer.mData[0]) = mActiveInterface->mClient[lClient].GetAvgLag();
									*(int *) &(lAnswer.mData[4]) = mActiveInterface->mClient[lClient].GetMinLag();
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
									AppendHandshakeLog("slot=%d send MRNM_LAG_INFO avg=%d min=%d", lClient,
										mActiveInterface->mClient[lClient].GetAvgLag(),
										mActiveInterface->mClient[lClient].GetMinLag());

									// Update display
									char lStrBuffer[20];

									ListView_SetItemText(lListHandle,
										GetTcpListRowForClient(mActiveInterface, lClient), 1,
										_itoa(mActiveInterface->mClient[lClient].GetAvgLag(),
											lStrBuffer, 10));
									ListView_SetItemText(lListHandle,
										GetTcpListRowForClient(mActiveInterface, lClient), 2,
										(char *) MR_LoadStringBuffered(IDS_CONNECTED));
									UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);

									// Verify if weconnected with all the client that we were suppose to
									mActiveInterface->SendConnectionDoneIfNeeded();
								}
							}
							break;

						case MRNM_LAG_INFO: // we are being sent lag info
							{
								TRACE("MRNM_LAG_INFO ");
								AppendHandshakeLog("slot=%d recv MRNM_LAG_INFO avg=%d min=%d", lClient,
									*(int *) &(lBuffer->mData[0]), *(int *) &(lBuffer->mData[4]));

								mActiveInterface->mClient[lClient].SetLag(*(int *) &(lBuffer->mData[0]), *(int *) &(lBuffer->mData[4]));
	
								// Update display
								char lStrBuffer[20];
	
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 1,
									_itoa(mActiveInterface->mClient[lClient].GetAvgLag(),
										lStrBuffer, 10));
								if(mActiveInterface->mServerMode) {
									ListView_SetItemText(lListHandle,
										GetTcpListRowForClient(mActiveInterface, lClient), 2,
										(char *) MR_LoadStringBuffered(IDS_WAITING_ACK));
								}
								else {
									ListView_SetItemText(lListHandle,
										GetTcpListRowForClient(mActiveInterface, lClient), 2,
										(char *) MR_LoadStringBuffered(IDS_CONNECTED));
								}
								UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);
							}
							break;

						case MRNM_CONNECTION_DONE: // the client has told us they are connected to everyone
							{
								TRACE("MRNM_CONNECTION_DONE ");
								AppendHandshakeLog("slot=%d recv MRNM_CONNECTION_DONE", lClient);

								ASSERT(mActiveInterface->mServerMode);
	
								// Mark the connection as completed
								ListView_SetItemText(lListHandle,
									GetTcpListRowForClient(mActiveInterface, lClient), 2,
									(char *) MR_LoadStringBuffered(IDS_CONNECTED));
								UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);
								mActiveInterface->mConnected[lClient] = TRUE;
							}
							break;

						case MRNM_READY: // race is beginning
							TRACE("MRNM_READY ");

							// Get Client Id
							mActiveInterface->mId = lBuffer->mData[0];

							// Quit this dialog with success

							// Remove all ports callback
							for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
								if(mActiveInterface->mClient[lCounter].IsConnected()) {
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetSocket(), pWindow, MRM_CLIENT + lCounter, 0);
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetUDPSocket(), pWindow, MRM_CLIENT + lCounter, 0);
								}

							}
							WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_CLIENT, 0);

							if(mActiveInterface->mReturnMessage == 0) {
								EndDialog(pWindow, IDOK);
							}
							else {
								SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
								DestroyWindow(pWindow);
							}

							return TRUE;

						case MRNM_CANCEL_GAME: // game has been cancelled
							TRACE("MRNM_CANCEL_GAME ");

							// notify the user that the game ended
							MessageBox(pWindow, MR_LoadString(IDS_GAME_CANCELLED), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);

							mActiveInterface->Disconnect();
							lReturnValue = TRUE;

							if(mActiveInterface->mReturnMessage == 0) {
								EndDialog(pWindow, IDCANCEL);
							}
							else {
								SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDCANCEL, 0);
								DestroyWindow(pWindow);
							}
							break;

						case MRNM_SET_PLAYER_ID:
							TRACE("MRNM_SET_PLAYER_ID ");

							// set mId
							mActiveInterface->mId = lBuffer->mData[0];
							break;

						case MRNM_PARTY_INFO:
							TRACE("MRNM_PARTY_INFO ");

							if(lBuffer->mDataLen >= 2) {
								const int lPartySize = max(1,
									min((int) lBuffer->mData[0], MR_MAX_LOCAL_PLAYER));
								const int lPartyIndex = lBuffer->mData[1];
								CString lPartyName((const char *) (lBuffer->mData + 2),
									lBuffer->mDataLen - 2);

								mActiveInterface->mClientPartySize[lClient] = lPartySize;
								if((lPartyIndex >= 0) && (lPartyIndex < MR_MAX_LOCAL_PLAYER)) {
									mActiveInterface->mClientPartyNames[lClient][lPartyIndex] = lPartyName;
									if((lPartyIndex == 0) && !lPartyName.IsEmpty()) {
										mActiveInterface->mClientName[lClient] = lPartyName;
									}
									UpdateTcpListRowsForClient(lListHandle, mActiveInterface, lClient);
								}
							}
							break;
					}
				}

				if(mActiveInterface->mClient[lClient].IsConnected()) {
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);
				}

				break;

			case FD_CONNECT: // we have successfully connected to the server or another client
				TRACE("\n%s: [ListCallBack] FD_CONNECT ", mActiveInterface->GetPlayerName());
				KillTimer(pWindow, lClient + 20); // kill timeout timer

				int param = mActiveInterface->mClient[lClient].mSocketConnected && !mActiveInterface->mClient[lClient].mSteamOnly ? WSAGETSELECTEVENT(pLParam) : pLParam;

				if(WSAGETSELECTERROR(pLParam)) {
					// Connection error with a client
					//ASSERT(FALSE);
					TRACE("CONNECTION_ERROR %d ", lClient);

					switch(WSAGETSELECTERROR(pLParam)) {
						case WSAECONNREFUSED:
							TRACE("WSAECONNREFUSED ");
							// assemble our message
							{
								char *lErrorString = new char[120]; // max player length is 40
								sprintf(lErrorString, "%s%s%s", 
									MR_LoadString(IDS_CONN_REFUSED1),
									mActiveInterface->mClientName[lClient],
									MR_LoadString(IDS_CONN_REFUSED2));

                                MessageBox(pWindow, lErrorString, MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);

								delete lErrorString;
							}
							break;
						case WSAENETUNREACH:
						case WSAETIMEDOUT:
							/* try a different IP if possible */
							/* but not if connecting to server */
							TRACE("WSAENETUNREACH/WSAETIMEDOUT ");
							if(!mActiveInterface->mClient[lClient].mTriedBackupIP && lClient != 0) {
								mActiveInterface->mClient[lClient].Disconnect();
								mActiveInterface->mClient[lClient].mTriedBackupIP = TRUE;

								SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

								ASSERT(lNewSocket != INVALID_SOCKET);

								SOCKADDR_IN lAddr;
								lAddr.sin_family = AF_INET;

								mActiveInterface->mClient[lClient].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket, sSteamID, mActiveInterface->mSteamOnly);

								if(((mActiveInterface->mClientAddr[lClient] & 0x0000FFFF) == 0x0000A8C0) ||
								   ((mActiveInterface->mClientAddr[lClient] & 0x000000FF) == 0x0000000A)) {
									/* Our server reported an internal address:
									 * This probably means that the server and this client are on the
									 * same internal network.  If this is the case, we need to use the IP
									 * we used for the server, but make sure we are connecting on a different
									 * port.
									 */
									if(mActiveInterface->mClientPort[0] != mActiveInterface->mClientPort[lClient]) {
										lAddr.sin_addr.s_addr = *(int *) &(mActiveInterface->mClientAddr[0]);
										lAddr.sin_port = *(int *) &(mActiveInterface->mClientPort[lClient]);
									} else {
										/* assemble error message */
										char *lErrorMessage = new char[120]; // max length of character name is 40
										sprintf(lErrorMessage, "%s%s%s",
											MR_LoadString(IDS_SAMEADDRPORT1),
											mActiveInterface->mClientName[lClient],
											MR_LoadString(IDS_SAMEADDRPORT2));

										MessageBox(pWindow, lErrorMessage, MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);

										delete lErrorMessage;
									}
								} else {
									lAddr.sin_addr.s_addr = *(int *) &(mActiveInterface->mClientBkAddr[lClient]);
									// we try to use the address at lBuffer->mData[4] if we can't connect with the IP the server saw
									lAddr.sin_port = *(int *) &(mActiveInterface->mClientPort[lClient]);
								}

								TRACE("Trying new IP %08x, port %d\n", lAddr.sin_addr.s_addr, lAddr.sin_port);

								// allow messages from the new client (under FD_CONNECT)
								WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);
								WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);

								// connect to the new client
								int lCode = mActiveInterface->Connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr), sSteamID, MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);
							} else {
								char *lErrorString = new char[120]; // max player length is 40
								sprintf(lErrorString, "%s%s%s",
									MR_LoadString(IDS_CONN_TIMEOUT1),
									mActiveInterface->mClientName[lClient],
									MR_LoadString(IDS_CONN_TIMEOUT2));

								MessageBox(pWindow, lErrorString, MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);

								delete lErrorString;
							}
							break;
						default:
							TRACE("UNKNOWN ");
							{
								char *lError = new char[120];
								sprintf(lError, "%s%s.",
									MR_LoadString(IDS_CONN_ERROR),
									mActiveInterface->mClientName[lClient]);
                                MessageBox(pWindow, lError, MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);
								delete lError;
							}
							break;
					}
				}
				else {
					TRACE("\n%s: [ListCallBack] MRNM_CONN_NAME_GET_SET ", mActiveInterface->GetPlayerName());

					// request client name to start the connection sequence
					// also include UDP port number in the request
					lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
					lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 5;
					*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);
					lAnswer.mData[4] = (MR_UInt8) max(1,
						min(mActiveInterface->mLocalPartySize, MR_MAX_LOCAL_PLAYER));

					memcpy(lAnswer.mData + 5, mActiveInterface->mPlayer, lAnswer.mDataLen - 5);

					mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
					mActiveInterface->SendPartyInfo(lClient);
				}

				break;
		}
		lReturnValue = TRUE;
	}

	return lReturnValue;
}

/**
 * This function sends the MRNM_CONNECTION_DONE message to the server, if we have successfully connected to all other clients.  The message is not
 * sent if any clients are not yet finished with connecting.
 */
void MR_NetworkInterface::SendConnectionDoneIfNeeded()
{
	MR_NetMessageBuffer lAnswer;

	if(mAllPreLoguedRecv) {
		BOOL lAllDone = TRUE;
		for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
			if(mPreLoguedClient[lCounter]) {
				if(!mClient[lCounter].LagDone()) {
					lAllDone = FALSE;
					break;
				}
			}
		}

		if(lAllDone) {
			lAnswer.mMessageType = MRNM_CONNECTION_DONE;
			lAnswer.mClient = mId;
			lAnswer.mDataLen = 0;
			AppendHandshakeLog("send MRNM_CONNECTION_DONE to server");
			mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
		}
		else {
			AppendHandshakeLog("hold MRNM_CONNECTION_DONE allPre=%d", mAllPreLoguedRecv);
		}
	}
	else {
		AppendHandshakeLog("hold MRNM_CONNECTION_DONE waiting for prelogued list");
	}
}

void MR_NetworkInterface::SendPartyInfo(int pClient)
{
	if((pClient < 0) || (pClient >= eMaxClient) || !mClient[pClient].IsConnected()) {
		return;
	}

	for(int i = 0; i < mLocalPartySize; ++i) {
		MR_NetMessageBuffer lAnswer;
		const CString &lName = mLocalPartyNames[i];

		lAnswer.mMessageType = MRNM_PARTY_INFO;
		lAnswer.mClient = mId;
		lAnswer.mDataLen = 2 + lName.GetLength();
		lAnswer.mData[0] = (MR_UInt8) mLocalPartySize;
		lAnswer.mData[1] = (MR_UInt8) i;
		if(lName.GetLength() > 0) {
			memcpy(lAnswer.mData + 2, (const char *) lName, lName.GetLength());
		}

		mClient[pClient].Send(&lAnswer, MR_NET_REQUIRED);
	}
}

/**
 * Initialize the sockets to INVALID_SOCKETs.  Also run Disconnect().
 */
MR_NetworkPort::MR_NetworkPort()
{
	mSocket = INVALID_SOCKET;
	mUDPRecvSocket = INVALID_SOCKET;
	mTriedBackupIP = FALSE;
	mSocketConnected = TRUE;

	Disconnect();
}

/**
 * Disconnect before destruction.
 */
MR_NetworkPort::~MR_NetworkPort()
{
	Disconnect();
}

/**
 * Connects to the socket passed as a parameter.  The socket passed should already be set up and connected to the client.  Also sets up the
 * UDP receive socket to receive messages from the client.
 *
 * @param pSocket An already-connected socket.
 */
void MR_NetworkPort::Connect(SOCKET pSocket, SOCKET pUDPRecvSocket, CSteamID pSteamID, BOOL pSteamOnly)
{
	Disconnect();
	mSocket = pSocket;
	mSteamID = pSteamID;
	mWatchdog = timeGetTime();
	mSteamOnly = pSteamOnly;

	mUDPRecvSocket = pUDPRecvSocket;

	/*
	// Create UDPPort

	// Create a new UDP Socket for the reception
	mUDPRecvSocket = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPRecvSocket != INVALID_SOCKET);

	// Set this port as non-blocking
	unsigned long lNonBlock = TRUE;
	int lCode = ioctlsocket(mUDPRecvSocket, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// Bind the socket to any available address
	SOCKADDR_IN lLocalAddr;
	lLocalAddr.sin_family = AF_INET;
	lLocalAddr.sin_addr.s_addr = INADDR_ANY;
	lLocalAddr.sin_port = 0;
	lCode = bind(mUDPRecvSocket, (LPSOCKADDR) &lLocalAddr, sizeof(lLocalAddr));
	ASSERT(lCode != SOCKET_ERROR);
	*/
}

/**
 * Set the remote UDP port.
 *
 * @param pPort Remote port to use for UDP
 */
void MR_NetworkPort::SetRemoteUDPPort(unsigned int pPort)
{
	// Determine remote address and port based on the TCP port
	int lSize = sizeof(mUDPRemoteAddr);
	int lCode = getpeername(mSocket, (LPSOCKADDR) &mUDPRemoteAddr, &lSize);

	if (!mSteamOnly) {
		ASSERT(lCode != SOCKET_ERROR);
	}

	mUDPRemoteAddr.sin_port = pPort;
}

void MR_NetworkPort::DisconnectSteam(BOOL pDisconnectSteam)
{
	if(mSteamID.IsValid() && pDisconnectSteam) {
		SteamNetworking()->CloseP2PSessionWithUser(mSteamID);
		mSteamID = CSteamID();
	}	
}

/**
 * Close the main socket and UDP receive socket, and reset variables.  Default lag is 300.  Not sure why.
 */
void MR_NetworkPort::Disconnect()
{
	if(mSocket != INVALID_SOCKET) {
		closesocket(mSocket);
	}

	// Don't close the UDP recv socket!
	
	//if(mUDPRecvSocket != INVALID_SOCKET) {
	//	closesocket(mUDPRecvSocket);
	//}

	mSocket = INVALID_SOCKET;

	mUDPRecvSocket = INVALID_SOCKET;

	// Keep the Steam ID too!

	// mSteamID = CSteamID();

	mLastSendedDatagramNumber[0] = 0;
	mLastSendedDatagramNumber[1] = 0;
	mLastSendedDatagramNumber[2] = 0;
	mLastSendedDatagramNumber[3] = 0;
	mLastReceivedDatagramNumber[0] = 0;
	mLastReceivedDatagramNumber[1] = 0;
	mLastReceivedDatagramNumber[2] = 0;
	mLastReceivedDatagramNumber[3] = 0;
	mLastClient[0] = MR_ID_NOT_SET;
	mLastClient[1] = MR_ID_NOT_SET;
	mLastClient[2] = MR_ID_NOT_SET;
	mLastClient[3] = MR_ID_NOT_SET;

	mAvgLag = 300;
	mMinLag = 300;
	mNbLagTest = 0;
	mTotalLag = 0;

	mOutQueueLen = 0;
	mOutQueueHead = 0;

	mInputMessageBufferIndex = 0;

	mTriedBackupIP = FALSE;
	mSocketConnected = TRUE;
}

/**
 * Tests if the port is connected (with mSocket).
 */
BOOL MR_NetworkPort::IsConnected() const
{
	return (mSocket != INVALID_SOCKET || mSteamID.IsValid());
}

/**
 * Returns the TCP socket used for communication with the client.
 */
SOCKET MR_NetworkPort::GetSocket() const
{
	return mSocket;
}

/**
 * Returns the UDP socket that is being used for listening.
 */
SOCKET MR_NetworkPort::GetUDPSocket() const
{
	return mUDPRecvSocket;
}

/**
 * Returns the Steam ID.
 */
CSteamID MR_NetworkPort::GetSteamId() const
{
	return mSteamID;
}

/**
 * Check if any new messages have been recieved.  If not, NULL is returned.  Otherwise the first message in the queue is returned.  The UDP socket is
 * checked before the TCP socket (TCP messages are not as time-critical).
 *
 * The parameter pClientId is a dirty hack for the one-port UDP hack.  Since this polls the global UDP receive port it is possible we could receive a
 * message not addressed to this particular port; therefore, we peek at the message and make sure it has the same ID as pClientId, or, we ignore it.
 *
 * @param pClientId Id of the client a packet should be from
 */
const MR_NetMessageBuffer *MR_NetworkPort::Poll(int pClientId, BOOL pCheckClientId)
{
	// Socket is assumed to be non-blocking, but it damn well better be because we set it that way
	if((mInputMessageBufferIndex == 0) && (mUDPRecvSocket != INVALID_SOCKET || mSteamOnly)) {
		while(1) {
			// see if there is a UDP packet addressed to us
			// this is a dirty hack
			int lLen = 0;
			uint32 msgSize = 0;
			BOOL lPassCheck = TRUE;
			if(pCheckClientId == TRUE && !mSteamOnly) {
				lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), MSG_PEEK);
				if((mInputMessageBuffer.mClient != pClientId) && (mInputMessageBuffer.mClient != MR_ID_NOT_SET)) {
					/* disable this for intensive testing
					sprintf(lErrorBuffer, "Ignoring packet from client %d (looking for %d), message %d\n",
								mInputMessageBuffer.mClient,
								pClientId,
								mInputMessageBuffer.mMessageType);
					OutputDebugString((LPCTSTR) lErrorBuffer);
					*/
					lPassCheck = FALSE;
				} else
					lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), 0);
			}
			else {
				if ( SteamNetworking()->IsP2PPacketAvailable( &msgSize, pClientId ) )
				{
					CSteamID steamIDRemote;
					uint32 bytesRead = 0;
					lLen = (int) msgSize;
					SteamNetworking()->ReadP2PPacket( &mInputMessageBuffer, msgSize, &bytesRead, &steamIDRemote, pClientId );
				} else {
					lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), 0);
				}
			}

			if(lLen > 0 && lPassCheck == TRUE) {
				int lQueueId = mInputMessageBuffer.mDatagramQueue;
				// We have received a datagram
				ASSERT(lLen == mInputMessageBuffer.mDataLen + MR_NET_HEADER_LEN);

				// Eliminate duplicate and late datagrams
				if(lLen == mInputMessageBuffer.mDataLen + MR_NET_HEADER_LEN && ((MR_Int8) ((MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber - (MR_Int8) mLastReceivedDatagramNumber[lQueueId]) > 0)
					|| (mInputMessageBuffer.mClient != mLastClient[lQueueId])) {
					// TRACE( "UDP recv\n" );
					mLastReceivedDatagramNumber[lQueueId] = mInputMessageBuffer.mDatagramNumber;
					mLastClient[lQueueId] = mInputMessageBuffer.mClient;

					mWatchdog = timeGetTime();
					// TRACE("UDP recv: client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
					return &mInputMessageBuffer;
				}
				else {
					// TRACE("Late UDP %d %d client %d\n", (MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber, (MR_Int8) mLastReceivedDatagramNumber[lQueueId], mInputMessageBuffer.mClient);
				}
			}
			else {
				// No data... try TCP
				break;
			}
		}
	}

	// check for TCP packets
	if(mSocket != INVALID_SOCKET || mSteamOnly) {
		if (mSteamOnly) {
			uint32 msgSize = 0;
			if ( SteamNetworking()->IsP2PPacketAvailable( &msgSize, pClientId + MR_NetworkInterface::eMaxClient + 1 ) )
			{
				TRACE("Found TCP packet from client %d\n", pClientId + MR_NetworkInterface::eMaxClient);
				CSteamID steamIDRemote;
				uint32 bytesRead = 0;
				mInputMessageBufferIndex = 0;
				mWatchdog = timeGetTime();
				SteamNetworking()->ReadP2PPacket( &mInputMessageBuffer, msgSize, &bytesRead, &steamIDRemote, pClientId + MR_NetworkInterface::eMaxClient + 1 );
				return &mInputMessageBuffer;
			}
		} else {
			if(mInputMessageBufferIndex < MR_NET_HEADER_LEN) {
				// Try to read message header
				int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, MR_NET_HEADER_LEN - mInputMessageBufferIndex, 0);

				if(lLen > 0) {
					mInputMessageBufferIndex += lLen;
					// TRACE("Begin recv, client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
				}
			}

			if(mInputMessageBufferIndex >= MR_NET_HEADER_LEN && (mInputMessageBuffer.mDataLen > 0)) {
				int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, mInputMessageBuffer.mDataLen - (mInputMessageBufferIndex - MR_NET_HEADER_LEN), 0);

				if(lLen > 0) {
					mInputMessageBufferIndex += lLen;
					// TRACE("Continue recv, client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
				}
			}

			if((mInputMessageBufferIndex >= MR_NET_HEADER_LEN) && (mInputMessageBufferIndex == (MR_NET_HEADER_LEN + mInputMessageBuffer.mDataLen))) {
				mInputMessageBufferIndex = 0;
				mWatchdog = timeGetTime();
				// TRACE("Done receiving packet from client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
				return &mInputMessageBuffer;
			}
		}

		if((mLastSendedDatagramNumber[1] > 16) && (timeGetTime() - mWatchdog) > MR_CONNECTION_TIMEOUT) {
			// (mLastSendedDatagramNumber[1]>16) -- this condition avoid disconnecting
			//                                   -- before game start
			// TRACE("Reception Timeout %d %d\n", timeGetTime() - mWatchdog, mInputMessageBufferIndex);

			if(mInputMessageBufferIndex != 0) {
				// TRACE("Message: %d %d\n", mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDataLen);
			}

			Disconnect();
		}
	}

	return NULL;
}

/**
 * Send a packet via UDP, using the supplied socket.
 *
 * @param pSocket The socket the packet will be sent from (generally MR_NetworkInterface::mUDPOutLongPort or MR_NetworkInterface::mUDPOutShortPort)
 * @param pMessage The message to be sent
 * @param pQueueId The queue ID of the message
 * @param pResendLast TRUE if this is a re-send of the last packet
 */
BOOL MR_NetworkPort::UDPSend(SOCKET pSocket, MR_NetMessageBuffer *pMessage, unsigned pQueueId, BOOL pResendLast, int pClient)
{
	BOOL lReturnValue = TRUE;

	// Debug lost packet simulation
	/*
	   if( (rand()%3) != 1 )
	   {
	   return TRUE;
	   }
	 */

	if(mUDPRecvSocket != INVALID_SOCKET || mSteamID.IsValid()) {
		int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;

		if(!pResendLast) {
			mLastSendedDatagramNumber[pQueueId]++;
		}

		pMessage->mDatagramQueue = pQueueId;
		pMessage->mDatagramNumber = mLastSendedDatagramNumber[pQueueId];

		int lSent = 0;

		if(!mSteamOnly) {
			int lSent = sendto(pSocket, ((const char *) pMessage), lToSend, 0, (LPSOCKADDR) & mUDPRemoteAddr, sizeof(mUDPRemoteAddr));
		} else {
			SteamNetworking()->SendP2PPacket(mSteamID, ((const char *) pMessage), lToSend, k_EP2PSendUnreliable, (pMessage->mMessageType >= 40 ? STM_IMR_CHANNEL : pClient));

			// TRACE("UDPSend: sending to %d of type %d and queue id %d\n", mSteamID.GetAccountID(), pMessage->mMessageType, pMessage->mDatagramQueue);
		}

		lReturnValue = (lSent != SOCKET_ERROR);
	}
	return lReturnValue;
}

/**
 * Send the given message via TCP.
 *
 * @param pMessage The message to be sent
 * @param pReqLevel Should be set to MR_NET_REQUIRED
 */
void MR_NetworkPort::Send(const MR_NetMessageBuffer *pMessage, int pReqLevel, int pClient)
{
	// First try to send buffered data
	if(mSocket != INVALID_SOCKET || mSteamID.IsValid()) {
		BOOL lEndQueueLoop = (mOutQueueLen == 0);

		while(!lEndQueueLoop) {
			// Determine first batch to send
			int lToSend;

			if((mOutQueueHead + mOutQueueLen) > MR_OUT_QUEUE_LEN) {
				lToSend = MR_OUT_QUEUE_LEN - mOutQueueHead;
			}
			else {
				lToSend = mOutQueueLen;
				lEndQueueLoop = TRUE;
			}

			int lReturnValue = 0;

			if(mSocket != INVALID_SOCKET && mSocketConnected && !mSteamOnly) {
				lReturnValue = send(mSocket, (const char *) (mOutQueue + mOutQueueHead), lToSend, 0);
			} else {
				// TRACE(" --- MR_NetworkPort::Send  --- \n");
				// TRACE("Queue Message to %d of type %d\n", mSteamID.GetAccountID(), int(((MR_NetMessageBuffer *) (mOutQueue + mOutQueueHead))->mMessageType));
				lReturnValue = SteamNetworking()->SendP2PPacket(mSteamID, (const char *) (mOutQueue + mOutQueueHead), lToSend, k_EP2PSendReliable, (pMessage->mMessageType >= 40 ? STM_IMR_CHANNEL : pClient)) ? lToSend : SOCKET_ERROR;

				TRACE("Send: sending to %d of type %d and queued\n", mSteamID.GetAccountID(), pMessage->mMessageType);
			}

			if(lReturnValue >= 0) {
				mOutQueueLen -= lReturnValue;

				mOutQueueHead = (mOutQueueHead + lReturnValue) % MR_OUT_QUEUE_LEN;

				if(lReturnValue != lToSend) {
					lEndQueueLoop = TRUE;
				}
			}
			else {
				lEndQueueLoop = TRUE;

				if((mSocket != INVALID_SOCKET && mSocketConnected && WSAGetLastError() == WSAEWOULDBLOCK) || mSteamID.IsValid()) {
					// nothing to do
				}
				else {
					TRACE("Communication error A %d\n", WSAGetLastError());
					Disconnect();
				}
			}
		}
	}

	if(mSocket != INVALID_SOCKET || mSteamID.IsValid()) {
		int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;

		int lReturnValue;						  // send return value

		if(mOutQueueLen > 0) {
			lReturnValue = 0;
		}
		else {
			if(mSocket != INVALID_SOCKET && mSocketConnected && !mSteamOnly) {
				lReturnValue = send(mSocket, ((const char *) pMessage), lToSend, 0);
			} else {
				// TRACE(" --- MR_NetworkPort::Send  --- \n");
				TRACE("Send: Message to %d of type %d and client %d on channel %d\n", mSteamID.GetAccountID(), pMessage->mMessageType, pMessage->mClient, (pMessage->mMessageType >= 40 ? STM_IMR_CHANNEL : pClient));
				lReturnValue = SteamNetworking()->SendP2PPacket(mSteamID, ((const char *) pMessage), lToSend, k_EP2PSendReliable, (pMessage->mMessageType >= 40 ? STM_IMR_CHANNEL : pClient)) ? lToSend : SOCKET_ERROR;
			}

			// TRACE( "Send %d %d %d\n", lToSend, lToSend-lSent, lReturnValue );

			if((lReturnValue == -1) && (WSAGetLastError() == WSAEWOULDBLOCK)) {
				ASSERT(FALSE);
				lReturnValue = 0;
			}
		}

		if(lReturnValue < 0) {
			TRACE("Communication error B %d\n", WSAGetLastError());
			Disconnect();
		}
		else if(lReturnValue != lToSend) {
			if((lReturnValue > 0) || (pReqLevel == MR_NET_REQUIRED) || ((pReqLevel == MR_NET_TRY) && (mOutQueueLen < (MR_OUT_QUEUE_LEN / 4)))) {
				lToSend -= lReturnValue;

				if(lToSend > (MR_OUT_QUEUE_LEN - mOutQueueLen)) {
					// no space left
					TRACE("Output queue full\n");
					Disconnect();
					ASSERT(FALSE);
				}
				else {
					// put the remaining part of the message in the queue
					int lTail = (mOutQueueHead + mOutQueueLen) % MR_OUT_QUEUE_LEN;

					int lFirstBlocSize = min(lToSend, MR_OUT_QUEUE_LEN - lTail);
					int lSecondBlocSize = max(0, lToSend - lFirstBlocSize);

					ASSERT(lFirstBlocSize > 0);

					memcpy(mOutQueue + lTail, pMessage + lReturnValue, lFirstBlocSize);

					if(lSecondBlocSize > 0) {
						memcpy(mOutQueue, pMessage + lFirstBlocSize + lReturnValue, lSecondBlocSize);
					}

					mOutQueueLen += lToSend;
				}
			}
		}
	}
}

/**
 * Adds an observed lag sample to the lag.  Used in lag testing (if you hadn't guessed).
 *
 * @param pLag Observed lag
 */
BOOL MR_NetworkPort::AddLagSample(int pLag)
{
	mNbLagTest++;

	mTotalLag += pLag;

	mAvgLag = mTotalLag / (mNbLagTest * 2);

	mMinLag = min(mMinLag, pLag / 2);

	return LagDone();
}

/**
 * Returns whether or not five lag tests have been performed.
 */
BOOL MR_NetworkPort::LagDone() const
{
	return (mNbLagTest >= 5);
}

/**
 * Returns the calculated average lag.
 */
int MR_NetworkPort::GetAvgLag() const
{
	return mAvgLag;
}

/**
 * Returns the minimum observed lag.
 */
int MR_NetworkPort::GetMinLag() const
{
	return mMinLag;
} 

/**
 * Sets the lag.
 *
 * @param pAvgLag The average lag
 * @param pMinLag The minimum lag
 */
void MR_NetworkPort::SetLag(int pAvgLag, int pMinLag)
{
	mAvgLag = pAvgLag;
	mMinLag = pMinLag;

	mNbLagTest = 100;							  // Set as completed

}

void MR_NetworkPort::SetInputMessageBuffer(MR_NetMessageBuffer* pInputMessageBuffer) 
{
	mInputMessageBuffer = *pInputMessageBuffer;
}

MR_NetMessageBuffer MR_NetworkPort::GetInputMessageBuffer()
{
	return mInputMessageBuffer;
}

// Helper functions

/**
 * Returns the local IP in a CString.
 */
CString GetLocalAddrStr(BOOL lSteamOnly)
{
	CString lReturnValue;

	char lHostname[100];
	HOSTENT *lHostEnt;
	int lAdapter = 0;

	gethostname(lHostname, sizeof(lHostname));

	lHostEnt = gethostbyname(lHostname);

	if(lHostEnt != NULL) {
		while(lHostEnt->h_addr_list[lAdapter] != NULL) {
			const unsigned char *lHostAddr = (const unsigned char *) lHostEnt->h_addr_list[lAdapter];
			lAdapter++;

			if(!lReturnValue.IsEmpty()) {
				lReturnValue += " ";
			}
			sprintf(lHostname, "%d.%d.%d.%d", lHostAddr[0], lHostAddr[1], lHostAddr[2], lHostAddr[3]);
			lReturnValue += lHostname;
		}
	}

	if(lAdapter == 0) {
		lReturnValue += " ";
		lReturnValue += MR_LoadString(IDS_NO_NET_CONNECT);
	}

	if(lSteamOnly) {
		lReturnValue = "Peer to Peer via Steam";
	}

	return lReturnValue;
}

/**
 * Return a 32-bit integer denoting the IP of the passed parameter string.
 *
 * @param pName IP in string form ( %d.%d.%d.%d)
 */
MR_UInt32 GetAddrFromStr(const char *pName)
{

	MR_UInt32 lReturnValue;
	unsigned int lNibble[4];

	// Verify if the given string is not a numeric addr

	int lNbField = sscanf(pName, " %d.%d.%d.%d", &(lNibble[0]),
		&(lNibble[1]),
		&(lNibble[2]),
		&(lNibble[3]));

	if((lNbField == 4) && (lNibble[0] != 0)) {
		lReturnValue = lNibble[0]
			+ (lNibble[1] << 8)
			+ (lNibble[2] << 16)
			+ (lNibble[3] << 24);

	}
	else {
		lReturnValue = INADDR_ANY;
	}
	return lReturnValue;
}

/**
 * Callback for generic dialogs.
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
static BOOL CALLBACK DialogProc(HWND pWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					return TRUE;

				case IDOK:
					EndDialog(pWindow, IDOK);
					return TRUE;
			}
	}

	return FALSE;
}

void MR_NetworkInterface::OnP2PSessionRequest(P2PSessionRequest_t *pParam)
{
	if(mGameName != "")
	{
		SteamNetworking()->AcceptP2PSessionWithUser(pParam->m_steamIDRemote);

		TRACE("AcceptP2PSessionWithUser with %d\n", pParam->m_steamIDRemote.GetAccountID());
	}
}

void MR_NetworkInterface::OnP2PSessionFailed(P2PSessionConnectFail_t *pParam)
{
	int lClient = MR_ID_NOT_SET;

	// find if the sender has a client id for MRM_CLIENT use
	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if(mClient[lCounter].GetSteamId() == pParam->m_steamIDRemote) {

			lClient = lCounter;
			break;
		}
	}

	if (lClient != MR_ID_NOT_SET)
	{
		const BOOL lWasPreLogued = mCanBePreLogued[lClient];
		CleanupClientState(lClient, mGameModal);

		if(lWasPreLogued) {
			NotifyClientRemoved(lClient);
		}
	}
}

void MR_NetworkInterface::CheckP2PAvailability() const 
{
	// TRACE("CheckP2PAvailability()\n");

	uint32 msgSize = 0;
	while ( SteamNetworking()->IsP2PPacketAvailable( &msgSize, STM_IMR_CHANNEL ) )
	{
		void *msg = malloc( msgSize );
		CSteamID steamIDRemote;
		uint32 bytesRead = 0;
		if ( SteamNetworking()->ReadP2PPacket( msg, msgSize, &bytesRead, &steamIDRemote, STM_IMR_CHANNEL ) )
		{
			sBuffer = (MR_NetMessageBuffer*) msg;
			sSteamID = steamIDRemote;
			
			// TRACE(" ----- ReadP2PPacket ----- \n");
			// TRACE(" -- Type %d / Client %d\n", sBuffer->mMessageType, sBuffer->mClient);

			int lClient = MR_ID_NOT_SET;

			// find if the sender has a client id for MRM_CLIENT use
			for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
				if(mActiveInterface->mClient[lCounter].GetSteamId() == steamIDRemote) {

					lClient = lCounter;
					break;
				}
			}

			if (sBuffer->mMessageType == STM_SERVER_CONNECT) {
				AppendHandshakeLog("Steam packet STM_SERVER_CONNECT from=%u", steamIDRemote.GetAccountID());
				if((mActiveInterface->mConnectModal != NULL) &&
					IsWindow(mActiveInterface->mConnectModal) &&
					!mActiveInterface->mServerMode) {
					MR_NetworkInterface::WaitGameNameCallBack(mActiveInterface->mConnectModal, MRM_SERVER_CONNECT, 0, 0);
				}
				else {
					AppendHandshakeLog("Steam packet STM_SERVER_CONNECT ignored outside WaitGameName");
				}
			} else if (sBuffer->mMessageType == STM_NEW_CLIENT) {		
				AppendHandshakeLog("Steam packet STM_NEW_CLIENT from=%u", steamIDRemote.GetAccountID());
				MR_NetworkInterface::ListCallBack(mActiveInterface->mGameModal, MRM_NEW_CLIENT, 0, 0);
			} else if ((sBuffer->mMessageType == STM_CLIENT_CONNECT) && (lClient != MR_ID_NOT_SET)) {		
				AppendHandshakeLog("Steam packet STM_CLIENT_CONNECT from=%u slot=%d", steamIDRemote.GetAccountID(), lClient);
				MR_NetworkInterface::ListCallBack(mActiveInterface->mGameModal, MRM_CLIENT + lClient, 0, FD_CONNECT);
			} else if ((sBuffer->mMessageType == STM_CLIENT_CLOSE) && (lClient != MR_ID_NOT_SET)) {		
				AppendHandshakeLog("Steam packet STM_CLIENT_CLOSE from=%u slot=%d", steamIDRemote.GetAccountID(), lClient);
				MR_NetworkInterface::ListCallBack(mActiveInterface->mGameModal, MRM_CLIENT + lClient, 0, FD_CLOSE);
			} else if ((sBuffer->mMessageType == MRNM_GAME_NAME) && (lClient != MR_ID_NOT_SET)) {
				AppendHandshakeLog("Steam packet MRNM_GAME_NAME from=%u slot=%d", steamIDRemote.GetAccountID(), lClient);
				MR_NetworkInterface::WaitGameNameCallBack(mActiveInterface->mConnectModal, MRM_CLIENT + lClient, 0, FD_READ);
			} else if (lClient == MR_ID_NOT_SET) {
				AppendHandshakeLog("Steam packet ignored type=%d from=%u", sBuffer->mMessageType, steamIDRemote.GetAccountID());
				TRACE(" Ignoring Steam message type %d from unknown client\n", sBuffer->mMessageType);
			} else {
				AppendHandshakeLog("Steam packet type=%d from=%u slot=%d", sBuffer->mMessageType, steamIDRemote.GetAccountID(), lClient);
				TRACE(" Message type received %d from client %d\n", sBuffer->mMessageType, lClient);
				MR_NetworkInterface::ListCallBack(mActiveInterface->mGameModal, MRM_CLIENT + lClient, 0, FD_READ);
			}
		}
		free( msg );
	}
}

int MR_NetworkInterface::Connect(SOCKET pS, const sockaddr *pName, int pNamelen, CSteamID pSteamID, UINT lMsg, long lEvent)
{
	if (mSteamOnly) {
		TRACE("::CONNECT From SteamID %d To SteamID %d / Msg %d / Event %d\n", SteamUser()->GetSteamID().GetAccountID(), pSteamID.GetAccountID(), lMsg, lEvent);

		MR_NetMessageBuffer lOutputBuffer;
		lOutputBuffer.mMessageType = lMsg;
		lOutputBuffer.mClient = mActiveInterface->mId;

		SteamNetworking()->SendP2PPacket(pSteamID, &lOutputBuffer, sizeof(lOutputBuffer), k_EP2PSendReliable, STM_IMR_CHANNEL);

		return 0;
	}

	return connect(pS, pName, pNamelen);
}

SOCKET MR_NetworkInterface::Accept(SOCKET pS, sockaddr *pAddr, int *pAddrlen, CSteamID pSteamID, UINT lMsg, long lEvent, int lClient)
{
	if (mSteamOnly) {
		TRACE("::ACCEPT From SteamID %d To SteamID %d / Msg %d / Event %d\n", SteamUser()->GetSteamID().GetAccountID(), pSteamID.GetAccountID(), lMsg, lEvent);

		MR_NetMessageBuffer lOutputBuffer;
		lOutputBuffer.mMessageType = lMsg;
		lOutputBuffer.mClient = mActiveInterface->mId;

		SteamNetworking()->SendP2PPacket(pSteamID, &lOutputBuffer, sizeof(lOutputBuffer), k_EP2PSendReliable, STM_IMR_CHANNEL);

		// MR_NetworkInterface::ListCallBack(GetActiveWindow(), MRM_CLIENT + lClient, 0, FD_CONNECT);

		return INVALID_SOCKET;
	}

	return accept(pS, pAddr, pAddrlen);
}
