
// MultiplayerPrefsPage.cpp
// The "Multiplayer" preferences page.
//
// Copyright (c) 2012 Michael Imamura.
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

#include "CheckRoomListDialog.h"
#include "CheckUpdateServerDialog.h"
#include "FolderViewer.h"
#include "GameDirector.h"
#include "PathSelector.h"

#include "resource.h"

#include "MultiplayerPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

MultiplayerPrefsPage::MultiplayerPrefsPage(GameDirector *app) :
	SUPER(_("Multiplayer"), IDD_MULTIPLAYER_PREFS),
	app(app)
{
}

MultiplayerPrefsPage::~MultiplayerPrefsPage()
{
}

BOOL MultiplayerPrefsPage::DlgProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	Config *cfg = Config::GetInstance();

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		case WM_INITDIALOG:
			SetDlgItemTextW(pWindow, IDC_IMR_CLIENT_GROUP, Str::UW(_("Internet Meeting Room")));

			SetDlgItemTextW(pWindow, IDC_MESSAGE_RECEIVED_SOUND, Str::UW(_("Play a sound when a &message is received")));
			SendDlgItemMessage(pWindow, IDC_MESSAGE_RECEIVED_SOUND, BM_SETCHECK, cfg->net.messageReceivedSound, 0);
			SetDlgItemTextW(pWindow, IDC_MESSAGE_RECEIVED_SOUND_ONLY_BG, Str::UW(_("Only play sound when not &foreground window")));
			SendDlgItemMessage(pWindow, IDC_MESSAGE_RECEIVED_SOUND_ONLY_BG, BM_SETCHECK, cfg->net.messageReceivedSoundOnlyBg, 0);

			SetDlgItemTextW(pWindow, IDC_LOG_CHATS, Str::UW(_("&Log all chat sessions to:")));
			SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_SETCHECK, cfg->net.logChats, 0);
			SetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, Str::PW(cfg->net.logChatsPath));
			SetDlgItemTextW(pWindow, IDC_OPEN_FOLDER, Str::UW(_("Open Folder")));
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDC_LOG_CHATS_BROWSE:
					{
						wchar_t buf[MAX_PATH];
						GetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, buf, sizeof(buf) / sizeof(wchar_t));
						OS::path_t curPath(buf);
						if (PathSelector(_("Select a destination folder for saved chat sessions.")).
							ShowModal(pWindow, curPath))
						{
							SetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, Str::PW(curPath));
						}
					}
					break;

				case IDC_OPEN_FOLDER:
					{
						wchar_t buf[MAX_PATH];
						GetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, buf, sizeof(buf) / sizeof(wchar_t));
						FolderViewer(buf).Show(pWindow);
					}
					break;
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					{
						wchar_t wbuf[MAX_PATH];

						cfg->net.messageReceivedSound = (SendDlgItemMessage(pWindow, IDC_MESSAGE_RECEIVED_SOUND, BM_GETCHECK, 0, 0) != FALSE);
						cfg->net.messageReceivedSoundOnlyBg = (SendDlgItemMessage(pWindow, IDC_MESSAGE_RECEIVED_SOUND_ONLY_BG, BM_GETCHECK, 0, 0) != FALSE);

						cfg->net.logChats = (SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_GETCHECK, 0, 0) != FALSE);
						if(GetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, wbuf, sizeof(wbuf) / sizeof(wchar_t)) == 0)
							ASSERT(FALSE);
						cfg->net.logChatsPath = wbuf;
					}

					cfg->Save();
					break;

			}
			break;

	}

	return lReturnValue;
}

}  // namespace Client
}  // namespace HoverRace
