
// CheckRoomListDialog.h
// Header for the room list URL checker.
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

#include "RoomListDialog.h"

namespace HoverRace {
namespace Client {

/**
 * Dialog to test a roomlist URL.
 * @author Michael Imamura
 */
class CheckRoomListDialog : public RoomListDialog
{
	typedef RoomListDialog SUPER;
	private:
		CheckRoomListDialog() : SUPER("") { }
	public:
		CheckRoomListDialog(const std::string &url);
		virtual ~CheckRoomListDialog();

	public:
		void ShowModal(HINSTANCE hinst, HWND parent);

	protected:
		virtual void HandleLoadFinished(HWND hwnd, result_t result);

	protected:
		virtual BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

}  // namespace Client
}  // namespace HoverRace
