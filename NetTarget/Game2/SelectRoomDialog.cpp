
// SelectRoomDialog.h
// The room list selector.
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

#include "StdAfx.h"

#include <mmsystem.h>

#include "../Util/Config.h"
//#include "../../engine/Util/Str.h"

#include "resource.h"

#include "SelectRoomDialog.h"

using namespace HoverRace::Client;
//using namespace HoverRace::Util;

namespace {
	struct DialogRect
	{
		int x;
		int y;
		int width;
		int height;
	};

	static const int INTERNET_PARAMS_WIDTH = 170;
	static const int INTERNET_PARAMS_TOP_HEIGHT = 112;
	static const int INTERNET_PARAMS_BUTTONS_LEFT = 45;
	static const int INTERNET_PARAMS_BUTTONS_RIGHT = 162;
	static const int INTERNET_PARAMS_ADD_BUTTON_Y = 89;
	static const int INTERNET_PARAMS_ACTION_GAP = 4;
	static const int INTERNET_PARAMS_ADD_BUTTON_WIDTH =
		((INTERNET_PARAMS_BUTTONS_RIGHT - INTERNET_PARAMS_BUTTONS_LEFT) -
			INTERNET_PARAMS_ACTION_GAP) / 2;
	static const int INTERNET_PARAMS_ADD_BUTTON_HEIGHT = 14;
	static const int INTERNET_PARAMS_REMOVE_BUTTON_WIDTH = 56;
	static const int INTERNET_PARAMS_ROW_Y = 105;
	static const int INTERNET_PARAMS_ROW_HEIGHT = 18;
	static const int INTERNET_PARAMS_ROW_TO_BUTTON_GAP = 2;
	static const int INTERNET_PARAMS_BUTTONS_X_OK = 45;
	static const int INTERNET_PARAMS_BUTTONS_X_CANCEL = 112;
	static const int INTERNET_PARAMS_BUTTONS_WIDTH = 50;
	static const int INTERNET_PARAMS_BUTTONS_HEIGHT = 14;
	static const int INTERNET_PARAMS_BOTTOM_MARGIN = 7;
	static const int INTERNET_PARAMS_BASE_BUTTON_Y = 111;

	static const int IDC_LOCAL_ADD_BUTTON = 2000;
	static const int IDC_LOCAL_REMOVE_BUTTON = 2001;
	static const int IDC_LOCAL_PLAYER_LABEL_BASE = 2010;
	static const int IDC_LOCAL_PLAYER_NAME_BASE = 2020;
	static const int IDC_LOCAL_PLAYER_CONTROLS_BASE = 2030;

	enum MR_InControler { MR_KDB, MR_JOY1, MR_JOY2, MR_JOY3, MR_JOY4 };

	enum
	{
		AxeTop,
		AxeBottom,
		AxeLeft,
		AxeRight,
		Btn1, Btn2, Btn3, Btn4, Btn5, Btn6, Btn7, Btn8
	};

	struct ControlKey
	{
		const char *oldKeyName;
		int stringId;
		MR_InControler controller;
		int keyValue;
	};

