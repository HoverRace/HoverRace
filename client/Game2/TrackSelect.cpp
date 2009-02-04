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
#include "../../engine/Util/Cursor.h"
#include "resource.h"
#include "io.h"
#include "../../compilers/MazeCompiler/TrackCommonStuff.h"
#include "../../engine/Util/StrRes.h"
#include "../../engine/Util/Config.h"

#include <algorithm>

class TrackEntry
{
	public:
		std::string mFileName;
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

		bool operator==(const TrackEntry &elem2) const
		{
			return ((mFileName == elem2.mFileName) && 
					(mDescription == elem2.mDescription));
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
static BOOL CALLBACK ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
static BOOL ReadTrackEntry(MR_RecordFile * pRecordFile, TrackEntry * pDest, const char *pFileName);
static bool CompareFunc(const TrackEntry *ent1, const TrackEntry *ent2);
static bool EqualityFunc(const TrackEntry *ent1, const TrackEntry *ent2);
static void SortList();
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
static int gsNbLaps;
static BOOL gsAllowWeapons = FALSE;
static HWND trackSelDlg;
static WNDPROC oldListProc;

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
		//MessageBox(pWindow, _("Track not found"), PACKAGE_NAME, MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		lReturnValue = new MR_RecordFile;
		if(!lReturnValue->OpenForRead(filename.c_str(), TRUE)) {
			delete lReturnValue;
			lReturnValue = NULL;
			MessageBox(pWindow, _("Bad track file format"), PACKAGE_NAME, MB_ICONERROR | MB_OK | MB_APPLMODAL);
			ASSERT(FALSE);
		}
		else {
			TrackEntry lCurrentEntry;

			if(!ReadTrackEntry(lReturnValue, &lCurrentEntry, pFileName)) {
				delete lReturnValue;
				lReturnValue = NULL;

				MessageBox(pWindow, _("Bad track file format"), PACKAGE_NAME, MB_ICONERROR | MB_OK | MB_APPLMODAL);
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

	// Load the entry list
	MR_WAIT_CURSOR ReadTrackList();
	SortList();

	gsNbLaps = 5;								  // Default value
	gsAllowWeapons = false;

	if(DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TRACK_SELECT), pParentWindow, TrackSelectCallBack) == IDOK) {
		pTrackFile = gsSortedTrackList[gsSelectedEntry]->mFileName;
		pNbLap = gsNbLaps;
		pAllowWeapons = (gsAllowWeapons != FALSE);
		lReturnValue = true;
	} else
	lReturnValue = false;
	CleanList();

	return lReturnValue;
}

static BOOL CALLBACK TrackSelectCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;
	//int lCounter;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			trackSelDlg = pWindow;

			// i18n for labels, etc.
			SetWindowText(pWindow, _("Track selection"));
			SetDlgItemText(pWindow, IDC_TRACKS_LBL, _("Tracks"));
			SetDlgItemText(pWindow, IDC_DESC_LBL, _("Description"));
			SetDlgItemText(pWindow, IDC_LAPS_LBL, _("Laps"));
			SetDlgItemText(pWindow, IDC_WEAPONS_CHK, _("Weapons"));
			SetDlgItemText(pWindow, IDOK, _("OK"));
			SetDlgItemText(pWindow, IDCANCEL, _("Cancel"));

			// Init track file list
			for (sorted_t::iterator iter = gsSortedTrackList.begin();
				iter != gsSortedTrackList.end(); ++iter)
			{
				SendDlgItemMessage(pWindow, IDC_LIST, LB_ADDSTRING, 0,
					(LPARAM) ((*iter)->mFileName.c_str()));
			}

			SetDlgItemInt(pWindow, IDC_NB_LAP, gsNbLaps, FALSE);
			SendDlgItemMessage(pWindow, IDC_WEAPONS, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(pWindow, IDC_NB_LAP_SPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));

			if (!gsSortedTrackList.empty()) {
				gsSelectedEntry = 0;
				SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
				SetDlgItemText(pWindow, IDC_DESCRIPTION, gsSortedTrackList[gsSelectedEntry]->mDescription.c_str());
				SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, 0, 0);
			}
			else {
				gsSelectedEntry = -1;
				SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
				SetDlgItemText(pWindow, IDC_DESCRIPTION, _(" no selection"));
				SendDlgItemMessage(pWindow, IDC_LIST, LB_SETCURSEL, -1, 0);
			}

			oldListProc = (WNDPROC) SetWindowLong(
				GetDlgItem(pWindow, IDC_LIST),
				GWL_WNDPROC,
				(LONG)ListCallBack);

			lReturnValue = TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDC_LIST:
					switch (HIWORD(pWParam)) {
						case LBN_SELCHANGE:
							gsSelectedEntry = SendDlgItemMessage(pWindow, IDC_LIST, LB_GETCURSEL, 0, 0);
							if (gsSortedTrackList.empty() || (gsSelectedEntry == -1)) {
								SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, FALSE, 0);
								SetDlgItemText(pWindow, IDC_DESCRIPTION, _(" no selection"));
							}
							else {
								SendDlgItemMessage(pWindow, IDOK, WM_ENABLE, TRUE, 0);
								SetDlgItemText(pWindow, IDC_DESCRIPTION, gsSortedTrackList[gsSelectedEntry]->mDescription.c_str());
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
							MessageBox(pWindow,
								_("Number of laps should be between 1 and 99"),
								PACKAGE_NAME, MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						else
							EndDialog(pWindow, IDOK);
					}
					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

BOOL CALLBACK ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	switch (pMsgId) {
		case WM_LBUTTONDBLCLK:
			SendMessage(trackSelDlg, WM_COMMAND, IDOK, 0);
			return TRUE;

		default:
			return CallWindowProc(oldListProc, pWindow, pMsgId, pWParam, pLParam);
	}
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

// Equality function for SortList().
bool EqualityFunc(const TrackEntry *ent1, const TrackEntry *ent2)
{
	return (*ent1) == (*ent2);
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

	// prune duplicates (but do not delete them)
	std::unique(gsSortedTrackList.begin(), gsSortedTrackList.end(), EqualityFunc);
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

			// Open the file and read additional info
			MR_RecordFile lRecordFile;

			if(!lRecordFile.OpenForRead((dir + lFileInfo.name).c_str()))
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
	gsTrackList.clear();
	gsSortedTrackList.clear();
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
