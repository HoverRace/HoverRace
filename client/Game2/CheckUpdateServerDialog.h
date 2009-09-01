// CheckUpdateServer.h
// Header for the update checker.
//
// Copyright (c) 2009 Ryan Curtin
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

#ifndef CHECK_UPDATE_SERVER_H
#define CHECK_UPDATE_SERVER_H

#include <string.h>

#include "../../engine/Net/CancelFlag.h"
#include "UpdateDownloader.h"

using namespace std;

namespace HoverRace {
namespace Client {

/***
 * Dialog to check for HoverRace updates.
 * Much of the code here is based on RoomList, RoomListDialog, and CheckRoomListDialog.
 *
 * @author Ryan Curtin
 */
class CheckUpdateServerDialog {
	public:
		CheckUpdateServerDialog(string url);
		~CheckUpdateServerDialog();

		void CheckUpdatesFromUrl(const string &url,
			Net::CancelFlagPtr cancelFlag = Net::CancelFlagPtr());

		void ShowModal(HINSTANCE hinst, HWND parent);

	protected:
		enum result_t {
			RESULT_UPTODATE,
			RESULT_UPDATE,
			RESULT_FAILED,
			RESULT_CANCELED
		};

		static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		static const WM_APP_UPDATE_CHECK_DONE = WM_APP + 20;

	private:
		void ThreadProc(HWND hwnd);

		string url;
		string errMsg;
		boost::thread loadThread;
		UpdateDownloaderPtr dlPtr;

}; // class CheckUpdateServerDialog

} // namespace Client
} // namespace HoverRace

#endif