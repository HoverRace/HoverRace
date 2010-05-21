
// RoomListDialog.cpp
// The base room list dialog.
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

#include "StdAfx.h"

#include "../../engine/Net/Agent.h"
#include "../../engine/Net/NetExn.h"
#include "../../engine/Util/Str.h"

#include "RoomList.h"

#include "RoomListDialog.h"

using namespace HoverRace;
using namespace HoverRace::Client;
using namespace HoverRace::Util;

namespace {
	// CancelFlag that checks if the thread has been interrupted.
	// Note: This cannot be moved to the engine since it must be in the same
	//       module as where the thread was created, due to limitations of
	//       boost::thread.
	class ThreadInterruptedCancelFlag : public Net::CancelFlag
	{
		virtual bool IsCanceled()
		{
			try {
				boost::this_thread::interruption_point();
			}
			catch (boost::thread_interrupted&) {
				return true;
			}
			return false;
		}
	};
}

/**
 * Constructor.
 * @param url The roomlist URL to use.
 */
RoomListDialog::RoomListDialog(const std::string &url) :
	url(url)
{
}

RoomListDialog::~RoomListDialog()
{
}

/**
 * Signal to the loading thread to cancel the transfer.
 */
void RoomListDialog::CancelLoad()
{
	loadThread.interrupt();
}

/**
 * Display an error message in a message box.
 * @param parent The parent window.
 */
void RoomListDialog::DisplayError(HWND parent) const
{
	std::string msg = _("Error while retrieving roomlist:");
	msg += '\n';
	msg += url;
	msg += "\n\n";
	msg += errMsg;

	MessageBoxW(parent, Str::UW(msg.c_str()), PACKAGE_NAME_L,
		MB_ICONWARNING | MB_OK);
}

// Roomlist load thread.
void RoomListDialog::ThreadProc(HWND hwnd)
{
	result_t result;
	Net::CancelFlagPtr cancelFlag((Net::CancelFlag*)new ThreadInterruptedCancelFlag());
	try {
		roomList = RoomListPtr(new RoomList());
		roomList->LoadFromUrl(url, cancelFlag);
		result = RESULT_SUCCESS;
	}
	catch (Net::CanceledExn&) {
		result = RESULT_CANCELED;
	}
	catch (Net::NetExn &ex) {
		errMsg = ex.what();
		result = RESULT_FAILED;
	}
	PostMessageW(hwnd, WM_APP_LOAD_COMPLETE, result, 0);
}

// Dialog callback -- will call DlgProc().
BOOL CALLBACK RoomListDialog::DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Determine which instance to route the message to.
	RoomListDialog *dlg;
	if (message == WM_INITDIALOG) {
		dlg = reinterpret_cast<RoomListDialog*>(lparam);
		SetWindowLong(hwnd, GWL_USERDATA, lparam);
	} else {
		dlg = reinterpret_cast<RoomListDialog*>(GetWindowLong(hwnd, GWL_USERDATA));
		if (message == WM_DESTROY) {
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}

	if (dlg == NULL) return FALSE;

	BOOL retv = FALSE;
	if (message == WM_APP_LOAD_COMPLETE) {
		dlg->HandleLoadFinished(hwnd, (result_t)wparam);
		dlg->loadThread.join();
	}
	else {
		BOOL retv = dlg->DlgProc(hwnd, message, wparam, lparam);
		if (message == WM_INITDIALOG) {
			// Kick off the load thread.
			dlg->loadThread = boost::thread(boost::bind(&RoomListDialog::ThreadProc, dlg, hwnd));
		}
	}
	return retv;
}
