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
#include "License.h"
#include "io.h"
#include "../MazeCompiler/TrackCommonStuff.h"
#include "../Util/StrRes.h"


class TrackEntry
{
public:
	CString mFileName;   
	CString mDescription;
	int     mRegistrationMode;
	int     mSortingIndex;   
};

#define TRACK_PATH1 "..\\Tracks\\"
#define TRACK_PATH2 ".\\Tracks\\"
#define TRACK_EXT   ".trk"

// Local functions
static BOOL CALLBACK TrackSelectCallBack(HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam);
static BOOL          ReadTrackEntry(MR_RecordFile *pRecordFile, TrackEntry* pDest, const char* pFileName);
static void          SortList();
static void          ReadTrackList();
static void          CleanList();

static int           CompareFunc(const void *elem1, const void *elem2);


// Local variable
#define MAX_TRACK_ENTRIES   100000
// #define MAX_TRACK_ENTRIES   5

static int  gsSelectedEntry = -1;
static int  gsNbTrack       = 0;

static TrackEntry  gsTrackList[MAX_TRACK_ENTRIES];
static TrackEntry* gsSortedTrackList[MAX_TRACK_ENTRIES];
static int         gsNbLaps;
static BOOL        gsAllowWeapons = FALSE;

MR_RecordFile* MR_TrackOpen(HWND pWindow, const char* pFileName)
{
	MR_RecordFile* lReturnValue = NULL;

	long lHandle;
	struct _finddata_t lFileInfo;
	CString            lPath = TRACK_PATH1;
	
	lHandle = _findfirst(lPath + pFileName + TRACK_EXT, &lFileInfo);

	if(lHandle == -1) {
		lPath = TRACK_PATH2;
		lHandle = _findfirst(lPath + pFileName + TRACK_EXT, &lFileInfo);
	}

	if(lHandle == -1) // because it may have changed
		MessageBox(pWindow, MR_LoadString(IDS_TRK_NOTFOUND), MR_LoadString(IDS_GAME_NAME), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	else {
		_findclose(lHandle);

		lReturnValue = new MR_RecordFile;
		if(!lReturnValue->OpenForRead(lPath + pFileName + TRACK_EXT, TRUE)) {
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

BOOL MR_SelectTrack(HWND pParentWindow, CString& pTrackFile, int& pNbLap, BOOL& pAllowWeapons) {
	BOOL lReturnValue = TRUE;
	gsSelectedEntry = -1;

	// Load the entry list
	 MR_WAIT_CURSOR

	ReadTrackList();
	SortList();

	gsNbLaps = 5; // Default value
	gsAllowWeapons = FALSE;

	if(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TRACK_SELECT), pParentWindow, TrackSelectCallBack) == IDOK) {
		pTrackFile    = gsSortedTrackList[gsSelectedEntry]->mFileName;
		pNbLap        = gsNbLaps;
		pAllowWeapons = gsAllowWeapons;
		lReturnValue  = TRUE;
	}
	else
		lReturnValue = FALSE;
	CleanList();

	return lReturnValue;
}

static BOOL CALLBACK TrackSelectCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam) {
	BOOL lReturnValue = FALSE;
	int  lCounter;

	switch(pMsgId)	{
		// Catch environment modification events
		case WM_INITDIALOG:
			// Init track file list
			for(lCounter = 0; lCounter < gsNbTrack; lCounter++)
				SendDlgItemMessage(pWindow, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)(const char *)(gsSortedTrackList[lCounter]->mFileName));

			SetDlgItemInt(pWindow, IDC_NB_LAP, gsNbLaps, FALSE);
			SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_SETCHECK,BST_CHECKED, 0);  
			SendDlgItemMessage(pWindow, IDC_NB_LAP_SPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));

			if(gsNbTrack > 0)	{
				gsSelectedEntry = 0;            
				SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
				SetDlgItemText(pWindow, IDC_DESCRIPTION, gsSortedTrackList[gsSelectedEntry]->mDescription);
				SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, 0, 0);
			}
			else {
				gsSelectedEntry = -1;
				SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
				SetDlgItemText(pWindow, IDC_DESCRIPTION, MR_LoadString(IDS_NO_SELECT));
				SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, -1, 0);
			}
			lReturnValue = TRUE;
			break;
		case WM_COMMAND:
			switch(LOWORD(pWParam))	{
				case IDC_LIST:
					switch(HIWORD(pWParam))	{
						case LBN_SELCHANGE:
							gsSelectedEntry = SendDlgItemMessage(pWindow, IDC_LIST, LB_GETCURSEL, 0, 0);
							if((gsNbTrack==0) || (gsSelectedEntry == -1)) {
								SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
								SetDlgItemText(pWindow, IDC_DESCRIPTION, MR_LoadString(IDS_NO_SELECT));
							}
							else {
								SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
								SetDlgItemText(pWindow, IDC_DESCRIPTION, gsSortedTrackList[gsSelectedEntry]->mDescription);
							}
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
							MessageBox( pWindow, MR_LoadString( IDS_LAP_RANGE ), MR_LoadString( IDS_GAME_NAME ), MB_ICONINFORMATION|MB_OK|MB_APPLMODAL );
						else
							EndDialog( pWindow, IDOK );
					}
					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

MR_TrackAvail MR_GetTrackAvail(const char* pFileName) {
	MR_TrackAvail lReturnValue = eTrackNotFound;

	long lHandle;
	struct _finddata_t lFileInfo;
	CString            lPath = TRACK_PATH1;
	
	lHandle = _findfirst(lPath + pFileName + TRACK_EXT, &lFileInfo);

	if(lHandle == -1) {
		lPath = TRACK_PATH2;
		lHandle = _findfirst(lPath + pFileName + TRACK_EXT, &lFileInfo);
	}

	if(lHandle != -1)	{
		_findclose(lHandle);

		MR_RecordFile lFile;

		if(!lFile.OpenForRead(lPath + pFileName + TRACK_EXT))
			ASSERT( FALSE );
		else {
			TrackEntry lCurrentEntry;

			if(ReadTrackEntry(&lFile, &lCurrentEntry, pFileName))
				lReturnValue = eTrackAvail;
		}
	}
	return lReturnValue;
}

BOOL ReadTrackEntry(MR_RecordFile *pRecordFile, TrackEntry *pDest, const char *pFileName) {
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

			lArchive >> pDest->mDescription;
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
			}
			else
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
				}
				else
					lReturnValue = TRUE;
			}
		}
	}
	return lReturnValue;
}