	static const ControlKey KeyChoice[] =
	{
		{ "Disable", IDS_DISABLE, MR_KDB, 0 },
		{ "Shift", IDS_SHIFT, MR_KDB, VK_SHIFT },
		{ "Ctrl", IDS_CTRL, MR_KDB, VK_CONTROL },
		{ "Up", IDS_UP, MR_KDB, VK_UP },
		{ "Down", IDS_DOWN, MR_KDB, VK_DOWN },
		{ "Right", IDS_RIGHT, MR_KDB, VK_RIGHT },
		{ "Left", IDS_LEFT, MR_KDB, VK_LEFT },
		{ "Insert", IDS_INS, MR_KDB, VK_INSERT },
		{ "Delete", IDS_DEL, MR_KDB, VK_DELETE },
		{ "Enter", IDS_ENTER, MR_KDB, VK_RETURN },
		{ "End", IDS_END, MR_KDB, VK_END },
		{ "Tab", IDS_TAB, MR_KDB, VK_TAB },
		{ "Joystick Btn1", IDS_JOY1BTN1, MR_JOY1, Btn1 },
		{ "Joystick Btn2", IDS_JOY1BTN2, MR_JOY1, Btn2 },
		{ "Joystick Btn3", IDS_JOY1BTN3, MR_JOY1, Btn3 },
		{ "Joystick Btn4", IDS_JOY1BTN4, MR_JOY1, Btn4 },
		{ "Joystick Btn5", IDS_JOY1BTN5, MR_JOY1, Btn5 },
		{ "Joystick Btn6", IDS_JOY1BTN6, MR_JOY1, Btn6 },
		{ "Joystick Btn7", IDS_JOY1BTN7, MR_JOY1, Btn7 },
		{ "Joystick Btn8", IDS_JOY1BTN8, MR_JOY1, Btn8 },
		{ "Joystick Top", IDS_JOY1TOP, MR_JOY1, AxeTop },
		{ "Joystick Bottom", IDS_JOY1DOWN, MR_JOY1, AxeBottom },
		{ "Joystick Right", IDS_JOY1RIGHT, MR_JOY1, AxeRight },
		{ "Joystick Left", IDS_JOY1LEFT, MR_JOY1, AxeLeft },
		{ "Joystick2 Btn1", IDS_JOY2BTN1, MR_JOY2, Btn1 },
		{ "Joystick2 Btn2", IDS_JOY2BTN2, MR_JOY2, Btn2 },
		{ "Joystick2 Btn3", IDS_JOY2BTN3, MR_JOY2, Btn3 },
		{ "Joystick2 Btn4", IDS_JOY2BTN4, MR_JOY2, Btn4 },
		{ "Joystick2 Btn5", IDS_JOY2BTN5, MR_JOY2, Btn5 },
		{ "Joystick2 Btn6", IDS_JOY2BTN6, MR_JOY2, Btn6 },
		{ "Joystick2 Btn7", IDS_JOY2BTN7, MR_JOY2, Btn7 },
		{ "Joystick2 Btn8", IDS_JOY2BTN8, MR_JOY2, Btn8 },
		{ "Joystick2 Top", IDS_JOY2TOP, MR_JOY2, AxeTop },
		{ "Joystick2 Bottom", IDS_JOY2DOWN, MR_JOY2, AxeBottom },
		{ "Joystick2 Right", IDS_JOY2RIGHT, MR_JOY2, AxeRight },
		{ "Joystick2 Left", IDS_JOY2LEFT, MR_JOY2, AxeLeft },
		{ "Joystick3 Btn1", IDS_JOY3BTN1, MR_JOY3, Btn1 },
		{ "Joystick3 Btn2", IDS_JOY3BTN2, MR_JOY3, Btn2 },
		{ "Joystick3 Btn3", IDS_JOY3BTN3, MR_JOY3, Btn3 },
		{ "Joystick3 Btn4", IDS_JOY3BTN4, MR_JOY3, Btn4 },
		{ "Joystick3 Btn5", IDS_JOY3BTN5, MR_JOY3, Btn5 },
		{ "Joystick3 Btn6", IDS_JOY3BTN6, MR_JOY3, Btn6 },
		{ "Joystick3 Btn7", IDS_JOY3BTN7, MR_JOY3, Btn7 },
		{ "Joystick3 Btn8", IDS_JOY3BTN8, MR_JOY3, Btn8 },
		{ "Joystick3 Top", IDS_JOY3TOP, MR_JOY3, AxeTop },
		{ "Joystick3 Bottom", IDS_JOY3DOWN, MR_JOY3, AxeBottom },
		{ "Joystick3 Right", IDS_JOY3RIGHT, MR_JOY3, AxeRight },
		{ "Joystick3 Left", IDS_JOY3LEFT, MR_JOY3, AxeLeft },
		{ "Joystick4 Btn1", IDS_JOY4BTN1, MR_JOY4, Btn1 },
		{ "Joystick4 Btn2", IDS_JOY4BTN2, MR_JOY4, Btn2 },
		{ "Joystick4 Btn3", IDS_JOY4BTN3, MR_JOY4, Btn3 },
		{ "Joystick4 Btn4", IDS_JOY4BTN4, MR_JOY4, Btn4 },
		{ "Joystick4 Btn5", IDS_JOY4BTN5, MR_JOY4, Btn5 },
		{ "Joystick4 Btn6", IDS_JOY4BTN6, MR_JOY4, Btn6 },
		{ "Joystick4 Btn7", IDS_JOY4BTN7, MR_JOY4, Btn7 },
		{ "Joystick4 Btn8", IDS_JOY4BTN8, MR_JOY4, Btn8 },
		{ "Joystick4 Top", IDS_JOY4TOP, MR_JOY4, AxeTop },
		{ "Joystick4 Bottom", IDS_JOY4DOWN, MR_JOY4, AxeBottom },
		{ "Joystick4 Right", IDS_JOY4RIGHT, MR_JOY4, AxeRight },
		{ "Joystick4 Left", IDS_JOY4LEFT, MR_JOY4, AxeLeft },
		{ "Space", IDS_SPACE, MR_KDB, VK_SPACE },
		{ "A Key", IDS_KEY_A, MR_KDB, 'A' },
		{ "B Key", IDS_KEY_B, MR_KDB, 'B' },
		{ "C Key", IDS_KEY_C, MR_KDB, 'C' },
		{ "D Key", IDS_KEY_D, MR_KDB, 'D' },
		{ "E Key", IDS_KEY_E, MR_KDB, 'E' },
		{ "F Key", IDS_KEY_F, MR_KDB, 'F' },
		{ "G Key", IDS_KEY_G, MR_KDB, 'G' },
		{ "H Key", IDS_KEY_H, MR_KDB, 'H' },
		{ "I Key", IDS_KEY_I, MR_KDB, 'I' },
		{ "J Key", IDS_KEY_J, MR_KDB, 'J' },
		{ "K Key", IDS_KEY_K, MR_KDB, 'K' },
		{ "L Key", IDS_KEY_L, MR_KDB, 'L' },
		{ "M Key", IDS_KEY_M, MR_KDB, 'M' },
		{ "N Key", IDS_KEY_N, MR_KDB, 'N' },
		{ "O Key", IDS_KEY_O, MR_KDB, 'O' },
		{ "P Key", IDS_KEY_P, MR_KDB, 'P' },
		{ "Q Key", IDS_KEY_Q, MR_KDB, 'Q' },
		{ "R Key", IDS_KEY_R, MR_KDB, 'R' },
		{ "S Key", IDS_KEY_S, MR_KDB, 'S' },
		{ "T Key", IDS_KEY_T, MR_KDB, 'T' },
		{ "U Key", IDS_KEY_U, MR_KDB, 'U' },
		{ "V Key", IDS_KEY_V, MR_KDB, 'V' },
		{ "W Key", IDS_KEY_W, MR_KDB, 'W' },
		{ "X Key", IDS_KEY_X, MR_KDB, 'X' },
		{ "Y Key", IDS_KEY_Y, MR_KDB, 'Y' },
		{ "Z Key", IDS_KEY_Z, MR_KDB, 'Z' },
		{ "Not a Key", IDS_DISABLE, MR_KDB, 0 },
		{ "Not a Key", IDS_DISABLE, MR_KDB, 0 },
		{ "Not a Key", IDS_DISABLE, MR_KDB, 0 },
		{ "Not a Key", IDS_DISABLE, MR_KDB, 0 },
	};

