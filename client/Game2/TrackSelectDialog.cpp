
// TrackSelectDialog.cpp
// Track selector.
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2010 Michael Imamura.
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

#include "StdAfx.h"

#include <algorithm>

#include "../../engine/Model/TrackEntry.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"

#include "resource.h"
#include "Rulebook.h"

#include "TrackSelectDialog.h"

using HoverRace::Model::TrackEntry;
using namespace HoverRace::Parcel;
using namespace HoverRace::Util;

namespace {
	HWND trackSelDlg;
	WNDPROC oldListProc;
}

namespace HoverRace {
namespace Client {

TrackSelectDialog::TrackSelectDialog() :
	laps(5), opts(0), idx(-1)
{
}

TrackSelectDialog::~TrackSelectDialog()
{
	Clean();
}

void TrackSelectDialog::Clean()
{
}

/**
 * Display the modal dialog.
 * @param hinst The instance handle.
 * @param parent The parent window handle.
 * @return The selected rulebook or @c NULL if the user canceled.
 */
RulebookPtr TrackSelectDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	trackList.Reload(Config::GetInstance()->GetTrackBundle());

	if (DialogBoxParamW(hinst, MAKEINTRESOURCEW(IDD_TRACK_SELECT), parent,
		DlgFunc, reinterpret_cast<LPARAM>(this)) == IDOK)
	{
		return boost::make_shared<Rulebook>(trackName, laps, opts);
	}
	else {
		return RulebookPtr();
	}
}

BOOL TrackSelectDialog::DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	BOOL retv = FALSE;

	switch (message) {

		case WM_INITDIALOG:
			trackSelDlg = hwnd;

			// i18n for labels, etc.
			SetWindowTextW(hwnd, Str::UW(_("Track selection")));
			SetDlgItemTextW(hwnd, IDC_TRACKS_LBL, Str::UW(_("Tracks")));
			SetDlgItemTextW(hwnd, IDC_DESC_LBL, Str::UW(_("Description")));
			SetDlgItemTextW(hwnd, IDC_LAPS_LBL, Str::UW(_("Laps")));
			SetDlgItemTextW(hwnd, IDC_WEAPONS_CHK, Str::UW(_("Weapons")));
			SetDlgItemTextW(hwnd, IDOK, Str::UW(_("OK")));
			SetDlgItemTextW(hwnd, IDCANCEL, Str::UW(_("Cancel")));
			SetDlgItemTextW(hwnd, IDC_OPEN_FOLDER, Str::UW(_("Open Download Folder")));
			SetDlgItemTextW(hwnd, IDC_CRAFTS_BOX, Str::UW(_("Crafts")));
			SetDlgItemTextW(hwnd, IDC_OPTIONS_BOX, Str::UW(_("Options")));
			// this may require re-thinking if more crafts are to be added
			SetDlgItemTextW(hwnd, IDC_BASIC_CHK, Str::UW(_("Basic")));
			SetDlgItemTextW(hwnd, IDC_BI_CHK, Str::UW(_("Bi-Turbo")));
			SetDlgItemTextW(hwnd, IDC_CX_CHK, Str::UW(_("Low CX")));
			SetDlgItemTextW(hwnd, IDC_EON_CHK, Str::UW(_("Eon")));
			SetDlgItemTextW(hwnd, IDC_MINES_CHK, Str::UW(_("Mines")));
			SetDlgItemTextW(hwnd, IDC_CANS_CHK, Str::UW(_("Boost Cans")));

			// Init track file list
			BOOST_FOREACH(TrackEntry *ent, trackList) {
				SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0,
					(LPARAM)((const wchar_t*)Str::UW(ent->name.c_str())));
			}

			SetDlgItemInt(hwnd, IDC_NB_LAP, laps, FALSE);

			// Options: Allowed items.
			SendDlgItemMessage(hwnd, IDC_WEAPONS_CHK, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_MINES_CHK, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CANS_CHK, BM_SETCHECK, BST_CHECKED, 0);

			// Options: Allowed crafts.
			SendDlgItemMessage(hwnd, IDC_BASIC_CHK, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_BI_CHK, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_CX_CHK, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(hwnd, IDC_EON_CHK, BM_SETCHECK, BST_CHECKED, 0);

			SendDlgItemMessage(hwnd, IDC_NB_LAP_SPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));

