
// ControlPrefsPage.h
// The "Controls" preferences page.
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

#include "Control/Controller.h"
#include "Control/UiHandler.h"
#include "GameDirector.h"

#include "resource.h"

#include "ControlPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Util;
using HoverRace::Client::Control::InputEventController;

#define MRM_CONTROL_TIMER   3
#define MRM_CONTROL_POLL    4

namespace HoverRace {
namespace Client {

ControlPrefsPage::ControlPrefsPage(GameDirector *app) :
	SUPER(_("Controls"), IDD_CONTROL),
	app(app)
{
}

ControlPrefsPage::~ControlPrefsPage()
{
}


void ControlPrefsPage::UpdateBindingLabels(HWND pWindow)
{
	Config *cfg = Config::GetInstance();
	InputEventController* const controller = app->GetController();

	char buffer[200];
	HWND combobox = GetDlgItem(pWindow, IDC_MAP_SELECT);
	SendMessage(combobox, WM_GETTEXT, 200, (LPARAM) &buffer);
	std::string mapname = buffer;

	// we need to populate the ListView with actions
	HWND hList = GetDlgItem(pWindow, IDC_CONTROL_BINDINGS);
	InputEventController::ActionMap map = controller->GetActionMap(mapname);

	// order bindings by defined ordering in ControlAction::listOrder
	std::map<int, int> orderMap;
	for(InputEventController::ActionMap::iterator it = map.begin(); it != map.end(); it++)
		orderMap[it->second->getListOrder()] = it->first;

	// Delete all items
	SendMessage(hList, LVM_DELETEALLITEMS, 0, 0);

	// ListView adds new items at the top... so start from the back
	for(std::map<int, int>::reverse_iterator it(orderMap.end()); 
			it != std::map<int, int>::reverse_iterator(orderMap.begin()); it++) {
		LVITEM item;
		memset(&item, 0, sizeof(item));
								
		std::string temp;
		item.mask = LVIF_TEXT;
		temp = map[it->second]->getName();
		item.pszText = (LPSTR) temp.c_str();
		int pos = SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM) &item);

		item.mask = LVIF_TEXT;
		item.iItem = pos;
		item.iSubItem = 1;
		temp = controller->HashToString(it->second).c_str();
		item.pszText = (LPSTR) temp.c_str();
		SendMessage(hList, LVM_SETITEM, pos, (LPARAM) &item);
	}
}


/***
 * Dialog callback for the controls property page.
 * We need to make a separate Controller object for this HWND.
 *
 * @param pWindow Handle of our window
 */