	static const int NB_KEY_PLAYER_1 = 60;
	static const int NB_KEY_PLAYER_2 = 87;

	enum ControlAction
	{
		CtlMotorOn,
		CtlBrake,
		CtlLeft,
		CtlRight,
		CtlJump,
		CtlFire,
		CtlWeapon,
		CtlLookBack,
		NB_CONTROL_ACTIONS
	};

	static const char *CONTROL_ACTION_NAMES[NB_CONTROL_ACTIONS] =
	{
		"Motor On",
		"Brake",
		"Turn Left",
		"Turn Right",
		"Jump",
		"Fire Weapon",
		"Select Weapon",
		"Look Back"
	};

	struct CachedJoystickState
	{
		BOOL loaded[MR_Config::MAX_PLAYERS];
		BOOL available[MR_Config::MAX_PLAYERS];
		JOYINFOEX state[MR_Config::MAX_PLAYERS];
	};

	struct ControlCaptureDialogState
	{
		int maxKeyIndex;
		int result;
		BOOL previousStates[sizeof(KeyChoice) / sizeof(KeyChoice[0])];
		char prompt[128];
	};

	static CachedJoystickState gCachedJoystickState;
	static const UINT_PTR CONTROL_CAPTURE_TIMER_ID = 1;
	static const UINT CONTROL_CAPTURE_TIMER_INTERVAL_MS = 50;

	static void MoveDialogItem(HWND hwnd, int ctrlId, const DialogRect &rect)
	{
		HWND ctrl = GetDlgItem(hwnd, ctrlId);
		if(ctrl == NULL) return;
		RECT pixelRect = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
		MapDialogRect(hwnd, &pixelRect);
		MoveWindow(ctrl, pixelRect.left, pixelRect.top,
			pixelRect.right - pixelRect.left, pixelRect.bottom - pixelRect.top, TRUE);
	}

	static void MoveWindowDialogUnits(HWND hwnd, HWND ctrl, const DialogRect &rect)
	{
		if(ctrl == NULL) return;
		RECT pixelRect = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
		MapDialogRect(hwnd, &pixelRect);
		MoveWindow(ctrl, pixelRect.left, pixelRect.top,
			pixelRect.right - pixelRect.left, pixelRect.bottom - pixelRect.top, TRUE);
	}

	static int GetPlayerKeyCount(int playerIdx)
	{
		return (playerIdx == 0) ? NB_KEY_PLAYER_1 : NB_KEY_PLAYER_2;
	}

	static int ClampKeyChoiceIndex(int playerIdx, int keyIndex)
	{
		const int maxIndex = GetPlayerKeyCount(playerIdx) - 1;
		if((keyIndex < 0) || (keyIndex > maxIndex)) return 0;
		return keyIndex;
	}

