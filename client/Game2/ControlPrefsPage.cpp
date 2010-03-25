
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
#include "GameApp.h"

#include "resource.h"

#include "ControlPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Client;
using namespace HoverRace::Util;
using HoverRace::Client::Control::Controller;

#define MRM_CONTROL_TIMER   3
#define MRM_CONTROL_POLL    4

ControlPrefsPage::ControlPrefsPage(MR_GameApp *app) :
	SUPER(_("Controls"), IDD_CONTROL),
	app(app)
{
}

ControlPrefsPage::~ControlPrefsPage()
{
}

void ControlPrefsPage::UpdateDialogLabels(HWND pWindow)
{
	Config *cfg = Config::GetInstance();
	Controller * const controller = app->GetController();

	SetDlgItemTextW(pWindow, IDC_MOTOR_ON1, Str::UW(controller->toString(cfg->controls[0].motorOn).c_str()));
	SetDlgItemTextW(pWindow, IDC_RIGHT1, Str::UW(controller->toString(cfg->controls[0].right).c_str()));
	SetDlgItemTextW(pWindow, IDC_LEFT1, Str::UW(controller->toString(cfg->controls[0].left).c_str()));
	SetDlgItemTextW(pWindow, IDC_JUMP1, Str::UW(controller->toString(cfg->controls[0].jump).c_str()));
	SetDlgItemTextW(pWindow, IDC_FIRE1, Str::UW(controller->toString(cfg->controls[0].fire).c_str()));
	SetDlgItemTextW(pWindow, IDC_BRAKE1, Str::UW(controller->toString(cfg->controls[0].brake).c_str()));
	SetDlgItemTextW(pWindow, IDC_SELWEAPON1, Str::UW(controller->toString(cfg->controls[0].weapon).c_str()));
	SetDlgItemTextW(pWindow, IDC_LOOKBACK1, Str::UW(controller->toString(cfg->controls[0].lookBack).c_str()));

	SetDlgItemTextW(pWindow, IDC_MOTOR_ON2, Str::UW(controller->toString(cfg->controls[1].motorOn).c_str()));
	SetDlgItemTextW(pWindow, IDC_RIGHT2, Str::UW(controller->toString(cfg->controls[1].right).c_str()));
	SetDlgItemTextW(pWindow, IDC_LEFT2, Str::UW(controller->toString(cfg->controls[1].left).c_str()));
	SetDlgItemTextW(pWindow, IDC_JUMP2, Str::UW(controller->toString(cfg->controls[1].jump).c_str()));
	SetDlgItemTextW(pWindow, IDC_FIRE2, Str::UW(controller->toString(cfg->controls[1].fire).c_str()));
	SetDlgItemTextW(pWindow, IDC_BRAKE2, Str::UW(controller->toString(cfg->controls[1].brake).c_str()));
	SetDlgItemTextW(pWindow, IDC_SELWEAPON2, Str::UW(controller->toString(cfg->controls[1].weapon).c_str()));
	SetDlgItemTextW(pWindow, IDC_LOOKBACK2, Str::UW(controller->toString(cfg->controls[1].lookBack).c_str()));

	SetDlgItemTextW(pWindow, IDC_MOTOR_ON3, Str::UW(controller->toString(cfg->controls[2].motorOn).c_str()));
	SetDlgItemTextW(pWindow, IDC_RIGHT3, Str::UW(controller->toString(cfg->controls[2].right).c_str()));
	SetDlgItemTextW(pWindow, IDC_LEFT3, Str::UW(controller->toString(cfg->controls[2].left).c_str()));
	SetDlgItemTextW(pWindow, IDC_JUMP3, Str::UW(controller->toString(cfg->controls[2].jump).c_str()));
	SetDlgItemTextW(pWindow, IDC_FIRE3, Str::UW(controller->toString(cfg->controls[2].fire).c_str()));
	SetDlgItemTextW(pWindow, IDC_BRAKE3, Str::UW(controller->toString(cfg->controls[2].brake).c_str()));
	SetDlgItemTextW(pWindow, IDC_SELWEAPON3, Str::UW(controller->toString(cfg->controls[2].weapon).c_str()));
	SetDlgItemTextW(pWindow, IDC_LOOKBACK3, Str::UW(controller->toString(cfg->controls[2].lookBack).c_str()));

	SetDlgItemTextW(pWindow, IDC_MOTOR_ON4, Str::UW(controller->toString(cfg->controls[3].motorOn).c_str()));
	SetDlgItemTextW(pWindow, IDC_RIGHT4, Str::UW(controller->toString(cfg->controls[3].right).c_str()));
	SetDlgItemTextW(pWindow, IDC_LEFT4, Str::UW(controller->toString(cfg->controls[3].left).c_str()));
	SetDlgItemTextW(pWindow, IDC_JUMP4, Str::UW(controller->toString(cfg->controls[3].jump).c_str()));
	SetDlgItemTextW(pWindow, IDC_FIRE4, Str::UW(controller->toString(cfg->controls[3].fire).c_str()));
	SetDlgItemTextW(pWindow, IDC_BRAKE4, Str::UW(controller->toString(cfg->controls[3].brake).c_str()));
	SetDlgItemTextW(pWindow, IDC_SELWEAPON4, Str::UW(controller->toString(cfg->controls[3].weapon).c_str()));
	SetDlgItemTextW(pWindow, IDC_LOOKBACK4, Str::UW(controller->toString(cfg->controls[3].lookBack).c_str()));
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

	Controller * const controller = app->GetController();

	BOOL lReturnValue = FALSE;
	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			// i18nize the choices
			SetDlgItemTextW(pWindow, IDC_PLAYER1, Str::UW(_("Player 1")));
			SetDlgItemTextW(pWindow, IDC_PLAYER2, Str::UW(_("Player 2")));
			SetDlgItemTextW(pWindow, IDC_PLAYER3, Str::UW(_("Player 3")));
			SetDlgItemTextW(pWindow, IDC_PLAYER4, Str::UW(_("Player 4")));
			SetDlgItemTextW(pWindow, IDC_MOTOR_ON, Str::UW(_("Motor On:")));
			SetDlgItemTextW(pWindow, IDC_TURN_RIGHT, Str::UW(_("Turn Right:")));
			SetDlgItemTextW(pWindow, IDC_TURN_LEFT, Str::UW(_("Turn Left:")));
			SetDlgItemTextW(pWindow, IDC_BRAKE, Str::UW(_("Brake:")));
			SetDlgItemTextW(pWindow, IDC_JUMP, Str::UW(_("Jump:")));
			SetDlgItemTextW(pWindow, IDC_FIRE_WEAPON, Str::UW(_("Fire Weapon:")));
			SetDlgItemTextW(pWindow, IDC_SELECT_WEAPON, Str::UW(_("Select Weapon:")));
			SetDlgItemTextW(pWindow, IDC_LOOK_BACK, Str::UW(_("Look Back:")));

			UpdateDialogLabels(pWindow);

			pressAnyKeyDialog = NULL;

			// set up new controls
			if (oldcontrols == NULL) {
				oldcontrols = new Config::cfg_controls_t[cfg->MAX_PLAYERS];
				memcpy((void *) oldcontrols, (void *) cfg->controls, sizeof(cfg->controls));
			}
			break;

		case WM_COMMAND:
			{
				// reset
				setControlControl = 0;
				setControlPlayer = 0;

				switch(LOWORD(pWParam)) {
					case IDC_MOTOR_ON1:
					case IDC_MOTOR_ON2:
					case IDC_MOTOR_ON3:
					case IDC_MOTOR_ON4:
						setControlControl = CTL_MOTOR_ON;
						break;

					case IDC_LEFT1:
					case IDC_LEFT2:
					case IDC_LEFT3:
					case IDC_LEFT4:
						setControlControl = CTL_LEFT;
						break;

					case IDC_RIGHT1:
					case IDC_RIGHT2:
					case IDC_RIGHT3:
					case IDC_RIGHT4:
						setControlControl = CTL_RIGHT;
						break;

					case IDC_JUMP1:
					case IDC_JUMP2:
					case IDC_JUMP3:
					case IDC_JUMP4:
						setControlControl = CTL_JUMP;
						break;
					
					case IDC_FIRE1:
					case IDC_FIRE2:
					case IDC_FIRE3:
					case IDC_FIRE4:
						setControlControl = CTL_FIRE;
						break;
					
					case IDC_BRAKE1:
					case IDC_BRAKE2:
					case IDC_BRAKE3:
					case IDC_BRAKE4:
						setControlControl = CTL_BRAKE;
						break;
					
					case IDC_SELWEAPON1:
					case IDC_SELWEAPON2:
					case IDC_SELWEAPON3:
					case IDC_SELWEAPON4:
						setControlControl = CTL_WEAPON;
						break;

					case IDC_LOOKBACK1:
					case IDC_LOOKBACK2:
					case IDC_LOOKBACK3:
					case IDC_LOOKBACK4:
						setControlControl = CTL_LOOKBACK;
						break;
				}

				switch(LOWORD(pWParam)) {
					case IDC_MOTOR_ON1:
					case IDC_LEFT1:
					case IDC_RIGHT1:
					case IDC_JUMP1:
					case IDC_FIRE1:
					case IDC_BRAKE1:
					case IDC_SELWEAPON1:
					case IDC_LOOKBACK1:
						setControlPlayer = 0;
						break;

					case IDC_MOTOR_ON2:
					case IDC_LEFT2:
					case IDC_RIGHT2:
					case IDC_JUMP2:
					case IDC_FIRE2:
					case IDC_BRAKE2:
					case IDC_SELWEAPON2:
					case IDC_LOOKBACK2:
						setControlPlayer = 1;
						break;

					case IDC_MOTOR_ON3:
					case IDC_LEFT3:
					case IDC_RIGHT3:
					case IDC_JUMP3:
					case IDC_FIRE3:
					case IDC_BRAKE3:
					case IDC_SELWEAPON3:
					case IDC_LOOKBACK3:
						setControlPlayer = 2;
						break;

					case IDC_MOTOR_ON4:
					case IDC_LEFT4:
					case IDC_RIGHT4:
					case IDC_JUMP4:
					case IDC_FIRE4:
					case IDC_BRAKE4:
					case IDC_SELWEAPON4:
					case IDC_LOOKBACK4:
						setControlPlayer = 3;
						break;
				}

				// create dialog window by hand, the hard way
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
					//lWinClass.lpszMenuName = MAKEINTRESOURCEW(FIREBALL_MAIN_MENU);
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
							(LPTSTR)&errMsg,
							0, NULL);
						MessageBox(NULL, (const char*)errMsg, "AIEEE", MB_ICONERROR | MB_APPLMODAL | MB_OK);
						LocalFree(errMsg);
					}
				} else {
					// put focus back to already existing window
					EnableWindow(pWindow, false);
					EnableWindow(pressAnyKeyDialog, true);
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
	static Control::Controller *tmpControl = NULL;

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

			tmpControl = new Control::Controller(pWindow);
			tmpControl->captureNextInput(setControlControl, setControlPlayer, pWindow);

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
					tmpControl->disableInput(setControlControl, setControlPlayer);
					tmpControl->stopCapture();
			
					// save new controls
					delete tmpControl;
					tmpControl = NULL;


					// unset the handle
					pressAnyKeyDialog = NULL;

					// now we have to tell the preferences dialog to refresh itself
					UpdateDialogLabels(preferencesDialog);
					KillTimer(pWindow, MRM_CONTROL_TIMER);
					KillTimer(pWindow, MRM_CONTROL_POLL);
					EnableWindow(preferencesDialog, TRUE);
					DestroyWindow(pWindow);
					break;
				case MRM_CONTROL_POLL:
					KillTimer(pWindow, MRM_CONTROL_POLL);
					tmpControl->poll();
					SetTimer(pWindow, MRM_CONTROL_POLL, 100, NULL);
			}
			break;
	}

	if(tmpControl != NULL) {
		tmpControl->poll();

		// check if things are updated
		if(tmpControl->controlsUpdated()) {
			// the new key binding is set, now it's time to close the window
			tmpControl->saveControls();
			delete tmpControl;
			tmpControl = NULL;

			UpdateDialogLabels(preferencesDialog);
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
