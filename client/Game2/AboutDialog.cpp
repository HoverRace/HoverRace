
// AboutDialog.cpp
// The "About" dialog.
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

#include "resource.h"

#include "AboutDialog.h"

using boost::format;
using boost::str;

using namespace HoverRace;
using namespace HoverRace::Util;

/**
 * Display the modal dialog.
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 * @return @c true.
 */
bool AboutDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	DialogBoxParamW(hinst, MAKEINTRESOURCEW(IDD_ABOUT),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));

	return true;
}

BOOL CALLBACK AboutDialog::DlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {

		case WM_INITDIALOG:
			{
				SetWindowTextW(pWindow, Str::UW(_("About HoverRace")));

				// set version string
				std::string verStr(_("HoverRace version "));
				if(Config::GetInstance()->IsPrerelease())
					verStr += Config::GetInstance()->GetFullVersion();
				else
					verStr += Config::GetInstance()->GetVersion();
				SetDlgItemTextW(pWindow, IDC_VER_TXT, Str::UW(verStr.c_str()));

				CHARFORMAT fmt;
				fmt.cbSize = sizeof(CHARFORMAT);
				fmt.yHeight = 240; // 12pt (units are 1/20 pt)
				fmt.dwEffects = CFE_BOLD | CFE_UNDERLINE;
				fmt.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_SIZE;
				SendMessage(GetDlgItem(pWindow, IDC_VER_TXT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) (CHARFORMAT FAR *) &fmt);

				std::ostringstream oss;
				oss <<
					_("HoverRace is brought to you by:") << "\r\n"
					"\r\n"
					"Richard Langlois (" << _("original author") << ")\r\n"
					"  Grokksoft Inc.\r\n"
					"\r\n" <<
					_("with contributions from (in alphabetical order)") << ":\r\n"
					"\r\n"
					"Austin L. Brock\r\n"
					"Ryan Curtin\r\n"
					"Michael Imamura\r\n"
					"\r\n" <<
					_("and with the help of the many testers willing to put up with bugs and other strange happenings.") << "\r\n"
					"\r\n" <<
					_("visit us at") << " " << HR_WEBSITE << "\r\n"
					"\r\n"
					"HoverRace © Richard Langlois, Grokksoft Inc.\r\n"
					"\r\n" <<
					_("Thanks also to the following projects") << ":\r\n"
					"\r\n"
					"Boost C++ Libraries\r\n"
					"  http://www.boost.org/\r\n"
					"\r\n"
					"libcurl - Daniel Stenberg and contributors.\r\n"
					"  http://curl.haxx.se/\r\n"
					"\r\n"
					"LibYAML - Kirill Simonov and contributors.\r\n"
					"  http://pyyaml.org/wiki/LibYAML\r\n"
					"\r\n"
					"LiteUnzip - Jeff Glatt, based on work by Lucian Wischik, based on work by Jean-Loup Gailly and Mark Adler.\r\n"
					"\r\n"
					"Lua -- Copyright (c) 1994-2008 Lua.org, PUC-Rio.\r\n"
					"  http://www.lua.org/\r\n"
					"\r\n"
					"Luabind -- Copyright (c) 2003 Daniel Wallin and Arvid Norberg\r\n"
					"  http://www.rasterbar.com/products/luabind.html\r\n"
				;
				SetDlgItemTextW(pWindow, IDC_ABOUT_TXT, Str::UW(oss.str().c_str()));
				SetDlgItemTextW(pWindow, IDOK, Str::UW(_("Close")));

				lReturnValue = TRUE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					EndDialog(pWindow, IDOK);
					lReturnValue = TRUE;
					break;
			}
			break;

	}
	return lReturnValue;
}