	static int GetControlBinding(const MR_Config::cfg_controls_t &controls, int actionIdx)
	{
		switch(actionIdx) {
			case CtlMotorOn: return controls.motorOn;
			case CtlBrake: return controls.brake;
			case CtlLeft: return controls.left;
			case CtlRight: return controls.right;
			case CtlJump: return controls.jump;
			case CtlFire: return controls.fire;
			case CtlWeapon: return controls.weapon;
			default: return controls.lookBack;
		}
	}

	static void SetControlBinding(MR_Config::cfg_controls_t &controls, int actionIdx, int keyIndex)
	{
		switch(actionIdx) {
			case CtlMotorOn: controls.motorOn = keyIndex; break;
			case CtlBrake: controls.brake = keyIndex; break;
			case CtlLeft: controls.left = keyIndex; break;
			case CtlRight: controls.right = keyIndex; break;
			case CtlJump: controls.jump = keyIndex; break;
			case CtlFire: controls.fire = keyIndex; break;
			case CtlWeapon: controls.weapon = keyIndex; break;
			default: controls.lookBack = keyIndex; break;
		}
	}

	static int GetJoystickIndex(MR_InControler controller)
	{
		switch(controller) {
			case MR_JOY1: return 0;
			case MR_JOY2: return 1;
			case MR_JOY3: return 2;
			case MR_JOY4: return 3;
			default: return -1;
		}
	}

	static BOOL ReadJoystickState(int joystickIdx, JOYINFOEX *joystick)
	{
		if((joystickIdx < 0) || (joystickIdx >= MR_Config::MAX_PLAYERS) || (joystick == NULL)) {
			return FALSE;
		}
		memset(joystick, 0, sizeof(*joystick));
		joystick->dwSize = sizeof(*joystick);
		joystick->dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
		return (joyGetPosEx(static_cast<UINT>(joystickIdx), joystick) == JOYERR_NOERROR);
	}

	static BOOL IsJoystickBindingPressed(const JOYINFOEX &joystick, int keyValue)
	{
		switch(keyValue) {
			case AxeTop: return (joystick.dwYpos < 16000);
			case AxeBottom: return (joystick.dwYpos > 48000);
			case AxeLeft: return (joystick.dwXpos < 16000);
			case AxeRight: return (joystick.dwXpos > 48000);
			case Btn1: return ((joystick.dwButtons & 1) != 0);
			case Btn2: return ((joystick.dwButtons & 2) != 0);
			case Btn3: return ((joystick.dwButtons & 4) != 0);
			case Btn4: return ((joystick.dwButtons & 8) != 0);
			case Btn5: return ((joystick.dwButtons & 16) != 0);
			case Btn6: return ((joystick.dwButtons & 32) != 0);
			case Btn7: return ((joystick.dwButtons & 64) != 0);
			case Btn8: return ((joystick.dwButtons & 128) != 0);
			default: return FALSE;
		}
	}

	static BOOL TryReadKeyChoiceState(int keyIndex, BOOL useCachedJoystickState, BOOL *pressed)
	{
		if((pressed == NULL) || (keyIndex < 0) ||
			(keyIndex >= static_cast<int>(sizeof(KeyChoice) / sizeof(KeyChoice[0])))) {
			return FALSE;
		}
		*pressed = FALSE;

		switch(KeyChoice[keyIndex].controller) {
			case MR_KDB:
				*pressed = ((GetAsyncKeyState(KeyChoice[keyIndex].keyValue) & 0x8000) != 0);
				return TRUE;

			case MR_JOY1:
			case MR_JOY2:
			case MR_JOY3:
			case MR_JOY4:
			{
				const int joystickIdx = GetJoystickIndex(KeyChoice[keyIndex].controller);
				JOYINFOEX joystick;
				BOOL available = FALSE;

				if(useCachedJoystickState) {
					if(!gCachedJoystickState.loaded[joystickIdx]) {
						gCachedJoystickState.loaded[joystickIdx] = TRUE;
						gCachedJoystickState.available[joystickIdx] =
							ReadJoystickState(joystickIdx, &gCachedJoystickState.state[joystickIdx]);
					}
					available = gCachedJoystickState.available[joystickIdx];
					if(available) joystick = gCachedJoystickState.state[joystickIdx];
				}
				else {
					available = ReadJoystickState(joystickIdx, &joystick);
				}

				if(available) {
					*pressed = IsJoystickBindingPressed(joystick, KeyChoice[keyIndex].keyValue);
				}
				return TRUE;
			}

			default:
				return FALSE;
		}
	}

