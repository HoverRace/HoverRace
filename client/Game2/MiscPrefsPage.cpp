
// MiscPrefsPage.cpp
// The "Miscellaneous" preferences page.
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

#include "../../engine/Util/Config.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"

#include "GameDirector.h"

#include "resource.h"

#include "MiscPrefsPage.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

MiscPrefsPage::MiscPrefsPage(GameDirector *app) :
	SUPER(_("Miscellaneous"), IDD_MISC),
	app(app)
{
}

MiscPrefsPage::~MiscPrefsPage()
{
}

BOOL MiscPrefsPage::DlgProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	Config *cfg = Config::GetInstance();

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		case WM_INITDIALOG:
			// i18nize text fields
			SetDlgItemTextW(pWindow, IDC_INTRO_MOVIE, Str::UW(_("Disable Intro Movie")));
			SetDlgItemTextW(pWindow, IDC_SHOW_INTERNET,
				Str::UW(_("Disable the prompt to connect to the Internet each time HoverRace starts")));
			SetDlgItemTextW(pWindow, IDC_NATIVE_BPP_FULLSCREEN,
				Str::UW(_("Use desktop &color depth when switching to fullscreen")));

			// Set default values correctly
			SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_SETCHECK, !cfg->misc.introMovie, 0);
			SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_SETCHECK, !cfg->misc.displayFirstScreen, 0);
			SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_SETCHECK, cfg->video.nativeBppFullscreen, 0);

			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					cfg->misc.introMovie = !SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_GETCHECK, 0, 0);
					cfg->misc.displayFirstScreen = !SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_GETCHECK, 0, 0);
					cfg->video.nativeBppFullscreen = (SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_GETCHECK, 0, 0) != FALSE);

					cfg->Save();
					break;

			}
			break;

	}

	return lReturnValue;
}

}  // namespace Client
}  // namespace HoverRace