			if (!trackList.IsEmpty()) {
				idx = 0;
				SendDlgItemMessage(hwnd, IDOK, WM_ENABLE, TRUE, 0);
				SetDlgItemTextW(hwnd, IDC_DESCRIPTION, Str::UW(trackList[idx]->description.c_str()));
				SendDlgItemMessage(hwnd, IDC_LIST, LB_SETCURSEL, 0, 0);
			}
			else {
				idx = -1;
				SendDlgItemMessage(hwnd, IDOK, WM_ENABLE, FALSE, 0);
				SetDlgItemTextW(hwnd, IDC_DESCRIPTION, Str::UW(_("no selection")));
				SendDlgItemMessage(hwnd, IDC_LIST, LB_SETCURSEL, -1, 0);
			}

			oldListProc = (WNDPROC) SetWindowLong(
				GetDlgItem(hwnd, IDC_LIST),
				GWL_WNDPROC,
				(LONG)ListCallBack);

			retv = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDC_LIST:
					switch (HIWORD(wparam)) {
						case LBN_SELCHANGE:
							idx = SendDlgItemMessage(hwnd, IDC_LIST, LB_GETCURSEL, 0, 0);
							if (trackList.IsEmpty() || idx == -1) {
								SendDlgItemMessage(hwnd, IDOK, WM_ENABLE, FALSE, 0);
								SetDlgItemTextW(hwnd, IDC_DESCRIPTION, Str::UW(_("no selection")));
							}
							else {
								SendDlgItemMessage(hwnd, IDOK, WM_ENABLE, TRUE, 0);
								SetDlgItemTextW(hwnd, IDC_DESCRIPTION, Str::UW(trackList[idx]->description.c_str()));
							}
							break;
					}
					break;

				case IDC_OPEN_FOLDER:
					OS::OpenPath(Config::GetInstance()->GetUserTrackPath());
					EndDialog(hwnd, IDCANCEL);
					break;

				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					retv = TRUE;
					break;

				case IDOK:
					if (idx != -1) {
						laps = GetDlgItemInt(hwnd, IDC_NB_LAP, NULL, FALSE);

						opts = 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_WEAPONS_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_WEAPONS : 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_MINES_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_MINES : 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_CANS_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_CANS : 0;

						opts |= (SendDlgItemMessage(hwnd, IDC_BASIC_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_BASIC : 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_BI_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_BI : 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_CX_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_CX : 0;
						opts |= (SendDlgItemMessage(hwnd, IDC_EON_CHK, BM_GETCHECK, 0, 0) == BST_CHECKED) ? OPT_ALLOW_EON : 0;

						trackName = trackList[idx]->name;

						if (laps < 1) {
							MessageBoxW(hwnd, Str::UW(_("Number of laps should be greater than 1")),
								PACKAGE_NAME_L, MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						}
						else if ((opts & 0x0F) == 0) {
							MessageBoxW(hwnd, Str::UW(_("At least one craft must be selected")),
								PACKAGE_NAME_L, MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						}
						else {
							EndDialog(hwnd, IDOK);
						}
					}
					retv = TRUE;
					break;
			}
	}

	return retv;
}

/// Global dialog callback dispatcher.
BOOL CALLBACK TrackSelectDialog::DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Determine which instance to route the message to.
	TrackSelectDialog *dlg;
	if (message == WM_INITDIALOG) {
		dlg = reinterpret_cast<TrackSelectDialog*>(lparam);
		SetWindowLong(hwnd, GWL_USERDATA, lparam);
	} else {
		dlg = reinterpret_cast<TrackSelectDialog*>(GetWindowLong(hwnd, GWL_USERDATA));
		if (message == WM_DESTROY) {
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}

	return (dlg == NULL) ? FALSE : dlg->DlgProc(hwnd, message, wparam, lparam);
}

/// Message handler for subclassed list widget.
BOOL CALLBACK TrackSelectDialog::ListCallBack(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_LBUTTONDBLCLK:
			SendMessage(trackSelDlg, WM_COMMAND, IDOK, 0);
			return TRUE;

		default:
			return CallWindowProc(oldListProc, hwnd, message, wparam, lparam);
	}
}

}  // namespace Client
}  // namespace HoverRace