	static BOOL CALLBACK ControlCaptureDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
	{
		ControlCaptureDialogState *state =
			reinterpret_cast<ControlCaptureDialogState *>(GetWindowLongPtr(pWindow, GWLP_USERDATA));

		switch(pMsgId) {
			case WM_INITDIALOG:
				state = reinterpret_cast<ControlCaptureDialogState *>(pLParam);
				SetWindowLongPtr(pWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
				SetDlgItemText(pWindow, IDC_CAPTURE_PROMPT, state->prompt);
				for(int keyIndex = 0; keyIndex <= state->maxKeyIndex; ++keyIndex) {
					TryReadKeyChoiceState(keyIndex, FALSE, &state->previousStates[keyIndex]);
				}
				SetTimer(pWindow, CONTROL_CAPTURE_TIMER_ID, CONTROL_CAPTURE_TIMER_INTERVAL_MS, NULL);
				SetFocus(pWindow);
				return FALSE;

			case WM_TIMER:
				if((state != NULL) && (pWParam == CONTROL_CAPTURE_TIMER_ID)) {
					for(int keyIndex = 1; keyIndex <= state->maxKeyIndex; ++keyIndex) {
						BOOL pressed = FALSE;
						TryReadKeyChoiceState(keyIndex, FALSE, &pressed);
						if(pressed && !state->previousStates[keyIndex]) {
							state->result = keyIndex;
							EndDialog(pWindow, IDOK);
							return TRUE;
						}
						state->previousStates[keyIndex] = pressed;
					}
				}
				break;

			case WM_COMMAND:
				switch(LOWORD(pWParam)) {
					case IDC_CAPTURE_CANCEL:
						EndDialog(pWindow, IDC_CAPTURE_CANCEL);
						return TRUE;
					case IDCANCEL:
						if(state != NULL) state->result = 0;
						EndDialog(pWindow, IDOK);
						return TRUE;
				}
				break;

			case WM_CLOSE:
				EndDialog(pWindow, IDC_CAPTURE_CANCEL);
				return TRUE;

			case WM_DESTROY:
				KillTimer(pWindow, CONTROL_CAPTURE_TIMER_ID);
				break;
		}
		return FALSE;
	}

	static BOOL CaptureControlBinding(HWND pWindow, int playerIdx, const char *prompt, int *bindingIndex)
	{
		ControlCaptureDialogState state;
		HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(pWindow, GWLP_HINSTANCE));

		memset(&state, 0, sizeof(state));
		memset(&gCachedJoystickState, 0, sizeof(gCachedJoystickState));
		state.maxKeyIndex = GetPlayerKeyCount(playerIdx) - 1;
		state.result = ClampKeyChoiceIndex(playerIdx, *bindingIndex);
		lstrcpyn(state.prompt, prompt, sizeof(state.prompt));

		if(DialogBoxParam(instance, MAKEINTRESOURCE(IDD_CONTROL_CAPTURE), pWindow,
			ControlCaptureDialogFunc, reinterpret_cast<LPARAM>(&state)) == IDOK) {
			*bindingIndex = ClampKeyChoiceIndex(playerIdx, state.result);
			return TRUE;
		}
		return FALSE;
	}

	static void BuildControlPrompt(char *buffer, int bufferLen, int playerIdx,
		int actionIdx, int step = 0, int totalSteps = 0)
	{
		if(step > 0) {
			sprintf(buffer, "Player %d - %s (%d/%d)", playerIdx + 1,
				CONTROL_ACTION_NAMES[actionIdx], step, totalSteps);
		}
		else {
			sprintf(buffer, "Player %d - %s", playerIdx + 1,
				CONTROL_ACTION_NAMES[actionIdx]);
		}
	}
}

SelectRoomDialog::SelectRoomDialog(const std::string &playerName,
	int onlinePartySize, const std::string *onlinePartyNames) :
	SUPER(MR_Config::GetInstance()->net.mainServer),
	playerName(playerName), onlinePartySize(max(1, min(onlinePartySize,
		MR_MAX_LOCAL_PLAYER))),
	addPlayerButton(NULL),
	removePlayerButton(NULL),
	finished(false)
{
	MR_Config *cfg = MR_Config::GetInstance();
	for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
		this->onlinePartyNames[i] =
			(onlinePartyNames != NULL) ? onlinePartyNames[i] : "";
		tempControls[i] = cfg->controls[i];
		playerLabels[i] = NULL;
		nameEdits[i] = NULL;
		controlButtons[i] = NULL;
	}
	if(this->onlinePartyNames[0].empty()) {
		this->onlinePartyNames[0] = playerName;
	}
	for(int i = 1; i < this->onlinePartySize; ++i) {
		if(this->onlinePartyNames[i].empty()) {
			char generated[32];
			sprintf(generated, "%s %d", playerName.c_str(), i + 1);
			this->onlinePartyNames[i] = generated;
		}
	}
}

SelectRoomDialog::~SelectRoomDialog()
{
}

const std::string &SelectRoomDialog::GetPlayerName() const
{
	return playerName;
}

