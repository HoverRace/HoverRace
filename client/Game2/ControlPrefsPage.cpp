
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

#include "Controller.h"
#include "GameApp.h"

#include "resource.h"

#include "ControlPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Client;
using namespace HoverRace::Util;
using HoverRace::Client::Control::Controller;

#define MR_APP_CLASS_NAME L"HoverRaceClass"

#define MRM_CONTROL_TIMER   3
#define MRM_CONTROL_POLL    4

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
	const char *mOldKeyName;
	int mStringId;
	MR_InControler mControler;
	int mKeyValue;								  // Key or Joy Control
};

/*
static const ControlKey KeyChoice[] =
{
	{	"Disable",			IDS_DISABLE,	MR_KDB,		0			},	// 0
	{	"Shift",			IDS_SHIFT,		MR_KDB,		VK_SHIFT	},	// 1
	{	"Ctrl",				IDS_CTRL,		MR_KDB,		VK_CONTROL	},	// 2
	{	"Up",				IDS_UP,			MR_KDB,		VK_UP		},	// 3
	{	"Down",				IDS_DOWN,		MR_KDB,		VK_DOWN		},	// 4
	{	"Right",			IDS_RIGHT,		MR_KDB,		VK_RIGHT	},	// 5
	{	"Left",				IDS_LEFT,		MR_KDB,		VK_LEFT		},	// 6
	{	"Insert",			IDS_INS,		MR_KDB,		VK_INSERT	},	// 7
	{	"Delete",			IDS_DEL,		MR_KDB,		VK_DELETE	},	// 8
	{	"Enter",			IDS_ENTER,		MR_KDB,		VK_RETURN	},	// 9
	{	"End",				IDS_END,		MR_KDB,		VK_END		},	// 10
	{	"Tab",				IDS_TAB,		MR_KDB,		VK_TAB		},	// 11
	{	"Joystick Btn1",	IDS_JOY1BTN1,	MR_JOY1,	Btn1		},	// 12
	{	"Joystick Btn2",	IDS_JOY1BTN2,	MR_JOY1,	Btn2		},	// 13
	{	"Joystick Btn3",	IDS_JOY1BTN3,	MR_JOY1,	Btn3		},	// 14
	{	"Joystick Btn4",	IDS_JOY1BTN4,	MR_JOY1,	Btn4		},	// 15
	{	"Joystick Btn5",	IDS_JOY1BTN5,	MR_JOY1,	Btn5		},	// 16
	{	"Joystick Btn6",	IDS_JOY1BTN6,	MR_JOY1,	Btn6		},	// 17
	{	"Joystick Btn7",	IDS_JOY1BTN7,	MR_JOY1,	Btn7		},	// 18
	{	"Joystick Btn8",	IDS_JOY1BTN8,	MR_JOY1,	Btn8		},	// 19
	{	"Joystick Top",		IDS_JOY1TOP,	MR_JOY1,	AxeTop		},	// 20
	{	"Joystick Bottom",	IDS_JOY1DOWN,	MR_JOY1,	AxeBottom	},	// 21
	{	"Joystick Right",	IDS_JOY1RIGHT,	MR_JOY1,	AxeRight	},	// 22
	{	"Joystick Left",	IDS_JOY1LEFT,	MR_JOY1,	AxeLeft		},	// 23
	{	"Joystick2 Btn1",	IDS_JOY2BTN1,	MR_JOY2,	Btn1		},	// 24
	{	"Joystick2 Btn2",	IDS_JOY2BTN2,	MR_JOY2,	Btn2		},	// 25
	{	"Joystick2 Btn3",	IDS_JOY2BTN3,	MR_JOY2,	Btn3		},	// 26
	{	"Joystick2 Btn4",	IDS_JOY2BTN4,	MR_JOY2,	Btn4		},	// 27
	{	"Joystick2 Btn5",	IDS_JOY2BTN5,	MR_JOY2,	Btn5		},	// 28
	{	"Joystick2 Btn6",	IDS_JOY2BTN6,	MR_JOY2,	Btn6		},	// 29
	{	"Joystick2 Btn7",	IDS_JOY2BTN7,	MR_JOY2,	Btn7		},	// 30
	{	"Joystick2 Btn8",	IDS_JOY2BTN8,	MR_JOY2,	Btn8		},	// 31
	{	"Joystick2 Top",	IDS_JOY2TOP,	MR_JOY2,	AxeTop		},	// 32
	{	"Joystick2 Bottom",	IDS_JOY2DOWN,	MR_JOY2,	AxeBottom	},	// 33
	{	"Joystick2 Right",	IDS_JOY2RIGHT,	MR_JOY2,	AxeRight	},	// 34
	{	"Joystick2 Left",	IDS_JOY2LEFT,	MR_JOY2,	AxeLeft		},	// 35
	{	"Joystick3 Btn1",	IDS_JOY3BTN1,	MR_JOY3,	Btn1		},	// 36
	{	"Joystick3 Btn2",	IDS_JOY3BTN2,	MR_JOY3,	Btn2		},	// 37
	{	"Joystick3 Btn3",	IDS_JOY3BTN3,	MR_JOY3,	Btn3		},	// 38
	{	"Joystick3 Btn4",	IDS_JOY3BTN4,	MR_JOY3,	Btn4		},	// 39
	{	"Joystick3 Btn5",	IDS_JOY3BTN5,	MR_JOY3,	Btn5		},	// 40
	{	"Joystick3 Btn6",	IDS_JOY3BTN6,	MR_JOY3,	Btn6		},	// 41
	{	"Joystick3 Btn7",	IDS_JOY3BTN7,	MR_JOY3,	Btn7		},	// 42
	{	"Joystick3 Btn8",	IDS_JOY3BTN8,	MR_JOY3,	Btn8		},	// 43
	{	"Joystick3 Top",	IDS_JOY3TOP,	MR_JOY3,	AxeTop		},	// 44
	{	"Joystick3 Bottom",	IDS_JOY3DOWN,	MR_JOY3,	AxeBottom	},	// 45
	{	"Joystick3 Right",	IDS_JOY3RIGHT,	MR_JOY3,	AxeRight	},	// 46
	{	"Joystick3 Left",	IDS_JOY3LEFT,	MR_JOY3,	AxeLeft		},	// 47
	{	"Joystick4 Btn1",	IDS_JOY4BTN1,	MR_JOY4,	Btn1		},	// 48
	{	"Joystick4 Btn2",	IDS_JOY4BTN2,	MR_JOY4,	Btn2		},	// 49
	{	"Joystick4 Btn3",	IDS_JOY4BTN3,	MR_JOY4,	Btn3		},	// 50
	{	"Joystick4 Btn4",	IDS_JOY4BTN4,	MR_JOY4,	Btn4		},	// 51
	{	"Joystick4 Btn5",	IDS_JOY4BTN5,	MR_JOY4,	Btn5		},	// 52
	{	"Joystick4 Btn6",	IDS_JOY4BTN6,	MR_JOY4,	Btn6		},	// 53
	{	"Joystick4 Btn7",	IDS_JOY4BTN7,	MR_JOY4,	Btn7		},	// 54
	{	"Joystick4 Btn8",	IDS_JOY4BTN8,	MR_JOY4,	Btn8		},	// 55
	{	"Joystick4 Top",	IDS_JOY4TOP,	MR_JOY4,	AxeTop		},	// 56
	{	"Joystick4 Bottom",	IDS_JOY4DOWN,	MR_JOY4,	AxeBottom	},	// 57
	{	"Joystick4 Right",	IDS_JOY4RIGHT,	MR_JOY4,	AxeRight	},	// 58
	{	"Joystick4 Left",	IDS_JOY4LEFT,	MR_JOY4,	AxeLeft		},	// 59
	{	"Space",			IDS_SPACE,		MR_KDB,		VK_SPACE	},  // 60, player 2 only from here
	{	"A Key",			IDS_KEY_A,		MR_KDB,		'A'			},	// 61
	{	"B Key",			IDS_KEY_B,		MR_KDB,		'B'			},	// 62
	{	"C Key",			IDS_KEY_C,		MR_KDB,		'C'			},	// 63
	{	"D Key",			IDS_KEY_D,		MR_KDB,		'D'			},	// 64
	{	"E Key",			IDS_KEY_E,		MR_KDB,		'E'			},	// 65
	{	"F Key",			IDS_KEY_F,		MR_KDB,		'F'			},	// 66
	{	"G Key",			IDS_KEY_G,		MR_KDB,		'G'			},	// 67
	{	"H Key",			IDS_KEY_H,		MR_KDB,		'H'			},	// 68
	{	"I Key",			IDS_KEY_I,		MR_KDB,		'I'			},	// 69
	{	"J Key",			IDS_KEY_J,		MR_KDB,		'J'			},	// 70
	{	"K Key",			IDS_KEY_K,		MR_KDB,		'K'			},	// 71
	{	"L Key",			IDS_KEY_L,		MR_KDB,		'L'			},	// 72
	{	"M Key",			IDS_KEY_M,		MR_KDB,		'M'			},	// 73
	{	"N Key",			IDS_KEY_N,		MR_KDB,		'N'			},	// 74
	{	"O Key",			IDS_KEY_O,		MR_KDB,		'O'			},	// 75
	{	"P Key",			IDS_KEY_P,		MR_KDB,		'P'			},	// 76
	{	"Q Key",			IDS_KEY_Q,		MR_KDB,		'Q'			},	// 77
	{	"R Key",			IDS_KEY_R,		MR_KDB,		'R'			},	// 78
	{	"S Key",			IDS_KEY_S,		MR_KDB,		'S'			},	// 79
	{	"T Key",			IDS_KEY_T,		MR_KDB,		'T'			},	// 80
	{	"U Key",			IDS_KEY_U,		MR_KDB,		'U'			},	// 81
	{	"V Key",			IDS_KEY_V,		MR_KDB,		'V'			},	// 82
	{	"W Key",			IDS_KEY_W,		MR_KDB,		'W'			},	// 83
	{	"X Key",			IDS_KEY_X,		MR_KDB,		'X'			},	// 84
	{	"Y Key",			IDS_KEY_Y,		MR_KDB,		'Y'			},	// 85
	{	"Z Key",			IDS_KEY_Z,		MR_KDB,		'Z'			},	// 86
	{	"Not a Key",		IDS_DISABLE,	MR_KDB,		0			},
	{	"Not a Key",		IDS_DISABLE,	MR_KDB,		0			},
	{	"Not a Key",		IDS_DISABLE,	MR_KDB,		0			},
	{	"Not a Key",		IDS_DISABLE,	MR_KDB,		0			},
};
*/

