
// PrefsDialog.cpp
// The "Preferences" dialog.
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
#include "GameApp.h"
#include "ControlPrefsPage.h"
#include "MiscPrefsPage.h"
#include "NetworkPrefsPage.h"
#include "VideoAudioPrefsPage.h"

#include "PrefsDialog.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

PrefsDialog::PrefsDialog(GameApp *app) :
	app(app)
{
	AddPage(videoAudioPage = new VideoAudioPrefsPage(app));
	AddPage(controlsPage = new ControlPrefsPage(app));
	AddPage(networkPage = new NetworkPrefsPage(app));
	AddPage(miscPage = new MiscPrefsPage(app));
}

PrefsDialog::~PrefsDialog()
{
	delete miscPage;
	delete networkPage;
	delete controlsPage;
	delete videoAudioPage;
}

/**
 * Add a prefs page (in addition to the standard pages).
 * @param page The page (may not be @c NULL).  It is up to the caller to free
 *             the pointer after the prefs dialog is no longer used.
 */
void PrefsDialog::AddPage(PrefsPage *page)
{
	pages.push_back(page);
}

/**
 * Display the modal dialog.
 * @param hinst The application instance handle.
 * @param parent The parent window.
 */
void PrefsDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	PROPSHEETHEADERW psh;
	PROPSHEETPAGEW *psp = new PROPSHEETPAGEW[pages.size()];

	int i = 0;
	for (pages_t::const_iterator iter = pages.begin();
		iter != pages.end(); ++iter)
	{
		(*iter)->InitPropPage(psp[i++], hinst);
	}

	Str::UW title(_("Preferences"));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = parent;
	psh.hInstance = hinst;
	psh.pszCaption = title;
	psh.nPages = pages.size();
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGEW)psp;
	psh.pfnCallback = NULL;

	PropertySheetW(&psh);

	delete[] psp;
}

}  // namespace Client
}  // namespace HoverRace