int SelectRoomDialog::GetOnlinePartySize() const
{
	return onlinePartySize;
}

const std::string &SelectRoomDialog::GetOnlinePartyName(int idx) const
{
	static const std::string EMPTY;
	return (idx >= 0 && idx < MR_MAX_LOCAL_PLAYER) ? onlinePartyNames[idx] : EMPTY;
}

void SelectRoomDialog::CreateDynamicControls(HWND hwnd)
{
	HFONT font = reinterpret_cast<HFONT>(SendMessage(hwnd, WM_GETFONT, 0, 0));

	addPlayerButton = CreateWindowEx(0, "BUTTON", "Add Local Player",
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(IDC_LOCAL_ADD_BUTTON),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), NULL);
	SendMessage(addPlayerButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
	removePlayerButton = CreateWindowEx(0, "BUTTON", "Remove Player",
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(IDC_LOCAL_REMOVE_BUTTON),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), NULL);
	SendMessage(removePlayerButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		playerLabels[i] = CreateWindowEx(0, "STATIC", "",
			WS_CHILD | SS_LEFT,
			0, 0, 0, 0, hwnd,
			reinterpret_cast<HMENU>(IDC_LOCAL_PLAYER_LABEL_BASE + i),
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), NULL);
		nameEdits[i] = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL,
			0, 0, 0, 0, hwnd,
			reinterpret_cast<HMENU>(IDC_LOCAL_PLAYER_NAME_BASE + i),
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), NULL);
		controlButtons[i] = CreateWindowEx(0, "BUTTON", "Controls",
			WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd,
			reinterpret_cast<HMENU>(IDC_LOCAL_PLAYER_CONTROLS_BASE + i),
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), NULL);

		SendMessage(playerLabels[i], WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		SendMessage(nameEdits[i], WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		SendMessage(controlButtons[i], WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
		SendMessage(nameEdits[i], EM_SETLIMITTEXT, 31, 0);
	}
}

void SelectRoomDialog::SyncPlayerNamesFromControls(HWND hwnd)
{
	char buffer[64];
	for(int i = 1; i < onlinePartySize; ++i) {
		if(nameEdits[i] == NULL) continue;
		GetWindowText(nameEdits[i], buffer, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		onlinePartyNames[i] = buffer;
	}
}

void SelectRoomDialog::UpdatePlayerRows(HWND hwnd)
{
	char buffer[64];
	const BOOL canAdd = (onlinePartySize < MR_MAX_LOCAL_PLAYER) ? TRUE : FALSE;
	const BOOL canRemove = (onlinePartySize > 1) ? TRUE : FALSE;

	if(addPlayerButton != NULL) {
		ShowWindow(addPlayerButton, SW_SHOW);
		EnableWindow(addPlayerButton, canAdd);
		SetWindowText(addPlayerButton, "Add Player");
	}
	if(removePlayerButton != NULL) {
		ShowWindow(removePlayerButton, SW_SHOW);
		EnableWindow(removePlayerButton, canRemove);
	}

	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		const BOOL show = (i < onlinePartySize) ? TRUE : FALSE;
		if(show) {
			sprintf(buffer, "Player %d:", i + 1);
			SetWindowText(playerLabels[i], buffer);
			SetWindowText(nameEdits[i], onlinePartyNames[i].c_str());
		}
		ShowWindow(playerLabels[i], show ? SW_SHOW : SW_HIDE);
		ShowWindow(nameEdits[i], show ? SW_SHOW : SW_HIDE);
		ShowWindow(controlButtons[i], show ? SW_SHOW : SW_HIDE);
	}
}

void SelectRoomDialog::ApplyDialogLayout(HWND hwnd)
{
	const int rowCount = max(0, onlinePartySize - 1);
	const int addButtonY = INTERNET_PARAMS_ADD_BUTTON_Y;
	const int rowBaseY = INTERNET_PARAMS_ROW_Y;
	const int buttonY = (rowCount == 0) ?
		INTERNET_PARAMS_BASE_BUTTON_Y :
		(rowBaseY + (rowCount * INTERNET_PARAMS_ROW_HEIGHT) +
			INTERNET_PARAMS_ROW_TO_BUTTON_GAP);
	const int dialogHeight = buttonY + INTERNET_PARAMS_BUTTONS_HEIGHT +
		INTERNET_PARAMS_BOTTOM_MARGIN;
	DialogRect rect;

	rect.x = INTERNET_PARAMS_BUTTONS_LEFT;
	rect.y = addButtonY;
	rect.width = INTERNET_PARAMS_ADD_BUTTON_WIDTH;
	rect.height = INTERNET_PARAMS_ADD_BUTTON_HEIGHT;
	MoveWindowDialogUnits(hwnd, addPlayerButton, rect);
	if(removePlayerButton != NULL) {
		rect.x = INTERNET_PARAMS_BUTTONS_LEFT + INTERNET_PARAMS_ADD_BUTTON_WIDTH +
			INTERNET_PARAMS_ACTION_GAP;
		rect.y = addButtonY;
		rect.width = INTERNET_PARAMS_ADD_BUTTON_WIDTH;
		rect.height = INTERNET_PARAMS_ADD_BUTTON_HEIGHT;
		MoveWindowDialogUnits(hwnd, removePlayerButton, rect);
	}

	for(int i = 1; i < MR_MAX_LOCAL_PLAYER; ++i) {
		const int rowY = rowBaseY + ((i - 1) * INTERNET_PARAMS_ROW_HEIGHT);

		rect.x = 7;
		rect.y = rowY + 3;
		rect.width = 35;
		rect.height = 8;
		MoveWindowDialogUnits(hwnd, playerLabels[i], rect);

		rect.x = 45;
		rect.y = rowY + 1;
		rect.width = 72;
		rect.height = 14;
		MoveWindowDialogUnits(hwnd, nameEdits[i], rect);

		rect.x = 120;
		rect.y = rowY + 1;
		rect.width = 42;
		rect.height = 14;
		MoveWindowDialogUnits(hwnd, controlButtons[i], rect);
	}

	rect.x = INTERNET_PARAMS_BUTTONS_X_OK;
	rect.y = buttonY;
	rect.width = INTERNET_PARAMS_BUTTONS_WIDTH;
	rect.height = INTERNET_PARAMS_BUTTONS_HEIGHT;
	MoveDialogItem(hwnd, IDOK, rect);

	rect.x = INTERNET_PARAMS_BUTTONS_X_CANCEL;
	rect.y = buttonY;
	rect.width = INTERNET_PARAMS_BUTTONS_WIDTH;
	rect.height = INTERNET_PARAMS_BUTTONS_HEIGHT;
	MoveDialogItem(hwnd, IDCANCEL, rect);

	RECT windowRect;
	RECT clientRect;
	RECT dialogPixelRect = { 0, 0, INTERNET_PARAMS_WIDTH, dialogHeight };
	MapDialogRect(hwnd, &dialogPixelRect);
	if(GetWindowRect(hwnd, &windowRect) && GetClientRect(hwnd, &clientRect)) {
		const int nonClientWidth =
			(windowRect.right - windowRect.left) -
			(clientRect.right - clientRect.left);
		const int nonClientHeight =
			(windowRect.bottom - windowRect.top) -
			(clientRect.bottom - clientRect.top);

		SetWindowPos(hwnd, NULL, 0, 0,
			(dialogPixelRect.right - dialogPixelRect.left) + nonClientWidth,
			(dialogPixelRect.bottom - dialogPixelRect.top) + nonClientHeight,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

BOOL SelectRoomDialog::ConfigurePlayerControls(HWND hwnd, int playerIdx)
{
	MR_Config::cfg_controls_t controls = tempControls[playerIdx];
	BOOL changed = FALSE;

	for(int actionIdx = 0; actionIdx < NB_CONTROL_ACTIONS; ++actionIdx) {
		char prompt[128];
		int bindingIndex = GetControlBinding(controls, actionIdx);

		BuildControlPrompt(prompt, sizeof(prompt), playerIdx, actionIdx,
			actionIdx + 1, NB_CONTROL_ACTIONS);
		if(!CaptureControlBinding(hwnd, playerIdx, prompt, &bindingIndex)) {
			break;
		}

		SetControlBinding(controls, actionIdx, bindingIndex);
		changed = TRUE;
	}

	if(changed) {
		tempControls[playerIdx] = controls;
	}
	return changed;
}

/**
 * Display the modal dialog.
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 */
RoomListPtr SelectRoomDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	int result = DialogBoxParamW(hinst,
		MAKEINTRESOURCEW(IDD_INTERNET_PARAMS),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));

	return (result == IDOK) ? GetRoomList() : RoomListPtr();
}

void SelectRoomDialog::HandleLoadFinished(HWND hwnd, result_t result)
{
	switch (result) {
		case RESULT_SUCCESS:
			EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
			ShowWindow(GetDlgItem(hwnd, IDC_MSG_LBL), SW_HIDE);
			PopulateList(GetDlgItem(hwnd, IDC_ROOMLIST));
			break;

		case RESULT_CANCELED:
			EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
			EndDialog(hwnd, IDCANCEL);
			break;

		case RESULT_FAILED:
			DisplayError(hwnd);
			EndDialog(hwnd, IDCANCEL);
			break;
	}

	finished = true;
}

/**
 * Populate and show the room list widget.
 * @param hwnd The window handle of the room list widget.
 */
void SelectRoomDialog::PopulateList(HWND hwnd)
{
	SendMessage(hwnd, LB_RESETCONTENT, 0, 0);
	RoomListPtr roomList = GetRoomList();
	const RoomList::rooms_t rooms = roomList->GetRooms();
	for (RoomList::rooms_t::const_iterator iter = rooms.begin();
		iter != rooms.end(); ++iter)
	{
		SendMessage(hwnd, LB_ADDSTRING, 0,
			(LPARAM)(const char*) ((*iter)->name.c_str()));
	}
	SendMessageW(hwnd, LB_SETCURSEL, 0, 0);

	ShowWindow(hwnd, SW_SHOW);
}

// Dialog callback.
BOOL SelectRoomDialog::DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	BOOL retv = FALSE;
	HWND okButton;

	switch (message) {

		case WM_INITDIALOG:
			SetWindowText(hwnd, "Internet Meeting Room");
			SetDlgItemText(hwnd, IDC_ALIAS, playerName.c_str());
			CreateDynamicControls(hwnd);
			UpdatePlayerRows(hwnd);
			ApplyDialogLayout(hwnd);
			SetDlgItemText(hwnd, IDOK, "OK");
			SetDlgItemText(hwnd, IDCANCEL, "Cancel");
			ShowWindow(GetDlgItem(hwnd, IDC_ROOMLIST), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, IDC_MSG_LBL), SW_SHOW);

			okButton = GetDlgItem(hwnd, IDOK);
			SetFocus(okButton);

			retv = TRUE;

			break;

		case WM_COMMAND:
		{
			const int commandId = LOWORD(wparam);

			if(commandId == IDC_LOCAL_ADD_BUTTON) {
				SyncPlayerNamesFromControls(hwnd);
				if(onlinePartySize < MR_MAX_LOCAL_PLAYER) {
					onlinePartySize++;
					UpdatePlayerRows(hwnd);
					ApplyDialogLayout(hwnd);
				}
				retv = TRUE;
				break;
			}
			else if(commandId == IDC_LOCAL_REMOVE_BUTTON) {
				if(onlinePartySize > 1) {
					SyncPlayerNamesFromControls(hwnd);
					onlinePartySize--;
					UpdatePlayerRows(hwnd);
					ApplyDialogLayout(hwnd);
				}
				retv = TRUE;
				break;
			}
			else if((commandId >= IDC_LOCAL_PLAYER_CONTROLS_BASE) &&
				(commandId < IDC_LOCAL_PLAYER_CONTROLS_BASE + MR_MAX_LOCAL_PLAYER)) {
				SyncPlayerNamesFromControls(hwnd);
				ConfigurePlayerControls(hwnd,
					commandId - IDC_LOCAL_PLAYER_CONTROLS_BASE);
				retv = TRUE;
				break;
			}

			switch (commandId) {
				case IDCANCEL:
					if (finished) {
						EndDialog(hwnd, IDCANCEL);
					}
					else {
						CancelLoad();
						ShowWindow(GetDlgItem(hwnd, IDC_ROOMLIST), SW_HIDE);
						SetDlgItemText(hwnd, IDC_MSG_LBL, "Canceling...");
						ShowWindow(GetDlgItem(hwnd, IDC_MSG_LBL), SW_SHOW);
						EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
						retv = TRUE;
					}
					break;

				case IDOK:
				{
					GetRoomList()->SetSelectedRoom(
						SendDlgItemMessage(hwnd, IDC_ROOMLIST, LB_GETCURSEL, 0, 0));

					char alias[64];
					GetDlgItemText(hwnd, IDC_ALIAS, alias, 64);
					alias[63] = 0;
					playerName = (const char *) alias;
					if(playerName.empty()) {
						MessageBox(hwnd, "Enter an alias for player 1 before joining the IMR.",
							"Internet Meeting Room", MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						SetFocus(GetDlgItem(hwnd, IDC_ALIAS));
						retv = TRUE;
						break;
					}

					SyncPlayerNamesFromControls(hwnd);
					onlinePartyNames[0] = playerName;

					for(int i = 1; i < onlinePartySize; ++i) {
						if(onlinePartyNames[i].empty()) {
							char message[128];
							sprintf(message,
								"Enter a name for local player %d before joining the IMR.",
								i + 1);
							MessageBox(hwnd, message, "Internet Meeting Room",
								MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
							SetFocus(nameEdits[i]);
							retv = TRUE;
							break;
						}
					}
					if(retv) break;

					{
						MR_Config *cfg = MR_Config::GetInstance();
						for(int i = 0; i < MR_MAX_LOCAL_PLAYER; ++i) {
							cfg->controls[i] = tempControls[i];
						}
						cfg->Save();
					}

					EndDialog(hwnd, IDOK);
					break;
				}
			}
			break;
		}
	}

	return retv;
}

