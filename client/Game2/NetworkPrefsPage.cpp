
// NetworkPrefsPage.cpp
// The "Network" preferences page.
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

#include "CheckRoomListDialog.h"
#include "CheckUpdateServerDialog.h"
#include "GameDirector.h"
#include "PathSelector.h"

#include "resource.h"

#include "NetworkPrefsPage.h"

using boost::format;
using boost::str;
using boost::wformat;

using namespace HoverRace::Util;

namespace fs = boost::filesystem;

namespace HoverRace {
namespace Client {

NetworkPrefsPage::NetworkPrefsPage(GameDirector *app) :
	SUPER(_("Network"), IDD_NETWORK),
	app(app)
{
}

NetworkPrefsPage::~NetworkPrefsPage()
{
}

void NetworkPrefsPage::SetPortFields(HWND hwnd, int tcpServ, int tcpRecv, int udpRecv)
{
	static format portFmt("%d", OS::stdLocale);

	SetDlgItemText(hwnd, IDC_TCP_SERV_PORT, str(portFmt % tcpServ).c_str());
	SetDlgItemText(hwnd, IDC_TCP_RECV_PORT, str(portFmt % tcpRecv).c_str());
	SetDlgItemText(hwnd, IDC_UDP_RECV_PORT, str(portFmt % udpRecv).c_str());
}

BOOL NetworkPrefsPage::DlgProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	Config *cfg = Config::GetInstance();

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		case WM_INITDIALOG:
			SetDlgItemTextW(pWindow, IDC_IMR_GROUP, Str::UW(_("Server Options")));

			SetDlgItemTextW(pWindow, IDC_SERVER_ADDR,
				Str::UW(_("Address of Server Roomlist:")));
			SetDlgItemText(pWindow, IDC_MAINSERVER, cfg->net.mainServer.c_str());
			SetDlgItemTextW(pWindow, IDC_CHECK_URL, Str::UW(_("Check URL")));

			SetDlgItemTextW(pWindow, IDC_UPDATE_SERVER_ADDR,
				Str::UW(_("Address of Update Server List:")));
			SetDlgItemText(pWindow, IDC_UPDATESERVER, cfg->net.updateServer.c_str());
			SetDlgItemTextW(pWindow, IDC_CHECK_UPDATES, Str::UW(_("Check for updates")));
			SetDlgItemTextW(pWindow, IDC_AUTO_CHECK, Str::UW(_("Automatically check for HoverRace updates")));
			SendDlgItemMessage(pWindow, IDC_AUTO_CHECK, BM_SETCHECK, cfg->net.autoUpdates, 0);