/*
#define NB_KEY_PLAYER_1   60
#define NB_KEY_PLAYER_2   87
#define NB_KEY_PLAYER_3   87
#define NB_KEY_PLAYER_4   87

BOOL gFirstKDBCall = TRUE;						  // Set to TRUE on each new game
BOOL gFirstKDBResetJoy1 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy2 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy3 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy4 = TRUE;					  // Set to TRUE on each new scan
*/

/*
static BOOL CheckKeyState(int pKeyIndex)
{
	BOOL lReturnValue = FALSE;

	static JOYINFOEX lJoystick1;
	static JOYINFOEX lJoystick2;
	static JOYINFOEX lJoystick3;
	static JOYINFOEX lJoystick4;

	switch (KeyChoice[pKeyIndex].mControler) {
		case MR_KDB:
			lReturnValue = GetAsyncKeyState(KeyChoice[pKeyIndex].mKeyValue);
			break;
		case MR_JOY1:
			if(gFirstKDBResetJoy1) {
				gFirstKDBResetJoy1 = FALSE;

				lJoystick1.dwSize = sizeof(lJoystick1);
				lJoystick1.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				joyGetPosEx(0, &lJoystick1);
			}

			switch (KeyChoice[pKeyIndex].mKeyValue) {
				case AxeTop:
					lReturnValue = (lJoystick1.dwYpos < 16000);
					break;
				case AxeBottom:
					lReturnValue = (lJoystick1.dwYpos > 48000);
					break;
				case AxeLeft:
					lReturnValue = (lJoystick1.dwXpos < 16000);
					break;
				case AxeRight:
					lReturnValue = (lJoystick1.dwXpos > 48000);
					break;
				case Btn1:
					lReturnValue = (lJoystick1.dwButtons & 1);
					break;
				case Btn2:
					lReturnValue = (lJoystick1.dwButtons & 2);
					break;
				case Btn3:
					lReturnValue = (lJoystick1.dwButtons & 4);
					break;
				case Btn4:
					lReturnValue = (lJoystick1.dwButtons & 8);
					break;
				case Btn5:
					lReturnValue = (lJoystick1.dwButtons & 16);
					break;
				case Btn6:
					lReturnValue = (lJoystick1.dwButtons & 32);
					break;
				case Btn7:
					lReturnValue = (lJoystick1.dwButtons & 64);
					break;
				case Btn8:
					lReturnValue = (lJoystick1.dwButtons & 128);
					break;
			}
			break;
		case MR_JOY2:
			if(gFirstKDBResetJoy2) {
				gFirstKDBResetJoy2 = FALSE;

				lJoystick2.dwSize = sizeof(lJoystick2);
				lJoystick2.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				joyGetPosEx(0, &lJoystick2);
			}

			switch (KeyChoice[pKeyIndex].mKeyValue) {
				case AxeTop:
					lReturnValue = (lJoystick2.dwYpos < 16000);
					break;
				case AxeBottom:
					lReturnValue = (lJoystick2.dwYpos > 48000);
					break;
				case AxeLeft:
					lReturnValue = (lJoystick2.dwXpos < 16000);
					break;
				case AxeRight:
					lReturnValue = (lJoystick2.dwXpos > 48000);
					break;
				case Btn1:
					lReturnValue = (lJoystick2.dwButtons & 1);
					break;
				case Btn2:
					lReturnValue = (lJoystick2.dwButtons & 2);
					break;
				case Btn3:
					lReturnValue = (lJoystick2.dwButtons & 4);
					break;
				case Btn4:
					lReturnValue = (lJoystick2.dwButtons & 8);
					break;
				case Btn5:
					lReturnValue = (lJoystick2.dwButtons & 16);
					break;
				case Btn6:
					lReturnValue = (lJoystick2.dwButtons & 32);
					break;
				case Btn7:
					lReturnValue = (lJoystick2.dwButtons & 64);
					break;
				case Btn8:
					lReturnValue = (lJoystick2.dwButtons & 128);
					break;
			}
			break;
		case MR_JOY3:
			if(gFirstKDBResetJoy3) {
				gFirstKDBResetJoy3 = FALSE;

				lJoystick3.dwSize = sizeof(lJoystick3);
				lJoystick3.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				joyGetPosEx(0, &lJoystick3);
			}

			switch (KeyChoice[pKeyIndex].mKeyValue) {
				case AxeTop:
					lReturnValue = (lJoystick3.dwYpos < 16000);
					break;
				case AxeBottom:
					lReturnValue = (lJoystick3.dwYpos > 48000);
					break;
				case AxeLeft:
					lReturnValue = (lJoystick3.dwXpos < 16000);
					break;
				case AxeRight:
					lReturnValue = (lJoystick3.dwXpos > 48000);
					break;
				case Btn1:
					lReturnValue = (lJoystick3.dwButtons & 1);
					break;
				case Btn2:
					lReturnValue = (lJoystick3.dwButtons & 2);
					break;
				case Btn3:
					lReturnValue = (lJoystick3.dwButtons & 4);
					break;
				case Btn4:
					lReturnValue = (lJoystick3.dwButtons & 8);
					break;
				case Btn5:
					lReturnValue = (lJoystick3.dwButtons & 16);
					break;
				case Btn6:
					lReturnValue = (lJoystick3.dwButtons & 32);
					break;
				case Btn7:
					lReturnValue = (lJoystick3.dwButtons & 64);
					break;
				case Btn8:
					lReturnValue = (lJoystick3.dwButtons & 128);
					break;
			}
			break;
		case MR_JOY4:
			if(gFirstKDBResetJoy4) {
				gFirstKDBResetJoy4 = FALSE;

				lJoystick4.dwSize = sizeof(lJoystick4);
				lJoystick4.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				joyGetPosEx(0, &lJoystick4);
			}

			switch (KeyChoice[pKeyIndex].mKeyValue) {
				case AxeTop:
					lReturnValue = (lJoystick4.dwYpos < 16000);
					break;
				case AxeBottom:
					lReturnValue = (lJoystick4.dwYpos > 48000);
					break;
				case AxeLeft:
					lReturnValue = (lJoystick4.dwXpos < 16000);
					break;
				case AxeRight:
					lReturnValue = (lJoystick4.dwXpos > 48000);
					break;
				case Btn1:
					lReturnValue = (lJoystick4.dwButtons & 1);
					break;
				case Btn2:
					lReturnValue = (lJoystick4.dwButtons & 2);
					break;
				case Btn3:
					lReturnValue = (lJoystick4.dwButtons & 4);
					break;
				case Btn4:
					lReturnValue = (lJoystick4.dwButtons & 8);
					break;
				case Btn5:
					lReturnValue = (lJoystick4.dwButtons & 16);
					break;
				case Btn6:
					lReturnValue = (lJoystick4.dwButtons & 32);
					break;
				case Btn7:
					lReturnValue = (lJoystick4.dwButtons & 64);
					break;
				case Btn8:
					lReturnValue = (lJoystick4.dwButtons & 128);
					break;
			}
			break;
	}
	return lReturnValue;
}
*/