BOOL ControlPrefsPage::DlgProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	Config *cfg = Config::GetInstance();

	static Config::cfg_controls_t *oldcontrols = NULL;

	InputEventController* const controller = app->GetController();

	BOOL lReturnValue = FALSE;
	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			// add available maps to the drop-down box
			{
				std::vector<std::string> mapnames = controller->GetAvailableMaps();
				HWND hList = GetDlgItem(pWindow, IDC_MAP_SELECT);
				
				// add all the maps except console-keys
				for(int i = 0; i < mapnames.size(); i++) {
					if(mapnames.at(i) == "console-keys")
						continue; // don't add this one
					int item = SendMessage(hList, CB_ADDSTRING, 0, (LPARAM) mapnames.at(i).c_str());
				}

				// select first player map (or first map)
				std::string mapname = _("Player");
				mapname += " 1";
				int select = SendMessage(hList, CB_FINDSTRINGEXACT, -1, (LPARAM) mapname.c_str());
				SendMessage(hList, CB_SETCURSEL, (select == CB_ERR) ? 0 : select, 0); // set current item

				// now fake the message that we just changed the selection
				// (this is why there is no break at the end of this case)
				pWParam = IDC_MAP_SELECT;

				// initialize columns in ListView box
				LVCOLUMNW keycol;
				memset(&keycol, 0, sizeof(keycol));
				keycol.mask = LVCF_WIDTH | LVCF_TEXT;
				keycol.cx = 400; // 400px
				keycol.pszText = (LPWSTR) _("Action");

				LVCOLUMNW bindcol;
				memset(&bindcol, 0, sizeof(bindcol));
				bindcol.mask = LVCF_WIDTH | LVCF_TEXT;
				bindcol.cx = 135; // 135px
				bindcol.pszText = (LPWSTR) _("Binding");

				HWND listview = GetDlgItem(pWindow, IDC_CONTROL_BINDING);
				SendMessageW(listview, LVM_INSERTCOLUMN, 0, (LPARAM) &keycol);
				SendMessageW(listview, LVM_INSERTCOLUMN, 1, (LPARAM) &bindcol);

				pressAnyKeyDialog = NULL;
			}

		case WM_COMMAND:
			{
				switch(LOWORD(pWParam)) {
					// The user changed the map selection
					case IDC_MAP_SELECT:
						UpdateBindingLabels(pWindow);
						break;

					case IDC_CHANGE_BINDING:
						{
							char buffer[200];
							HWND combobox = GetDlgItem(pWindow, IDC_MAP_SELECT);
							SendMessage(combobox, WM_GETTEXT, 200, (LPARAM) &buffer);
							std::string mapname = buffer;

							// we need to populate the ListView with actions
							HWND hList = GetDlgItem(pWindow, IDC_CONTROL_BINDINGS);
							InputEventController::ActionMap map = controller->GetActionMap(mapname);

							// order bindings by defined ordering in ControlAction::listOrder
							std::map<int, int> orderMap;
							for(InputEventController::ActionMap::iterator it = map.begin(); it != map.end(); it++)
								orderMap[it->second->getListOrder()] = it->first;
							
							// user wants to change assigned binding
							int selectedIndex = SendMessage(hList, LVM_GETSELECTIONMARK, 0, 0);
							if(selectedIndex == -1)
								break; // apparently nothing is selected

							setControlHash = orderMap[selectedIndex];
							setControlMap = mapname;

							if(pressAnyKeyDialog == NULL) {
								WNDCLASSW lWinClass;

								lWinClass.style = CS_DBLCLKS;
								lWinClass.lpfnWndProc = PressKeyDialogFunc;
								lWinClass.cbClsExtra = 0;
								lWinClass.cbWndExtra = 0;
								lWinClass.hInstance = GetInstanceHandle();
								lWinClass.hIcon = NULL;
								lWinClass.hCursor = LoadCursor(NULL, IDC_ARROW);
								lWinClass.hbrBackground = (HBRUSH) COLOR_APPWORKSPACE + 1;
								lWinClass.lpszMenuName = NULL;
								lWinClass.lpszClassName = L"IDD_PRESS_ANY_KEY";

								lReturnValue = RegisterClassW(&lWinClass);

								// set HWND for when we get called back
								preferencesDialog = pWindow;

								RECT size = {0};
								GetWindowRect(app->GetWindowHandle(), &size);
								SetCursorPos(size.left + 110, size.top + 80); // move to center of new window

								pressAnyKeyDialog = CreateWindowW(
									L"IDD_PRESS_ANY_KEY",
									PACKAGE_NAME_L,
									(WS_POPUPWINDOW | WS_VISIBLE),
									size.left + 30,
									size.top + 30,
									160,
									100,
									app->GetWindowHandle(),
									NULL,
									GetInstanceHandle(),
									this);

								if(pressAnyKeyDialog == NULL) { // report error if necessary
									DWORD err = GetLastError();
									LPVOID errMsg;
					
									FormatMessage(
										FORMAT_MESSAGE_ALLOCATE_BUFFER |
										FORMAT_MESSAGE_FROM_SYSTEM,
										NULL,
										err,
										MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
										(LPTSTR) &errMsg,
										0, NULL);
									MessageBox(NULL, (const char*) errMsg, "AIEEE", MB_ICONERROR | MB_APPLMODAL | MB_OK);
									LocalFree(errMsg);
								}
							} else {
								// put focus back to already existing window
								EnableWindow(pWindow, false);
								EnableWindow(pressAnyKeyDialog, true);
							}
						}
						break;
				}
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					delete[] oldcontrols;
					oldcontrols = NULL;
					pressAnyKeyDialog = NULL;

					// reload controller
					app->ReloadController();

					cfg->Save();
					break;

				case PSN_RESET:
					delete[] oldcontrols;
					oldcontrols = NULL;
					pressAnyKeyDialog = NULL;
					break;

				case PSN_KILLACTIVE:
					// for receipt of PSN_APPLY
					SetWindowLong(pWindow, DWL_MSGRESULT, FALSE);
					break;
			}
	}

	return lReturnValue;
}

