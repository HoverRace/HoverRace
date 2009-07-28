
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
#include "GameApp.h"
#include "PathSelector.h"

#include "resource.h"

#include "NetworkPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Client;
using namespace HoverRace::Util;

NetworkPrefsPage::NetworkPrefsPage(MR_GameApp *app) :
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
			SetDlgItemTextW(pWindow, IDC_IMR_GROUP, Str::UW(_("Internet Meeting Room")));

			SetDlgItemTextW(pWindow, IDC_SERVER_ADDR,
				Str::UW(_("Address of Server Roomlist:")));
			SetDlgItemText(pWindow, IDC_MAINSERVER, cfg->net.mainServer.c_str());

			SetDlgItemTextW(pWindow, IDC_LOG_CHATS, Str::UW(_("&Log all chat sessions to:")));
			SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_SETCHECK, cfg->net.logChats, 0);
			SetDlgItemText(pWindow, IDC_LOG_CHATS_TXT, cfg->net.logChatsPath.c_str());
			SetDlgItemTextW(pWindow, IDC_OPEN_FOLDER, Str::UW(_("Open Folder")));

			SetDlgItemTextW(pWindow, IDC_CONNECTION_GROUP, Str::UW(_("Connection")));

			SetDlgItemTextW(pWindow, IDC_TCP_SERV_PORT_C, Str::UW(_("TCP Server Port:")));
			SetDlgItemTextW(pWindow, IDC_TCP_RECV_PORT_C, Str::UW(_("TCP Receive Port:")));
			SetDlgItemTextW(pWindow, IDC_UDP_RECV_PORT_C, Str::UW(_("UDP Receive Port:")));
			SetPortFields(pWindow, cfg->net.tcpServPort, cfg->net.tcpRecvPort, cfg->net.udpRecvPort);

			SetDlgItemTextW(pWindow, IDC_SERVER_RESET_DEFAULT, Str::UW("Reset to Default"));
			SetDlgItemTextW(pWindow, IDC_CONNECTION_RESET_DEFAULT, Str::UW("Reset to Defaults"));
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDC_LOG_CHATS_BROWSE:
					{
						char buf[MAX_PATH];
						GetDlgItemText(pWindow, IDC_LOG_CHATS_TXT, buf, sizeof(buf));
						std::string curPath = buf;
						if (PathSelector(_("Select a destination folder for saved chat sessions.")).
							ShowModal(pWindow, curPath))
						{
							SetDlgItemText(pWindow, IDC_LOG_CHATS_TXT, curPath.c_str());
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

				case IDC_SERVER_RESET_DEFAULT:
					SetDlgItemText(pWindow, IDC_MAINSERVER, Config::GetDefaultRoomListUrl().c_str());
					break;

				case IDC_OPEN_FOLDER:
					{
						char buf[MAX_PATH];
						GetDlgItemText(pWindow, IDC_LOG_CHATS_TXT, buf, sizeof(buf));
						OS::OpenLink(buf);
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
						if(GetDlgItemText(pWindow, IDC_MAINSERVER, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.mainServer = lBuffer;
						app->SignalServerHasChanged();

						cfg->net.logChats = (SendDlgItemMessage(pWindow, IDC_LOG_CHATS, BM_GETCHECK, 0, 0) != FALSE);
						if(GetDlgItemText(pWindow, IDC_LOG_CHATS_TXT, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);
						cfg->net.logChatsPath = lBuffer;

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
