
// PrefsPage.cpp
// Base class for preferences pages.
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

#include "PrefsPage.h"

using namespace HoverRace::Client;
using namespace HoverRace::Util;

/**
 * Constructor.
 * @param title The localized dialog title (UTF-8).
 * @param dlgTmplId The dialog template ID.
 */
PrefsPage::PrefsPage(const std::string &title, DWORD dlgTmplId) :
	title(Str::UW(title.c_str())), dlgTmplId(dlgTmplId)
{
}

PrefsPage::~PrefsPage()
{
}

/**
 * Initialize a PROPSHEETPAGEW struct for this property page.
 * @param page [out] The struct to fill.
 * @param hinst The application instance handle.
 */
void PrefsPage::InitPropPage(PROPSHEETPAGEW &page, HINSTANCE hinst)
{
	memset(&page, 0, sizeof(PROPSHEETPAGEW));
	page.dwSize = sizeof(PROPSHEETPAGEW);
	page.dwFlags = PSP_USETITLE;
	page.hInstance = hinst;
	page.pszTemplate = MAKEINTRESOURCEW(dlgTmplId);
	page.pfnDlgProc = DlgFunc;
	page.pszTitle = title.c_str();
	page.lParam = reinterpret_cast<LPARAM>(this);
	page.pfnCallback = NULL;

	this->hinst = hinst;
}

BOOL CALLBACK PrefsPage::DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Determine which instance to route the message to.
	PrefsPage *dlg = NULL;
	if (message == WM_INITDIALOG) {
		PROPSHEETPAGEW *page = reinterpret_cast<PROPSHEETPAGEW*>(lparam);
		dlg = reinterpret_cast<PrefsPage*>(page->lParam);
		SetWindowLong(hwnd, GWL_USERDATA, page->lParam);
	}
	else {
		dlg = reinterpret_cast<PrefsPage*>(GetWindowLong(hwnd, GWL_USERDATA));
		if (message == WM_DESTROY) {
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}

	return (dlg == NULL) ? FALSE : dlg->DlgProc(hwnd, message, wparam, lparam);
}