LRESULT ControlPrefsPage::PressKeyDialogProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam) {	
	static InputEventController *tmpControl = NULL;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_CREATE:
			// i18n
			SetWindowTextW(pWindow, Str::UW(_("Control Assignment")));

			// add a static control
			CreateWindowExW(0, L"STATIC", Str::UW(_("Press a control or wait 3 seconds to disable...")),
					WS_CHILD | WS_VISIBLE,
					5, 5, 150, 100,
					pWindow, 0, 0, 0);

			// disable main window
			EnableWindow(app->GetWindowHandle(), false);
			EnableWindow(pWindow, true);

			tmpControl = new InputEventController(pWindow, Control::UiHandlerPtr());
			tmpControl->CaptureNextInput(setControlHash, setControlMap);

			// set timer for 3 seconds
			SetTimer(pWindow, MRM_CONTROL_TIMER, 3000, NULL);

			// set polling timer since joystick events do not trigger a message
			// and the controller will not be polled otherwise
			SetTimer(pWindow, MRM_CONTROL_POLL, 100, NULL);
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				// Game control
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					break;

				case IDC_DISABLE:
					EndDialog(pWindow, IDC_DISABLE);
					break;
			}
			break;

		case WM_TIMER:
			switch (pWParam) {
				case MRM_CONTROL_TIMER:
					// 3 seconds are up, disable input
					tmpControl->DisableCaptureInput();
					tmpControl->SaveControllerConfig();
			
					// save new controls
					delete tmpControl;
					tmpControl = NULL;

					// unset the handle
					pressAnyKeyDialog = NULL;

					// now we have to tell the preferences dialog to refresh itself
					app->GetController()->ReloadConfig();
					UpdateBindingLabels(preferencesDialog);
					KillTimer(pWindow, MRM_CONTROL_TIMER);
					KillTimer(pWindow, MRM_CONTROL_POLL);
					EnableWindow(preferencesDialog, TRUE);
					DestroyWindow(pWindow);
					break;
				case MRM_CONTROL_POLL:
					KillTimer(pWindow, MRM_CONTROL_POLL);
					tmpControl->Poll();
					SetTimer(pWindow, MRM_CONTROL_POLL, 100, NULL);
			}
			break;
	}

	if(tmpControl != NULL) {
		tmpControl->Poll();

		// check if things are updated
		if(!tmpControl->IsCapturing()) {
			// the new key binding is set, now it's time to close the window
			tmpControl->SaveControllerConfig();

			delete tmpControl;
			tmpControl = NULL;

			app->GetController()->ReloadConfig();
			UpdateBindingLabels(preferencesDialog);
			pressAnyKeyDialog = NULL;
			KillTimer(pWindow, MRM_CONTROL_TIMER);
			EnableWindow(preferencesDialog, TRUE);
			DestroyWindow(pWindow);
		}
	}

	return DefWindowProc(pWindow, pMsgId, pWParam, pLParam);
}

LRESULT CALLBACK ControlPrefsPage::PressKeyDialogFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Route the message to the instance.
	ControlPrefsPage *dlg;
	if (message == WM_CREATE) {
		CREATESTRUCT *page = reinterpret_cast<CREATESTRUCT*>(lparam);
		dlg = reinterpret_cast<ControlPrefsPage*>(page->lpCreateParams);
		SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(page->lpCreateParams));
	}
	else {
		dlg = reinterpret_cast<ControlPrefsPage*>(GetWindowLong(hwnd, GWL_USERDATA));
		if (message == WM_DESTROY) {
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}

	return (dlg == NULL) ?
		DefWindowProc(hwnd, message, wparam, lparam) :
		dlg->PressKeyDialogProc(hwnd, message, wparam, lparam);
}

}  // namespace Client
}  // namespace HoverRace