void SortList() {
	// Init pointer list   
	if(gsNbTrack > 0) {
		for(int lCounter = 0; lCounter < gsNbTrack; lCounter++)
			gsSortedTrackList[ lCounter ] = &(gsTrackList[lCounter]);
	
		qsort(gsSortedTrackList, gsNbTrack, sizeof(gsSortedTrackList[0]),	CompareFunc);
	}
}

int CompareFunc(const void *elem1, const void *elem2) {
	int lReturnValue = 0;
	const TrackEntry **lElem1 = (const TrackEntry **) elem1;
	const TrackEntry **lElem2 = (const TrackEntry **) elem2;
	
	lReturnValue = (*lElem1)->mSortingIndex - (*lElem2)->mSortingIndex;

	if(lReturnValue == 0)
		lReturnValue = strcmp((*lElem1)->mFileName, (*lElem2)->mFileName);

	return lReturnValue;
}

void ReadTrackList() {
	long lHandle;
	struct _finddata_t lFileInfo;
	CString            lPath = TRACK_PATH1;
	
	CleanList();

	lHandle = _findfirst(lPath + "*" + TRACK_EXT, &lFileInfo);

	if(lHandle == -1)	{
		lPath = TRACK_PATH2;
		lHandle = _findfirst(lPath + "*" + TRACK_EXT, &lFileInfo);
	}

	if(lHandle != -1) {
		do {
			gsTrackList[gsNbTrack].mFileName = CString(lFileInfo.name, strlen(lFileInfo.name) - strlen(TRACK_EXT));

			// Open the file and read aditionnal info
			MR_RecordFile lRecordFile;

			if(!lRecordFile.OpenForRead(lPath + gsTrackList[gsNbTrack].mFileName + TRACK_EXT))
				ASSERT(FALSE);
			else {
				if(ReadTrackEntry(&lRecordFile, &(gsTrackList[gsNbTrack]), NULL))
					gsNbTrack++;
				else
					ASSERT(FALSE);
			}
		}
		while(_findnext(lHandle, &lFileInfo) == 0);

		_findclose(lHandle);
	}
}

void CleanList() {
	for(int lCounter = 0; lCounter < gsNbTrack; lCounter++) {
		gsTrackList[lCounter].mFileName    = "";
		gsTrackList[lCounter].mDescription = "";
	}
	gsNbTrack = 0;
}