ControlPrefsPage::ControlPrefsPage(MR_GameApp *app) :
	SUPER(_("Controls"), IDD_CONTROL),
	app(app)
{
}

ControlPrefsPage::~ControlPrefsPage()
{
}

/**
 * Read the control state for a specific player.
 * @param playerIdx The player index (0 = first player, 1 = second, etc.).
 * @return The control state.
 */
/*
int MR_GameApp::ReadAsyncInputControllerPlayer(int playerIdx)
{
	int retv = 0;

	// hack in for now
	Control::ControlState cur = controller->getState(playerIdx);

	if (cur.motorOn)  retv |= MR_MainCharacter::eMotorOn;
	if (cur.jump)     retv |= MR_MainCharacter::eJump;
	if (cur.brake)    retv |= MR_MainCharacter::eBreakDirection;
	if (cur.fire)     retv |= MR_MainCharacter::eFire;
	if (cur.weapon)   retv |= MR_MainCharacter::eSelectWeapon;
	if (cur.lookBack) retv |= MR_MainCharacter::eLookBack;
	if (cur.right)    retv |= MR_MainCharacter::eRight;
	if (cur.left)     retv |= MR_MainCharacter::eLeft;

	controller->poll();

	return retv;
}
*/

/*
void MR_GameApp::ReadAsyncInputController()
{
	gFirstKDBResetJoy1 = TRUE;
	gFirstKDBResetJoy2 = TRUE;

	if(mCurrentSession != NULL) {
		if(GetForegroundWindow() == mMainWindow)
		{
			static BOOL lFirstCall = TRUE;
			int lControlState1 = 0;
			int lControlState2 = 0;
			int lControlState3 = 0;
			int lControlState4 = 0;

			lControlState1 = ReadAsyncInputControllerPlayer(0);

			// If we're in multiplayer mode we need to check those keys too
			if(mCurrentSession->GetMainCharacter2() != NULL) {
				lControlState2 = ReadAsyncInputControllerPlayer(1);
			}
			if(mCurrentSession->GetMainCharacter3() != NULL) {
				lControlState3 = ReadAsyncInputControllerPlayer(2);
			}
			if(mCurrentSession->GetMainCharacter4() != NULL) {
				lControlState4 = ReadAsyncInputControllerPlayer(3);
			}

			if(lFirstCall)
				lFirstCall = FALSE;
			else
				mCurrentSession->SetControlState(lControlState1, lControlState2, lControlState3, lControlState4);
		}
	}
}
*/

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
