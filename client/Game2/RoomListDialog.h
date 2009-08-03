
// RoomListDialog.h
// Header for the base room list dialog.
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

#pragma once

#include "RoomList.h"

namespace HoverRace {
namespace Client {

/**
 * Base class for dialogs that load the room list in the background.
 *
 * When a subclass opens a modal dialog using the DlgFunc callback, the
 * roomlist loading thread will be started.  When the transfer has finished,
 * the HandLoadFinished member function is called with the result of the
 * transfer.  The subclass can cancel the transfer by calling CancelLoad,
 * but should still wait for HandleLoadFinished to be called.
 *
 * @author Michael Imamura
 */
class RoomListDialog {
	private:
		RoomListDialog() { }
	protected:
		RoomListDialog(const std::string &url);
		virtual ~RoomListDialog();

	protected:
		const std::string &GetUrl() const { return url; }
		const std::string &GetErrorMessage() const { return errMsg; }
		RoomListPtr GetRoomList() const { return roomList; }

	protected:
		enum result_t {
			RESULT_SUCCESS,
			RESULT_CANCELED,
			RESULT_FAILED,
		};
		void CancelLoad();
		void DisplayError(HWND parent) const;
		virtual void HandleLoadFinished(HWND hwnd, result_t result) = 0;

	private:
		void ThreadProc(HWND dlgHwnd);

	protected:
		virtual BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) = 0;
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		std::string url;
		boost::thread loadThread;
		std::string errMsg;
		RoomListPtr roomList;

	protected:
		static const WM_APP_LOAD_COMPLETE = WM_APP + 20;
};

}  // namespace Client
}  // namespace HoverRace