			SetDlgItemTextW(pWindow, IDC_LOG_CHATS, Str::UW(_("&Log all chat sessions to:")));
			SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_SETCHECK, cfg->net.logChats, 0);
			SetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, cfg->net.logChatsPath.file_string().c_str());
			SetDlgItemTextW(pWindow, IDC_OPEN_FOLDER, Str::UW(_("Open Folder")));

			SetDlgItemTextW(pWindow, IDC_CONNECTION_GROUP, Str::UW(_("Connection")));

			SetDlgItemTextW(pWindow, IDC_TCP_SERV_PORT_C, Str::UW(_("TCP Server Port:")));
			SetDlgItemTextW(pWindow, IDC_TCP_RECV_PORT_C, Str::UW(_("TCP Receive Port:")));
			SetDlgItemTextW(pWindow, IDC_UDP_RECV_PORT_C, Str::UW(_("UDP Receive Port:")));
			SetPortFields(pWindow, cfg->net.tcpServPort, cfg->net.tcpRecvPort, cfg->net.udpRecvPort);

			SetDlgItemTextW(pWindow, IDC_SERVER_RESET_DEFAULT, Str::UW(_("Reset to Default")));
			SetDlgItemTextW(pWindow, IDC_UPDATE_SERVER_RESET_DEFAULT, Str::UW(_("Reset to Default")));
			SetDlgItemTextW(pWindow, IDC_CONNECTION_RESET_DEFAULT, Str::UW(_("Reset to Defaults")));
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
							SetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, curPath.file_string().c_str());
						}
					}
					break;

				case IDC_CHECK_URL:
					{
						char buf[MAX_PATH];
						GetDlgItemText(pWindow, IDC_MAINSERVER, buf, sizeof(buf));
						CheckRoomListDialog(buf).ShowModal(NULL, pWindow);
					}
					break;

				case IDC_CHECK_UPDATES:
					{
						char buf[MAX_PATH];
						GetDlgItemText(pWindow, IDC_UPDATESERVER, buf, sizeof(buf));
						CheckUpdateServerDialog(buf).ShowModal(NULL, pWindow);
					}
					break;

				case IDC_SERVER_RESET_DEFAULT:
					SetDlgItemText(pWindow, IDC_MAINSERVER, Config::GetDefaultRoomListUrl().c_str());
					break;

				case IDC_UPDATE_SERVER_RESET_DEFAULT:
					SetDlgItemText(pWindow, IDC_UPDATESERVER, Config::GetDefaultUpdateServerUrl().c_str());
					break;

				case IDC_OPEN_FOLDER:
					{
						wchar_t buf[MAX_PATH];
						GetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, buf, sizeof(buf) / sizeof(wchar_t));

						if (VerifyDirectory(pWindow, buf)) {
							OS::OpenPath(buf);
						}
					}
					break;

				case IDC_CONNECTION_RESET_DEFAULT:
					SetPortFields(pWindow,
						Config::cfg_net_t::DEFAULT_TCP_SERV_PORT,
						Config::cfg_net_t::DEFAULT_TCP_RECV_PORT,
						Config::cfg_net_t::DEFAULT_UDP_RECV_PORT
						);
					break;
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					{
						char lBuffer[MAX_PATH];
						wchar_t wbuf[MAX_PATH];
						if(GetDlgItemText(pWindow, IDC_MAINSERVER, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.mainServer = lBuffer;
						app->SignalServerHasChanged();

						if(GetDlgItemText(pWindow, IDC_UPDATESERVER, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);
						
						cfg->net.updateServer = lBuffer;
						bool newUpdateSetting = (SendDlgItemMessage(pWindow, IDC_AUTO_CHECK, BM_GETCHECK, 0, 0) != FALSE); 
						if(newUpdateSetting != cfg->net.autoUpdates) {
							cfg->net.autoUpdates = newUpdateSetting;
							app->ChangeAutoUpdates(newUpdateSetting);
						}

						cfg->net.logChats = (SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_GETCHECK, 0, 0) != FALSE);
						if(GetDlgItemTextW(pWindow, IDC_LOG_CHATS_TXT, wbuf, sizeof(wbuf) / sizeof(wchar_t)) == 0)
							ASSERT(FALSE);
						cfg->net.logChatsPath = wbuf;

						if(GetDlgItemText(pWindow, IDC_TCP_SERV_PORT, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.tcpServPort = atoi(lBuffer);

						if(GetDlgItemText(pWindow, IDC_TCP_RECV_PORT, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.tcpRecvPort = atoi(lBuffer);
						
						if(GetDlgItemText(pWindow, IDC_UDP_RECV_PORT, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.udpRecvPort = atoi(lBuffer);
					}

					cfg->Save();
					break;

			}
			break;

	}

	return lReturnValue;
}

/**
 * Verify the directory exists.
 * If the directory does not exist, the user will be prompted to create it.
 * @param wnd The parent window for dialogs.
 * @param path The path to check.
 * @return @c true if the directory exists, @c false otherwise.
 */
bool NetworkPrefsPage::VerifyDirectory(HWND wnd, const wchar_t *path) {
	if (fs::exists(path)) {
		if (fs::is_directory(path)) {
			return true;
		} else {
			MessageBoxW(wnd,
				str(wformat(L"%s\n\n%s") %
					(const wchar_t*)Str::UW(_("The path is not a directory.")) %
					path).c_str(),
				PACKAGE_NAME_L,
				MB_ICONWARNING);
			return false;
		}
	} else {
		int resp = MessageBoxW(wnd,
			str(wformat(L"%s\n\n%s\n\n%s") %
				(const wchar_t*)Str::UW(_("Directory does not exist.")) %
				path %
				(const wchar_t*)Str::UW(_("Would you like to create it?"))).c_str(),
			PACKAGE_NAME_L,
			MB_ICONQUESTION | MB_YESNO);

		if (resp == IDYES) {
			fs::create_directories(path);
			if (fs::exists(path) && fs::is_directory(path)) {
				return true;
			} else {
				MessageBoxW(wnd,
					Str::UW(_("Failed to create directory.")),
					PACKAGE_NAME_L, MB_ICONWARNING);
				return false;
			}
		}

		return false;
	}
}

}  // namespace Client
}  // namespace HoverRace
