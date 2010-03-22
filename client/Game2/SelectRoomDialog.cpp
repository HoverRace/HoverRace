
// SelectRoomDialog.h
// The room list selector.
//
// Copyright (c) 2009, 2010 Michael Imamura.
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

#include "StdAfx.h"

#include "../../engine/Util/Config.h"
#include "../../engine/Util/Str.h"

#include "resource.h"

#include "SelectRoomDialog.h"

using namespace HoverRace::Client;
using namespace HoverRace::Util;

SelectRoomDialog::SelectRoomDialog(const std::string &playerName) :
	SUPER(Config::GetInstance()->net.mainServer),
	playerName(playerName), finished(false)
{
}

SelectRoomDialog::~SelectRoomDialog()
{
}

const std::string &SelectRoomDialog::GetPlayerName() const
{
	return playerName;
}

/**
 * Display the modal dialog.
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 */
RoomListPtr SelectRoomDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	int result = DialogBoxParamW(hinst,
		MAKEINTRESOURCEW(IDD_INTERNET_PARAMS),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));

	return (result == IDOK) ? GetRoomList() : RoomListPtr();
}

void SelectRoomDialog::HandleLoadFinished(HWND hwnd, result_t result)
{
	switch (result) {
		case RESULT_SUCCESS:
			EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
			ShowWindow(GetDlgItem(hwnd, IDC_MSG_LBL), SW_HIDE);
			PopulateList(GetDlgItem(hwnd, IDC_ROOMLIST));
			break;

		case RESULT_CANCELED:
			EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
			EndDialog(hwnd, IDCANCEL);
			break;

		case RESULT_FAILED:
			DisplayError(hwnd);
			EndDialog(hwnd, IDCANCEL);
			break;
	}

	finished = true;
}

/**
 * Populate and show the room list widget.
 * @param hwnd The window handle of the room list widget.
 */
void SelectRoomDialog::PopulateList(HWND hwnd)
{
	RoomListPtr roomList = GetRoomList();
	const RoomList::rooms_t rooms = roomList->GetRooms();
	for (RoomList::rooms_t::const_iterator iter = rooms.begin();
		iter != rooms.end(); ++iter)
	{
		SendMessageW(hwnd, LB_ADDSTRING, 0,
			(LPARAM)(const wchar_t*)Str::UW((*iter)->name.c_str()));
	}
	SendMessageW(hwnd, LB_SETCURSEL, 0, 0);

	ShowWindow(hwnd, SW_SHOW);
}

// Dialog callback.
BOOL SelectRoomDialog::DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	BOOL retv = FALSE;

	switch (message) {

		case WM_INITDIALOG:
			SetWindowTextW(hwnd, Str::UW(_("Internet Meeting Room")));
			SetDlgItemTextW(hwnd, IDC_MSG_LBL, Str::UW(_("Connecting...")));
			SetDlgItemTextW(hwnd, IDC_YOUR_ALIAS, Str::UW(_("Your alias:")));
			SetDlgItemTextW(hwnd, IDC_ROOM_C, Str::UW(_("Room:")));
			SetDlgItemTextW(hwnd, IDC_ALIAS, Str::UW(playerName.c_str()));
			SetDlgItemTextW(hwnd, IDOK, Str::UW(_("OK")));
			SetDlgItemTextW(hwnd, IDCANCEL, Str::UW(_("Cancel")));
			retv = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDCANCEL:
					if (finished) {
						EndDialog(hwnd, IDCANCEL);
					}
					else {
						CancelLoad();
						SetDlgItemTextW(hwnd, IDC_MSG_LBL, Str::UW(_("Canceling...")));
						EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
						retv = TRUE;
					}
					break;

				case IDOK:
					GetRoomList()->SetSelectedRoom(
						SendDlgItemMessage(hwnd, IDC_ROOMLIST, LB_GETCURSEL, 0, 0));

					// Update the player name.
					wchar_t alias[64];
					GetDlgItemTextW(hwnd, IDC_ALIAS, alias, 64);
					alias[63] = 0;
					playerName = (const char*)Str::WU(alias);

					EndDialog(hwnd, IDOK);
					break;
			}
			break;
	}

	return retv;
}

