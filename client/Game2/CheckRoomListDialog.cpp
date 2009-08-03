
// CheckRoomListDialog.cpp
// The room list URL checker.
//
// Copyright (c) 2009 Michael Imamura.
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

#include "../../engine/Util/Str.h"

#include "RoomList.h"
#include "resource.h"

#include "CheckRoomListDialog.h"

using namespace HoverRace;
using namespace HoverRace::Client;
using namespace HoverRace::Util;

/**
 * Constructor.
 * @param url The URL to check.
 */
CheckRoomListDialog::CheckRoomListDialog(const std::string &url) :
	SUPER(url)
{
}

CheckRoomListDialog::~CheckRoomListDialog()
{
}

/**
 * Display the modal dialog.
 * If the user doesn't cancel the operation, will display the result
 * of the check in a message box.
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 */
void CheckRoomListDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	result_t result = (result_t)DialogBoxParamW(hinst,
		MAKEINTRESOURCEW(IDD_CHECK_ROOMLIST),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));
	switch (result) {
		case RESULT_SUCCESS:
			MessageBoxW(parent, Str::UW(_("Roomlist URL is valid!")), PACKAGE_NAME_L,
				MB_OK);
			break;
		case RESULT_FAILED:
			DisplayError(parent);
			break;
	}
}

void CheckRoomListDialog::HandleLoadFinished(HWND hwnd, result_t result)
{
	EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
	EndDialog(hwnd, result);
}

// Dialog callback.
BOOL CheckRoomListDialog::DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	BOOL retv = FALSE;

	switch (message) {

		case WM_INITDIALOG:
			SetWindowTextW(hwnd, PACKAGE_NAME_L);
			SetDlgItemTextW(hwnd, IDC_MSG_LBL, Str::UW(_("Checking roomlist URL.  Please wait.")));
			SetDlgItemTextW(hwnd, IDCANCEL, Str::UW(_("Cancel")));
			retv = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDCANCEL:
					CancelLoad();
					SetDlgItemTextW(hwnd, IDC_MSG_LBL, Str::UW(_("Canceling...")));
					EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
					retv = TRUE;
					break;
			}
			break;
	}

	return retv;
}

