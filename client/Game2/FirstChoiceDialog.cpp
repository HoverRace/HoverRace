
// FirstChoiceDialog.cpp
// The first choice (play online) dialog.
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
//

#include "StdAfx.h"

#include "../../engine/Util/Str.h"

#include "resource.h"

#include "FirstChoiceDialog.h"

using namespace HoverRace::Client;
using namespace HoverRace::Util;

/**
 * Display the modal dialog.
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 * @return @c true if the user chooses to play online, @c false otherwise.
 */
bool FirstChoiceDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	DWORD dlgRetv = DialogBoxParamW(hinst, MAKEINTRESOURCEW(IDD_FIRST_CHOICE),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));

	return (dlgRetv == IDOK);
}

BOOL CALLBACK FirstChoiceDialog::DlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	switch (pMsgId) {
		case WM_INITDIALOG:
			// i18n
			SetWindowTextW(pWindow, PACKAGE_NAME_L);
			SetDlgItemTextW(pWindow, IDC_CLICK_OK,
				Str::UW(_("Click OK to play on the Internet against other people")));
			SetDlgItemTextW(pWindow, IDOK, Str::UW(_("OK")));
			SetDlgItemTextW(pWindow, IDCANCEL, Str::UW(_("Cancel")));
			SetDlgItemTextW(pWindow, IDC_MAKE_SURE,
				Str::UW(_("Make sure that you are connected to the Internet before clicking on OK.  If you have any problems, visit our forums at http://www.hoverrace.com/bb for help.")));
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
				case IDOK:
					EndDialog(pWindow, LOWORD(pWParam));
					return TRUE;
			}
			break;
	}

	return FALSE;
}

