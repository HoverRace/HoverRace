
// SelectRoomDialog.h
// Header for the room list selector.
//
// Copyright (c) 2009, 2010 Michael Imamura.
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
#include "../../include/LocalPlayer.h"

namespace HoverRace {
namespace Client {

/**
 * Dialog to set username and select a room.
 * @author Michael Imamura
 */
class SelectRoomDialog : public RoomListDialog
{
	typedef RoomListDialog SUPER;
	public:
		SelectRoomDialog(const std::string &playerName, int onlinePartySize,
			const std::string *onlinePartyNames, int minPartySize = 1,
			BOOL localOnly = FALSE);
		virtual ~SelectRoomDialog();

	public:
		const std::string &GetPlayerName() const;
		int GetOnlinePartySize() const;
		const std::string &GetOnlinePartyName(int idx) const;
		BOOL WasAccepted() const;

	public:
		RoomListPtr ShowModal(HINSTANCE hinst, HWND parent);

	protected:
		virtual void HandleLoadFinished(HWND hwnd, result_t result);
		virtual BOOL ShouldLoadRooms() const;
	private:
		void PopulateList(HWND hwnd);
		void CreateDynamicControls(HWND hwnd);
		void SyncPlayerNamesFromControls(HWND hwnd);
		void UpdatePlayerRows(HWND hwnd);
		void ApplyDialogLayout(HWND hwnd);
		BOOL ConfigurePlayerControls(HWND hwnd, int playerIdx);

	protected:
		virtual BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		std::string playerName;
		int minPartySize;
		BOOL localOnly;
		int onlinePartySize;
		std::string onlinePartyNames[MR_MAX_LOCAL_PLAYER];
		MR_Config::cfg_controls_t tempControls[MR_MAX_LOCAL_PLAYER];
		HWND addPlayerButton;
		HWND removePlayerButton;
		HWND playerLabels[MR_MAX_LOCAL_PLAYER];
		HWND nameEdits[MR_MAX_LOCAL_PLAYER];
		HWND controlButtons[MR_MAX_LOCAL_PLAYER];
		BOOL accepted;
		volatile bool finished;
};

}  // namespace Client
}  // namespace HoverRace
