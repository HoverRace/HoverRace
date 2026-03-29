// MR_GameApp.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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

#include "stdafx.h"

#include <sstream>

#include "GameApp.h"
#include "resource.h"
#include "CommonDialog.h"
#include "NetworkSession.h"
#include "TrackSelect.h"
#include "TrackDownloadDialog.h"
#include "../Util/DllObjectFactory.h"
#include "../VideoServices/ColorPalette.h"
#include "../MainCharacter/MainCharacter.h"
#include "../Util/FuzzyLogic.h"
#include "../Util/Profiler.h"
#include "../Util/StrRes.h"
#include "../Util/Config.h"
#include "InternetRoom.h"
#include "Security.h"

#include <vfw.h>

#include <direct.h>

namespace {
	void ApplySteamPersonaName(MR_Config *cfg)
	{
		if(cfg == NULL || cfg->player.nickNameSet) {
			return;
		}

		ISteamFriends *steamFriends = SteamFriends();
		if(steamFriends == NULL) {
			return;
		}

		const char *personaName = steamFriends->GetPersonaName();
		if(personaName != NULL && personaName[0] != '\0') {
			cfg->player.nickName = personaName;
			cfg->player.nickNameSet = true;
		}
	}

	bool ParseTcpGameSummary(std::string &summary, int &nbLap, bool &allowWeapons,
		bool &allowCans, bool &allowMines, unsigned &allowedCraftMask)
	{
		const std::string craftsTag = " crafts ";
		const std::string minesOn = " mines on";
		const std::string minesOff = " mines off";
		const std::string cansOn = " cans on";
		const std::string cansOff = " cans off";
		const std::string withWeapons = " with weapons";
		const std::string noWeapons = " no weapons";

		nbLap = 5;
		allowWeapons = false;
		allowCans = true;
		allowMines = true;
		allowedCraftMask = MR_GetDefaultAllowedCraftMask();

		const std::string::size_type craftsPos = summary.rfind(craftsTag);
		if((craftsPos != std::string::npos) &&
			(craftsPos + craftsTag.length() < summary.length()))
		{
			allowedCraftMask = MR_ParseAllowedCraftMask(
				summary.c_str() + craftsPos + craftsTag.length());
			summary.erase(craftsPos);
		}

		if(summary.length() > minesOn.length() &&
			summary.compare(summary.length() - minesOn.length(), minesOn.length(), minesOn) == 0)
		{
			allowMines = true;
			summary.erase(summary.length() - minesOn.length());
		}
		else if(summary.length() > minesOff.length() &&
			summary.compare(summary.length() - minesOff.length(), minesOff.length(), minesOff) == 0)
		{
			allowMines = false;
			summary.erase(summary.length() - minesOff.length());
		}

		if(summary.length() > cansOn.length() &&
			summary.compare(summary.length() - cansOn.length(), cansOn.length(), cansOn) == 0)
		{
			allowCans = true;
			summary.erase(summary.length() - cansOn.length());
		}
		else if(summary.length() > cansOff.length() &&
			summary.compare(summary.length() - cansOff.length(), cansOff.length(), cansOff) == 0)
		{
			allowCans = false;
			summary.erase(summary.length() - cansOff.length());
		}

		if(summary.length() > withWeapons.length() &&
			summary.compare(summary.length() - withWeapons.length(), withWeapons.length(), withWeapons) == 0)
		{
			allowWeapons = true;
			summary.erase(summary.length() - withWeapons.length());
		}
		else if(summary.length() > noWeapons.length() &&
			summary.compare(summary.length() - noWeapons.length(), noWeapons.length(), noWeapons) == 0)
		{
			allowWeapons = false;
			summary.erase(summary.length() - noWeapons.length());
		}

		const std::string::size_type lapsPos = summary.rfind(" laps");
		const std::string::size_type lapPos = (lapsPos == std::string::npos) ? summary.rfind(" lap") : lapsPos;
		if(lapPos != std::string::npos) {
			const std::string::size_type numberStart = summary.rfind(' ', lapPos - 1);
			if((numberStart != std::string::npos) && (numberStart + 1 < lapPos)) {
				nbLap = atoi(summary.c_str() + numberStart + 1);
				if(nbLap < 1) {
					nbLap = 5;
				}
				summary.erase(numberStart);
				while(!summary.empty() && summary[summary.length() - 1] == ' ') {
					summary.erase(summary.length() - 1);
				}
				return !summary.empty();
			}
		}

		return !summary.empty();
	}

	BOOL GetMonitorRectFromWindowCenter(HWND window, RECT *rect, char *deviceName = NULL)
	{
		RECT windowRect;
		if(!GetWindowRect(window, &windowRect)) {
			return FALSE;
		}

		POINT center;
		center.x = windowRect.left + ((windowRect.right - windowRect.left) / 2);
		center.y = windowRect.top + ((windowRect.bottom - windowRect.top) / 2);

		HMONITOR monitor = MonitorFromPoint(center, MONITOR_DEFAULTTONEAREST);
		if(monitor == NULL) {
			return FALSE;
		}

		MONITORINFOEX monitorInfo;
		memset(&monitorInfo, 0, sizeof(monitorInfo));
		monitorInfo.cbSize = sizeof(monitorInfo);
		if(!GetMonitorInfo(monitor, &monitorInfo)) {
			return FALSE;
		}

		*rect = monitorInfo.rcMonitor;
		if(deviceName != NULL) {
			strcpy(deviceName, monitorInfo.szDevice);
		}
		return TRUE;
	}

	BOOL GetMonitorRectForDevice(const char *deviceName, RECT *rect)
	{
		if(deviceName == NULL || *deviceName == '\0') {
			return FALSE;
		}

		DEVMODE devMode;
		memset(&devMode, 0, sizeof(devMode));
		devMode.dmSize = sizeof(devMode);

		if(EnumDisplaySettingsEx(deviceName, ENUM_CURRENT_SETTINGS, &devMode, 0)) {
			rect->left = devMode.dmPosition.x;
			rect->top = devMode.dmPosition.y;
			rect->right = rect->left + static_cast<LONG>(devMode.dmPelsWidth);
			rect->bottom = rect->top + static_cast<LONG>(devMode.dmPelsHeight);
			return TRUE;
		}

		return FALSE;
	}

	BOOL GetPrimaryMonitorRectImpl(RECT *rect, char *deviceName = NULL)
	{
		POINT origin = { 0, 0 };
		HMONITOR monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
		if(monitor == NULL) {
			return FALSE;
		}

		MONITORINFOEX monitorInfo;
		memset(&monitorInfo, 0, sizeof(monitorInfo));
		monitorInfo.cbSize = sizeof(monitorInfo);
		if(!GetMonitorInfo(monitor, &monitorInfo)) {
			return FALSE;
		}

		*rect = monitorInfo.rcMonitor;
		if(deviceName != NULL) {
			strcpy(deviceName, monitorInfo.szDevice);
		}
		return TRUE;
	}
}

// If MR_AVI_CAPTURE is defined
// #define MR_AVI_CAPTUREh

/*
#ifdef MR_AVI_CAPTURE
#include <Vfw.h>

const gCaptureFrameRate = 20; // nb frames sec (between 8-24 is good)

PAVIFILE         gCaptureFile           = NULL;
PAVISTREAM       gCaptureStream         = NULL;
int              gCaptureFrameNo        = 0;

void InitCapture( const char* pFileName, MR_VideoBuffer* pVideoBuffer );
void CloseCapture();
void CaptureScreen( MR_VideoBuffer* pVideoBuffer );

#endif
*/

#define MR_APP_CLASS_NAME "HoverRaceClass"

#define MRM_RETURN2WINDOWMODE  1
#define MRM_EXIT_MENU_LOOP     2
#define MRM_VALIDATE_DESKTOP_FULLSCREEN 3
#define MRM_APPLY_ADAPTIVE_RENDER_SCALE (WM_APP + 101)

static const int gAdaptiveRenderScaleDownStep = 10;
static const int gAdaptiveRenderScaleUpStep = 2;
static const DWORD gAdaptiveRenderScaleDownThresholdMs = 22;
static const DWORD gAdaptiveRenderScaleDownContinueThresholdMs = 15;
static const DWORD gAdaptiveRenderScaleDownFastThresholdMs = 20;
static const DWORD gAdaptiveRenderScaleUpThresholdMs = 12;
static const DWORD gAdaptiveRenderScaleCooldownMs = 1000;
static const DWORD gAdaptiveRenderScaleDownFastCooldownMs = 200;
static const int gAdaptiveRenderScaleSampleFrames = 10;
static const int gAdaptiveRenderScaleUpSampleWindows = 5;
static const DWORD gAdaptiveRenderScaleRetryWindowMs = 15000;

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

#define NB_KEY_PLAYER_1   60
#define NB_KEY_PLAYER_2   87
#define NB_KEY_PLAYER_3   87
#define NB_KEY_PLAYER_4   87

BOOL gFirstKDBCall = TRUE;						  // Set to TRUE on each new game

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

struct CachedJoystickState
{
	BOOL loaded[MR_Config::MAX_PLAYERS];
	BOOL available[MR_Config::MAX_PLAYERS];
	JOYINFOEX state[MR_Config::MAX_PLAYERS];
};

struct ControlDialogState
{
	MR_Config::cfg_controls_t controls[MR_Config::MAX_PLAYERS];
};

struct ControlCaptureDialogState
{
	int maxKeyIndex;
	int result;
	BOOL previousStates[sizeof(KeyChoice) / sizeof(KeyChoice[0])];
	char prompt[128];
};

static CachedJoystickState gCachedJoystickState;

static const int gControlButtonIds[MR_Config::MAX_PLAYERS][NB_CONTROL_ACTIONS] =
{
	{ IDC_MOTOR_ON1, IDC_BREAK1, IDC_LEFT1, IDC_RIGHT1, IDC_JUMP1, IDC_FIRE1, IDC_SELWEAPON1, IDC_LOOKBACK1 },
	{ IDC_MOTOR_ON2, IDC_BREAK2, IDC_LEFT2, IDC_RIGHT2, IDC_JUMP2, IDC_FIRE2, IDC_SELWEAPON2, IDC_LOOKBACK2 },
	{ IDC_MOTOR_ON3, IDC_BREAK3, IDC_LEFT3, IDC_RIGHT3, IDC_JUMP3, IDC_FIRE3, IDC_SELWEAPON3, IDC_LOOKBACK3 },
	{ IDC_MOTOR_ON4, IDC_BREAK4, IDC_LEFT4, IDC_RIGHT4, IDC_JUMP4, IDC_FIRE4, IDC_SELWEAPON4, IDC_LOOKBACK4 }
};

static const int gSetPlayerButtonIds[MR_Config::MAX_PLAYERS] =
{
	IDC_SET_PLAYER1,
	IDC_SET_PLAYER2,
	IDC_SET_PLAYER3,
	IDC_SET_PLAYER4
};

static const int gResetPlayerButtonIds[MR_Config::MAX_PLAYERS] =
{
	IDC_RESET_PLAYER1,
	IDC_RESET_PLAYER2,
	IDC_RESET_PLAYER3,
	IDC_RESET_PLAYER4
};

static const char *gControlActionNames[NB_CONTROL_ACTIONS] =
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

static const UINT_PTR CONTROL_CAPTURE_TIMER_ID = 1;
static const UINT CONTROL_CAPTURE_TIMER_INTERVAL_MS = 50;

static int GetPlayerKeyCount(int playerIdx)
{
	switch(playerIdx) {
		case 0:
			return NB_KEY_PLAYER_1;
		case 1:
			return NB_KEY_PLAYER_2;
		case 2:
			return NB_KEY_PLAYER_3;
		case 3:
		default:
			return NB_KEY_PLAYER_4;
	}
}

static int ClampKeyChoiceIndex(int playerIdx, int keyIndex)
{
	const int maxIndex = GetPlayerKeyCount(playerIdx) - 1;
	if((keyIndex < 0) || (keyIndex > maxIndex)) {
		return 0;
	}
	return keyIndex;
}

static int GetControlBinding(const MR_Config::cfg_controls_t &controls, int actionIdx)
{
	switch(actionIdx) {
		case CtlMotorOn:
			return controls.motorOn;
		case CtlBrake:
			return controls.brake;
		case CtlLeft:
			return controls.left;
		case CtlRight:
			return controls.right;
		case CtlJump:
			return controls.jump;
		case CtlFire:
			return controls.fire;
		case CtlWeapon:
			return controls.weapon;
		case CtlLookBack:
		default:
			return controls.lookBack;
	}
}

static void SetControlBinding(MR_Config::cfg_controls_t &controls, int actionIdx, int keyIndex)
{
	switch(actionIdx) {
		case CtlMotorOn:
			controls.motorOn = keyIndex;
			break;
		case CtlBrake:
			controls.brake = keyIndex;
			break;
		case CtlLeft:
			controls.left = keyIndex;
			break;
		case CtlRight:
			controls.right = keyIndex;
			break;
		case CtlJump:
			controls.jump = keyIndex;
			break;
		case CtlFire:
			controls.fire = keyIndex;
			break;
		case CtlWeapon:
			controls.weapon = keyIndex;
			break;
		case CtlLookBack:
			controls.lookBack = keyIndex;
			break;
	}
}

static void LoadKeyChoiceText(int keyIndex, char *buffer, int bufferLen)
{
	if((keyIndex < 0) || (keyIndex >= static_cast<int>(sizeof(KeyChoice) / sizeof(KeyChoice[0])))) {
		keyIndex = 0;
	}

	if(LoadString(NULL, KeyChoice[keyIndex].mStringId, buffer, bufferLen) == 0) {
		lstrcpyn(buffer, KeyChoice[keyIndex].mOldKeyName, bufferLen);
	}
}

static void UpdateControlButtonText(HWND pWindow, int playerIdx, int actionIdx, int keyIndex)
{
	char buffer[64];

	LoadKeyChoiceText(ClampKeyChoiceIndex(playerIdx, keyIndex), buffer, sizeof(buffer));
	SetDlgItemText(pWindow, gControlButtonIds[playerIdx][actionIdx], buffer);
}

static void RefreshControlButtons(HWND pWindow, const ControlDialogState *state)
{
	for(int playerIdx = 0; playerIdx < MR_Config::MAX_PLAYERS; ++playerIdx) {
		for(int actionIdx = 0; actionIdx < NB_CONTROL_ACTIONS; ++actionIdx) {
			UpdateControlButtonText(pWindow, playerIdx, actionIdx,
				GetControlBinding(state->controls[playerIdx], actionIdx));
		}
	}
}

static void ResetCachedJoystickState()
{
	memset(&gCachedJoystickState, 0, sizeof(gCachedJoystickState));
}

static int GetJoystickIndex(MR_InControler controller)
{
	switch(controller) {
		case MR_JOY1:
			return 0;
		case MR_JOY2:
			return 1;
		case MR_JOY3:
			return 2;
		case MR_JOY4:
			return 3;
		default:
			return -1;
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
		case AxeTop:
			return (joystick.dwYpos < 16000);
		case AxeBottom:
			return (joystick.dwYpos > 48000);
		case AxeLeft:
			return (joystick.dwXpos < 16000);
		case AxeRight:
			return (joystick.dwXpos > 48000);
		case Btn1:
			return ((joystick.dwButtons & 1) != 0);
		case Btn2:
			return ((joystick.dwButtons & 2) != 0);
		case Btn3:
			return ((joystick.dwButtons & 4) != 0);
		case Btn4:
			return ((joystick.dwButtons & 8) != 0);
		case Btn5:
			return ((joystick.dwButtons & 16) != 0);
		case Btn6:
			return ((joystick.dwButtons & 32) != 0);
		case Btn7:
			return ((joystick.dwButtons & 64) != 0);
		case Btn8:
			return ((joystick.dwButtons & 128) != 0);
		default:
			return FALSE;
	}
}

static BOOL TryReadKeyChoiceState(int keyIndex, BOOL useCachedJoystickState, BOOL *pressed)
{
	if((pressed == NULL) || (keyIndex < 0) ||
		(keyIndex >= static_cast<int>(sizeof(KeyChoice) / sizeof(KeyChoice[0]))))
	{
		return FALSE;
	}

	*pressed = FALSE;

	switch(KeyChoice[keyIndex].mControler) {
		case MR_KDB:
			*pressed = ((GetAsyncKeyState(KeyChoice[keyIndex].mKeyValue) & 0x8000) != 0);
			return TRUE;

		case MR_JOY1:
		case MR_JOY2:
		case MR_JOY3:
		case MR_JOY4:
		{
			const int joystickIdx = GetJoystickIndex(KeyChoice[keyIndex].mControler);
			JOYINFOEX joystick;
			BOOL available = FALSE;

			if(useCachedJoystickState) {
				if(!gCachedJoystickState.loaded[joystickIdx]) {
					gCachedJoystickState.loaded[joystickIdx] = TRUE;
					gCachedJoystickState.available[joystickIdx] =
						ReadJoystickState(joystickIdx, &gCachedJoystickState.state[joystickIdx]);
				}

				available = gCachedJoystickState.available[joystickIdx];
				if(available) {
					joystick = gCachedJoystickState.state[joystickIdx];
				}
			}
			else {
				available = ReadJoystickState(joystickIdx, &joystick);
			}

			if(available) {
				*pressed = IsJoystickBindingPressed(joystick, KeyChoice[keyIndex].mKeyValue);
			}
			return TRUE;
		}

		default:
			return FALSE;
	}
}

static BOOL CheckKeyState(int pKeyIndex)
{
	BOOL pressed = FALSE;
	TryReadKeyChoiceState(pKeyIndex, TRUE, &pressed);
	return pressed;
}

static BOOL FindControlBindingById(int controlId, int *playerIdx, int *actionIdx)
{
	for(int player = 0; player < MR_Config::MAX_PLAYERS; ++player) {
		for(int action = 0; action < NB_CONTROL_ACTIONS; ++action) {
			if(gControlButtonIds[player][action] == controlId) {
				if(playerIdx != NULL) {
					*playerIdx = player;
				}
				if(actionIdx != NULL) {
					*actionIdx = action;
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}

static int FindPlayerSetupButton(int controlId)
{
	for(int player = 0; player < MR_Config::MAX_PLAYERS; ++player) {
		if(gSetPlayerButtonIds[player] == controlId) {
			return player;
		}
	}

	return -1;
}

static void BuildControlPrompt(char *buffer, int bufferLen, int playerIdx, int actionIdx,
	int step = 0, int totalSteps = 0)
{
	if(step > 0) {
		sprintf(buffer, "Player %d - %s (%d/%d)", playerIdx + 1,
			gControlActionNames[actionIdx], step, totalSteps);
	}
	else {
		sprintf(buffer, "Player %d - %s", playerIdx + 1, gControlActionNames[actionIdx]);
	}
}

static void LoadDefaultControls(MR_Config::cfg_controls_t *controls)
{
	if(controls == NULL) {
		return;
	}

	controls[0].motorOn = 1;
	controls[0].right = 5;
	controls[0].left = 6;
	controls[0].jump = 3;
	controls[0].fire = 2;
	controls[0].brake = 4;
	controls[0].weapon = 11;
	controls[0].lookBack = 10;

	controls[1].motorOn = 66;
	controls[1].right = 64;
	controls[1].left = 61;
	controls[1].jump = 83;
	controls[1].fire = 78;
	controls[1].brake = 79;
	controls[1].weapon = 77;
	controls[1].lookBack = 65;

	memset(&controls[2], 0, sizeof(MR_Config::cfg_controls_t));
	memset(&controls[3], 0, sizeof(MR_Config::cfg_controls_t));
}

static int FindResetPlayerButton(int controlId)
{
	for(int player = 0; player < MR_Config::MAX_PLAYERS; ++player) {
		if(gResetPlayerButtonIds[player] == controlId) {
			return player;
		}
	}

	return -1;
}

static BOOL CALLBACK ControlCaptureDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	ControlCaptureDialogState *state =
		reinterpret_cast<ControlCaptureDialogState *>(GetWindowLongPtr(pWindow, GWLP_USERDATA));

	switch(pMsgId) {
		case WM_INITDIALOG:
		{
			state = reinterpret_cast<ControlCaptureDialogState *>(pLParam);
			SetWindowLongPtr(pWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
			SetDlgItemText(pWindow, IDC_CAPTURE_PROMPT, state->prompt);

			for(int keyIndex = 0; keyIndex <= state->maxKeyIndex; ++keyIndex) {
				TryReadKeyChoiceState(keyIndex, FALSE, &state->previousStates[keyIndex]);
			}

			SetTimer(pWindow, CONTROL_CAPTURE_TIMER_ID, CONTROL_CAPTURE_TIMER_INTERVAL_MS, NULL);
			SetFocus(pWindow);
			return FALSE;
		}

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
					if(state != NULL) {
						state->result = 0;
					}
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
	state.maxKeyIndex = GetPlayerKeyCount(playerIdx) - 1;
	state.result = ClampKeyChoiceIndex(playerIdx, *bindingIndex);
	lstrcpyn(state.prompt, prompt, sizeof(state.prompt));

	if(DialogBoxParam(instance, MAKEINTRESOURCE(IDD_CONTROL_CAPTURE), pWindow,
		ControlCaptureDialogFunc, reinterpret_cast<LPARAM>(&state)) == IDOK)
	{
		*bindingIndex = ClampKeyChoiceIndex(playerIdx, state.result);
		return TRUE;
	}

	return FALSE;
}

MR_GameApp *MR_GameApp::This;

// Local prototypes
static HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE FAR * lphPalette);
static HPALETTE CreateDIBPalette(LPBITMAPINFO lpbmi, LPINT lpiNumColors);

// class MR_GameThread

unsigned long MR_GameThread::Loop(LPVOID pThread)
{
	MR_GameThread *lThis = (MR_GameThread *) pThread;
	MR_Config *cfg = MR_Config::GetInstance();

	const int sleepLength = cfg->misc.mainLoopSleepLength;
   
	while(true) {
		// Sleep(sleepLength);

		lThis->mNetInterface.CheckP2PAvailability();

		EnterCriticalSection(&lThis->mMutex);

		if(lThis->mTerminate)
			break;

		ASSERT(lThis->mGameApp->mCurrentSession != NULL);

		// Game processing
		MR_SAMPLE_START(ReadInputs, "ReadInputs");
		lThis->mGameApp->ReadAsyncInputController();
		MR_SAMPLE_END(ReadInputs);
		MR_SAMPLE_START(Process, "Process");

		BOOL lRefreshView = FALSE;

#ifdef MR_AVI_CAPTURE
		lRefreshView = lThis->mGameApp->mCurrentSession->Process(1000 / gCaptureFrameRate);
#else
		lRefreshView = lThis->mGameApp->mCurrentSession->Process();
#endif

		MR_SAMPLE_END(Process);

		// Always render — decouple rendering from physics simulation timestep.
		// Physics still ticks in 15ms slices, but rendering runs at display rate.
		{
			MR_SAMPLE_START(Refresh, "Refresh");
			lThis->mGameApp->RefreshView();
			lThis->mGameApp->mNbFrames++;
			MR_SAMPLE_END(Refresh);
		}

		MR_PRINT_STATS(10);						  // Print and reset profiling statistics every 5 seconds

		LeaveCriticalSection(&lThis->mMutex);
	}

	return 0;
}

MR_GameThread::MR_GameThread(MR_GameApp * pApp)
{
	mGameApp = pApp;
	InitializeCriticalSection(&mMutex);
	mThread = NULL;
	mTerminate = FALSE;
	mPauseLevel = 0;
}

MR_GameThread::~MR_GameThread()
{
	DeleteCriticalSection(&mMutex);
}

MR_GameThread *MR_GameThread::New(MR_GameApp * pApp)
{
	unsigned long lDummyInt;
	MR_GameThread *lReturnValue = new MR_GameThread(pApp);

	lReturnValue->mThread = CreateThread(NULL, 0, Loop, lReturnValue, 0, &lDummyInt);

	if(lReturnValue->mThread == NULL) {
		delete lReturnValue;
		lReturnValue = NULL;
	}
	return lReturnValue;
}

void MR_GameThread::Kill()
{
	mTerminate = TRUE;

	while(mPauseLevel > 0)
		Restart();

	WaitForSingleObject(mThread, 3000);
	delete this;
}

void MR_GameThread::Pause()
{
	if(mPauseLevel == 0)
		EnterCriticalSection(&mMutex);
	mPauseLevel++;
}

void MR_GameThread::Restart()
{
	if(mPauseLevel > 0) {
		mPauseLevel--;
		if(mPauseLevel == 0)
			LeaveCriticalSection(&mMutex);
	}
}

MR_GameApp::MR_GameApp(HINSTANCE pInstance)
{
	This = this;
	mInstance = pInstance;
	mMainWindow = NULL;
	mBadVideoModeDlg = NULL;
	mMovieWnd = NULL;
	mWindowedMenu = NULL;
	mDesktopFullscreenDevice[0] = '\0';
	mAccelerators = NULL;
	mVideoBuffer = NULL;
	mObserver1 = NULL;
	mObserver2 = NULL;
	mObserver3 = NULL;
	mObserver4 = NULL;
	mCurrentSession = NULL;
	mGameThread = NULL;

	safeMode = false;
	allowMultipleInstances = false;
	mDesktopFullscreen = false;
	mInResizeLoop = false;
	mInMenuLoop = false;
	SetRectEmpty(&mWindowedRect);
	mWindowedStyle = 0;
	mWindowedExStyle = 0;
	mWindowedShowCmd = SW_SHOWNORMAL;
	mAdaptiveRenderScalePercent = 100;
	mAppliedRenderScalePercent = 100;
	mAdaptiveRenderScaleMaxPercent = 100;
	mAdaptiveRenderScaleUpBasePercent = 0;
	mAdaptiveRenderScaleDownChainCount = 0;
	mAdaptiveRenderScaleLastChangeTick = 0;
	mAdaptiveRenderScaleLastIncreaseTick = 0;
	mAdaptiveRenderScaleAccumulatedMs = 0;
	mAdaptiveRenderScaleSampleCount = 0;
	mAdaptiveRenderScaleGoodSampleCount = 0;
	memset(mAdaptiveRenderScaleCacheWidth, 0, sizeof(mAdaptiveRenderScaleCacheWidth));
	memset(mAdaptiveRenderScaleCacheHeight, 0, sizeof(mAdaptiveRenderScaleCacheHeight));
	memset(mAdaptiveRenderScaleCachePercent, 0, sizeof(mAdaptiveRenderScaleCachePercent));
	mAdaptiveRenderScaleCacheNext = 0;
	mRenderPerfAccumulatedFrameMs = 0;
	mRenderPerfAccumulatedCpuMs = 0;
	mRenderPerfAccumulatedPresentMs = 0;
	mRenderPerfMaxFrameMs = 0;
	mRenderPerfMaxCpuMs = 0;
	mRenderPerfMaxPresentMs = 0;
	mRenderPerfSampleCount = 0;

	mCurrentMode = e3DView;

	mClrScrTodo = 2;
	mLastMouseMoveTick = 0;
	mCursorVisible = TRUE;

	mPaletteChangeAllowed = TRUE;

	char exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH - 1);

	// Change the working directory to the app's directory.
	char *appPath = strdup(exePath);
	char *appDiv = strrchr(appPath, '\\');
	*appDiv = '\0';
	chdir(appPath);
	free(appPath);

	// Load our own version info so we can pass it along to the config.
	long verMajor = 0;
	long verMinor = 0;
	long verPatch = 0;
	long verBuild = 0;
	DWORD dummyHandle;
	DWORD verInfoSize = GetFileVersionInfoSize(exePath, &dummyHandle);
	void *verInfo = malloc(verInfoSize);
	if (GetFileVersionInfo(exePath, 0, verInfoSize, verInfo)) {
		UINT outSize;
		void *outPtr;
		if (VerQueryValue(verInfo, "\\", (LPVOID*)&outPtr, &outSize)) {
			VS_FIXEDFILEINFO *fixedInfo = (VS_FIXEDFILEINFO*)outPtr;
			verMajor = (fixedInfo->dwProductVersionMS >> 16) & 0xffff;
			verMinor = fixedInfo->dwProductVersionMS & 0xffff;
			verPatch = (fixedInfo->dwProductVersionLS >> 16) & 0xffff;
			verBuild = fixedInfo->dwProductVersionLS & 0xffff;
		}
	}
	free(verInfo);

	// Format the version.
	if (verMajor == 0 && verMinor == 0 && verPatch == 0 && verBuild == 0) {
		//FIXME: Oh bother, this means the .exe was compiled without
		//       version resources.  What do we do now?
	}
	std::ostringstream verOss;
	verOss << verMajor << '.' << verMinor << '.' << verPatch << '.' << verBuild;
	std::string appVer = verOss.str();

	// Process command-line options.
	ProcessCmdLine(__argc, __argv);

	// Load the configuration, using the default OS-specific path.
	MR_Config *cfg = MR_Config::Init(appVer);
	if (!safeMode) {
		LoadRegistry();
		cfg->Load();
		OutputDebugString("Loaded config.\n");
	}

	mServerHasChanged = FALSE;
}

MR_GameApp::~MR_GameApp()
{
	Clean();
	MR_DllObjectFactory::Clean(FALSE);
	MR_SoundServer::Close();
	delete mVideoBuffer;

	MR_Config::Shutdown();

	SteamAPI_Shutdown();
}

void MR_GameApp::Clean()
{
	if(mGameThread != NULL) {
		mGameThread->Kill();
		mGameThread = NULL;
	}
	delete mCurrentSession;
	mCurrentSession = NULL;

	mObserver1->Delete();
	mObserver2->Delete();
	mObserver3->Delete();
	mObserver4->Delete();

	mObserver1 = NULL;
	mObserver2 = NULL;
	mObserver3 = NULL;
	mObserver4 = NULL;

	MR_DllObjectFactory::Clean(TRUE);

	mClrScrTodo = 2;
	gFirstKDBCall = TRUE;						  // Set to TRUE on each new game
	ResetAdaptiveRenderScale();

}

/**
 * Process command-line options.
 * @param argc The arg count.
 * @param argv The original argument list.
 */
void MR_GameApp::ProcessCmdLine(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];

		if (strcmp("-s", arg) == 0) {
			safeMode = true;
		}
		else if (strcmp("-m", arg) == 0) {
			allowMultipleInstances = true;
		}
	}
}

void MR_GameApp::LoadRegistry()
{
	MR_Config *cfg = MR_Config::GetInstance();

	char lBuffer[80];
	DWORD lBufferSize = sizeof(lBuffer);

	// Now verify in the registry if this information can not be retrieved
	HKEY lProgramKey;

	int lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\HoverRace.com\\HoverRace",
		0,
		KEY_EXECUTE,
		&lProgramKey);

	if(lError == ERROR_SUCCESS) { // opened key successfully
		MR_UInt8 lControlBuffer[32];
		DWORD lControlBufferSize = sizeof(lControlBuffer);

		if(RegQueryValueEx(lProgramKey, "Control", 0, NULL, lControlBuffer, &lControlBufferSize) == ERROR_SUCCESS) {
			for (int p = 0, i = 0; p < MR_Config::MAX_PLAYERS; ++p) {
				MR_Config::cfg_controls_t &ctl = cfg->controls[p];
				ctl.motorOn = lControlBuffer[i++];
				ctl.right = lControlBuffer[i++];
				ctl.left = lControlBuffer[i++];
				ctl.jump = lControlBuffer[i++];
				ctl.fire = lControlBuffer[i++];
				ctl.brake = lControlBuffer[i++];
				ctl.weapon = lControlBuffer[i++];
				ctl.lookBack = lControlBuffer[i++];
			}
		}

		double lVideoSetting[3];
		DWORD lVideoSettingSize = sizeof(lVideoSetting);

		if(RegQueryValueEx(lProgramKey, "VideoColors", 0, NULL, (MR_UInt8 *) lVideoSetting, &lVideoSettingSize) == ERROR_SUCCESS) {
			cfg->video.gamma = lVideoSetting[0];
			cfg->video.contrast = lVideoSetting[1];
			cfg->video.brightness = lVideoSetting[2];
		}

		lBufferSize = sizeof(lBuffer);
		if(RegQueryValueEx(lProgramKey, "Alias", 0, NULL, (MR_UInt8 *) lBuffer, &lBufferSize) == ERROR_SUCCESS) {
			cfg->player.nickName = lBuffer;
			cfg->player.nickNameSet = !cfg->player.nickName.empty();
		}

		BOOL lBool;

		lBufferSize = sizeof(lBool);
		if(RegQueryValueEx(lProgramKey, "DisplayFirstScreen", 0, NULL, (MR_UInt8 *) &lBool, &lBufferSize) == ERROR_SUCCESS)
			cfg->misc.displayFirstScreen = !lBool;

		lBufferSize = sizeof(lBool);
		if(RegQueryValueEx(lProgramKey, "IntroMovie", 0, NULL, (MR_UInt8 *) &lBool, &lBufferSize) == ERROR_SUCCESS)
			cfg->misc.introMovie = !lBool;

		// Get the address of the server (we need a larger buffer)
		{
			char  lServerBuffer[500];
			DWORD lServerBufferSize = sizeof(lServerBuffer);
			if(RegQueryValueEx(lProgramKey, "MainServer", 0, NULL, (MR_UInt8 *) lServerBuffer, &lServerBufferSize) == ERROR_SUCCESS) {
				cfg->net.mainServer = lServerBuffer;
			}
		}
	}
}

void MR_GameApp::SaveRegistry()
{
	MR_Config::GetInstance()->Save();
}

BOOL MR_GameApp::IsGameRunning()
{
	BOOL lReturnValue = FALSE;

	if(mCurrentSession != NULL) {
		MR_MainCharacter *lPlayer = mCurrentSession->GetMainCharacter();

		if(lPlayer != NULL) {
			if(!(lPlayer->GetTotalLap() <= lPlayer->GetLap())) {
				lPlayer = mCurrentSession->GetMainCharacter2();

				if(lPlayer == NULL) {
					lReturnValue = TRUE;
				}

				lPlayer = mCurrentSession->GetMainCharacter3();

				if(lPlayer == NULL) {
					lReturnValue = TRUE;
				}

				lPlayer = mCurrentSession->GetMainCharacter4();

				if(lPlayer == NULL) {
					lReturnValue = TRUE;
				}
				else {
					if(lPlayer->GetTotalLap() <= lPlayer->GetLap()) {
						lReturnValue = TRUE;
					}
				}
			}
		}
	}
	return lReturnValue;
}

int MR_GameApp::AskUserToAbortGame()
{
	int lReturnValue = IDOK;

	if(IsGameRunning()) {
		lReturnValue = MessageBox(mMainWindow, MR_LoadString(IDS_ABORT_GAME), MR_LoadString(IDS_GAME_NAME), MB_OKCANCEL | MB_ICONWARNING);

		if(lReturnValue == 0) {
			lReturnValue = IDOK;
		}
	}
	return lReturnValue;
}

void MR_GameApp::DisplayHelp()
{

	// first return to window mode
	SetVideoMode(0, 0);

	unsigned int lReturnCode = (int) ShellExecute(mMainWindow,
		NULL,
		"..\\Help\\Help.doc",					  // "..\\Help\\Index.html",
		NULL,
		NULL,
		SW_SHOWNORMAL);

	if(lReturnCode <= 32) {

		MessageBox(mMainWindow, MR_LoadString(IDS_HELP_REQ), MR_LoadString(IDS_ERROR), MB_ICONERROR | MB_APPLMODAL | MB_OK);

		/*
		   MessageBox( mMainWindow,
		   "To be able to display Hover Race Help page\n"
		   "you must have a WWW browser installed on\n"
		   "your system",
		   "Hover Race Help Error",
		   MB_ICONERROR|MB_APPLMODAL|MB_OK );
		 */

	}

}

void MR_GameApp::DisplayBetaZone()
{

	// first return to window mode
	SetVideoMode(0, 0);

	LoadURLShortcut(mMainWindow, MR_LoadString(IDS_BETAZONE));
}

void MR_GameApp::DisplaySite()
{
	// first return to window mode
	SetVideoMode(0, 0);
	LoadURLShortcut(mMainWindow, MR_LoadString(IDS_WEBSITE));
}

void MR_GameApp::DisplayAbout()
{
	SetVideoMode(0, 0);
	int retv;
	if ((retv = DialogBox(mInstance, MAKEINTRESOURCE(IDD_ABOUT), mMainWindow, AboutDlgFunc)) <= 0) {
		DWORD err = GetLastError();
		LPVOID errMsg;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errMsg,
			0, NULL );
		MessageBox(mMainWindow, (const char*)errMsg, "AIEEE", MB_ICONERROR | MB_APPLMODAL | MB_OK);
		LocalFree(errMsg);
	}
	AssignPalette();
}

int MR_GameApp::MainLoop()
{
	MSG lMessage;
	BOOL lEofGame = FALSE;

	while(!lEofGame) {
		WaitMessage();

		// Documentation can't be found on any of the *Message() methods.
		// They must be built-in or inherited.
		while(PeekMessage(&lMessage, NULL, 0, 0, PM_NOREMOVE)) {
			if(GetMessage(&lMessage, NULL, 0, 0)) {
				if(!TranslateAccelerator(mMainWindow, mAccelerators, &lMessage)) {
					TranslateMessage(&lMessage);
					DispatchMessage(&lMessage);
				}
			}
			else {
				lEofGame = TRUE;
			}
		}
	}
	Clean();

	/*
	   // Idle section
	   if( !lEofGame )
	   {
	   // WARNING: This main loop section will have to be tuned

	   if( mCurrentSession == NULL )
	   {
	   WaitMessage();
	   }
	   else
	   {

	   // Game processing
	   MR_SAMPLE_START( ReadInputs, "ReadInputs" );

	   ReadAsyncInputController();

	   MR_SAMPLE_END( ReadInputs );

	   MR_SAMPLE_START( Process, "Process" );

	   #ifdef MR_AVI_CAPTURE
	   mCurrentSession->Process( 1000/gCaptureFrameRate );
	   #else
	   mCurrentSession->Process( );
	   #endif

	   MR_SAMPLE_END( Process );

	   MR_SAMPLE_START( Refresh, "Refresh" );
	   RefreshView();
	   mNbFrames++;
	   MR_SAMPLE_END( Refresh );

	   MR_PRINT_STATS( 10 ); // Print and reset profiling statistics every 5 seconds

	   }
	   }
	   }
	 */

	return lMessage.wParam;
}

BOOL MR_GameApp::IsFirstInstance() const
{
	if (allowMultipleInstances) return TRUE;

	HWND lPrevAppWnd = FindWindow(MR_APP_CLASS_NAME, NULL);
	HWND lChildAppWnd = NULL;

	// Determine if another window with our class name exists...
	if(lPrevAppWnd != NULL) {
		// if so, does it have any popups?
		lChildAppWnd = GetLastActivePopup(lPrevAppWnd);

		// Bring the main window to the top.
		SetForegroundWindow(lPrevAppWnd);
		// If iconic, restore the main window.
		ShowWindow(lPrevAppWnd, SW_RESTORE);

		// If there was an active popup, bring it along too!
		if((lChildAppWnd != NULL) && (lPrevAppWnd != lChildAppWnd))
			SetForegroundWindow(lChildAppWnd);
	}
	return (lPrevAppWnd == NULL);
}

BOOL MR_GameApp::InitApplication()
{
	BOOL lReturnValue = TRUE;

	WNDCLASS lWinClass;

	lWinClass.style = CS_DBLCLKS;
	lWinClass.lpfnWndProc = DispatchFunc;
	lWinClass.cbClsExtra = 0;
	lWinClass.cbWndExtra = 0;
	lWinClass.hInstance = mInstance;
	lWinClass.hIcon = LoadIcon(mInstance, MAKEINTRESOURCE(IDI_HOVER_ICON));
	lWinClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	lWinClass.hbrBackground = (HBRUSH) COLOR_APPWORKSPACE + 1;
	lWinClass.lpszMenuName = MAKEINTRESOURCE(FIREBALL_MAIN_MENU);
	lWinClass.lpszClassName = MR_APP_CLASS_NAME;

	lReturnValue = RegisterClass(&lWinClass);

	ASSERT(lReturnValue);

	return lReturnValue;
}

BOOL MR_GameApp::CreateMainWindow()
{
	BOOL lReturnValue = TRUE;
	MR_Config *cfg = MR_Config::GetInstance();

	int windowPosX = cfg->video.windowPosX;
	int windowPosY = cfg->video.windowPosY;
	int windowSizeX = cfg->video.windowSizeX;
	int windowSizeY = cfg->video.windowSizeY;

	if(!cfg->video.windowMonitor.empty()) {
		RECT initialRect;
		ResolveInitialWindowRect(&initialRect);
		windowPosX = initialRect.left;
		windowPosY = initialRect.top;
		windowSizeX = initialRect.right - initialRect.left;
		windowSizeY = initialRect.bottom - initialRect.top;
	}

	// attempt to make the main window
	// WS_OVERLAPPEDWINDOW already includes WS_MAXIMIZEBOX — keep it enabled
	// so the user can click the maximize button for a regular maximized
	// window (keeps title bar + taskbar). F11 remains the toggle for
	// borderless fullscreen (no title bar, covers entire monitor).
	mMainWindow = CreateWindowEx(WS_EX_APPWINDOW, MR_APP_CLASS_NAME, MR_LoadString(IDS_CAPTION), WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_EX_CLIENTEDGE, windowPosX, windowPosY, windowSizeX, windowSizeY, NULL, NULL, mInstance, NULL);

	if(mMainWindow == NULL)
		lReturnValue = FALSE;					  // making of window failed
	else {
		if(cfg->video.windowMaximized) {
			ShowWindow(mMainWindow, SW_SHOWMAXIMIZED);
		}
		InvalidateRect(mMainWindow, NULL, FALSE);
		UpdateWindow(mMainWindow);
		SetFocus(mMainWindow);
	}

	RefreshTitleBar();
	UpdateMenuItems();

	return lReturnValue;
}

void MR_GameApp::RefreshTitleBar()
{
	// fixed to get rid of registration
	if(mMainWindow != NULL)
		SetWindowText(mMainWindow, MR_LoadString(IDS_CAPTION));
}

void MR_GameApp::ResetAdaptiveRenderScale()
{
	const BOOL lNeedApply = (mAppliedRenderScalePercent != 100);
	mAdaptiveRenderScalePercent = 100;
	mAppliedRenderScalePercent = 100;
	mAdaptiveRenderScaleMaxPercent = 100;
	mAdaptiveRenderScaleUpBasePercent = 0;
	mAdaptiveRenderScaleDownChainCount = 0;
	mAdaptiveRenderScaleLastChangeTick = 0;
	mAdaptiveRenderScaleLastIncreaseTick = 0;
	mAdaptiveRenderScaleAccumulatedMs = 0;
	mAdaptiveRenderScaleSampleCount = 0;
	mAdaptiveRenderScaleGoodSampleCount = 0;

	if(mVideoBuffer != NULL) {
		mVideoBuffer->SetRenderScalePercent(100);
		if(lNeedApply && mVideoBuffer->IsWindowMode() && !mVideoBuffer->IsModeSettingInProgress()) {
			mVideoBuffer->SetVideoMode();
			if(mMainWindow != NULL) {
				PostMessage(mMainWindow, WM_QUERYNEWPALETTE, 0, 0);
			}
		}
	}
}

void MR_GameApp::RememberAdaptiveRenderScaleForSize(int pDisplayXRes, int pDisplayYRes, int pPercent)
{
	int lIndex;

	if(pDisplayXRes <= 0 || pDisplayYRes <= 0) {
		return;
	}

	if(pPercent < 10) {
		pPercent = 10;
	}
	else if(pPercent > 100) {
		pPercent = 100;
	}

	for(lIndex = 0; lIndex < ADAPTIVE_RENDER_SCALE_CACHE_SIZE; lIndex++) {
		if((mAdaptiveRenderScaleCacheWidth[lIndex] == pDisplayXRes) &&
			(mAdaptiveRenderScaleCacheHeight[lIndex] == pDisplayYRes))
		{
			mAdaptiveRenderScaleCachePercent[lIndex] = pPercent;
			return;
		}
	}

	lIndex = mAdaptiveRenderScaleCacheNext;
	mAdaptiveRenderScaleCacheWidth[lIndex] = pDisplayXRes;
	mAdaptiveRenderScaleCacheHeight[lIndex] = pDisplayYRes;
	mAdaptiveRenderScaleCachePercent[lIndex] = pPercent;
	mAdaptiveRenderScaleCacheNext = (mAdaptiveRenderScaleCacheNext + 1) % ADAPTIVE_RENDER_SCALE_CACHE_SIZE;
}

int MR_GameApp::GetCachedAdaptiveRenderScaleForSize(int pDisplayXRes, int pDisplayYRes) const
{
	int lIndex;

	for(lIndex = 0; lIndex < ADAPTIVE_RENDER_SCALE_CACHE_SIZE; lIndex++) {
		if((mAdaptiveRenderScaleCacheWidth[lIndex] == pDisplayXRes) &&
			(mAdaptiveRenderScaleCacheHeight[lIndex] == pDisplayYRes) &&
			(mAdaptiveRenderScaleCachePercent[lIndex] >= 10))
		{
			return mAdaptiveRenderScaleCachePercent[lIndex];
		}
	}

	return 100;
}

void MR_GameApp::ResetAdaptiveRenderScaleForResize(int pPercent)
{
	if(pPercent < 10) {
		pPercent = 10;
	}
	else if(pPercent > 100) {
		pPercent = 100;
	}

	mAdaptiveRenderScalePercent = pPercent;
	mAppliedRenderScalePercent = pPercent;
	mAdaptiveRenderScaleMaxPercent = 100;
	mAdaptiveRenderScaleUpBasePercent = 0;
	mAdaptiveRenderScaleDownChainCount = 0;
	mAdaptiveRenderScaleLastChangeTick = 0;
	mAdaptiveRenderScaleLastIncreaseTick = 0;
	mAdaptiveRenderScaleAccumulatedMs = 0;
	mAdaptiveRenderScaleSampleCount = 0;
	mAdaptiveRenderScaleGoodSampleCount = 0;

	if(mVideoBuffer != NULL) {
		mVideoBuffer->SetRenderScalePercent(pPercent);
	}
}

BOOL MR_GameApp::ShouldUseAdaptiveRenderScale() const
{
	MR_Config *cfg = MR_Config::GetInstance();
	if((cfg == NULL) || !cfg->video.adaptiveRenderScale) {
		return FALSE;
	}
	if(mVideoBuffer == NULL || !mVideoBuffer->IsWindowMode()) {
		return FALSE;
	}
	if(mCurrentSession == NULL) {
		return FALSE;
	}
	return (mCurrentMode == e3DView) || (mCurrentMode == eDebugView);
}

void MR_GameApp::RequestAdaptiveRenderScale(int pPercent)
{
	if(pPercent < 10) {
		pPercent = 10;
	}
	else if(pPercent > 100) {
		pPercent = 100;
	}

	if(mAdaptiveRenderScalePercent == pPercent) {
		return;
	}

	mAdaptiveRenderScalePercent = pPercent;
	if(mMainWindow != NULL) {
		PostMessage(mMainWindow, MRM_APPLY_ADAPTIVE_RENDER_SCALE, (WPARAM) pPercent, 0);
	}
}

void MR_GameApp::ApplyAdaptiveRenderScale(int pPercent)
{
	if(pPercent < 10) {
		pPercent = 10;
	}
	else if(pPercent > 100) {
		pPercent = 100;
	}

	mAdaptiveRenderScalePercent = pPercent;

	if(mVideoBuffer == NULL) {
		mAppliedRenderScalePercent = pPercent;
		return;
	}

	if(mAppliedRenderScalePercent == pPercent) {
		mVideoBuffer->SetRenderScalePercent(pPercent);
		return;
	}

	TRACE("AdaptiveRenderScale %d -> %d\n", mAppliedRenderScalePercent, pPercent);
	mVideoBuffer->SetRenderScalePercent(pPercent);

	if(mVideoBuffer->IsWindowMode() && !mVideoBuffer->IsModeSettingInProgress()) {
		PauseGameThread();
		mClrScrTodo = 2;
		if(!mVideoBuffer->SetVideoMode()) {
			mAdaptiveRenderScalePercent = mAppliedRenderScalePercent;
			mVideoBuffer->SetRenderScalePercent(mAppliedRenderScalePercent);
			RestartGameThread();
			return;
		}
		RestartGameThread();
		if(mMainWindow != NULL) {
			PostMessage(mMainWindow, WM_QUERYNEWPALETTE, 0, 0);
		}
	}

	mAppliedRenderScalePercent = pPercent;
}

void MR_GameApp::UpdateAdaptiveRenderScale(DWORD pFrameMs)
{
	MR_Config *cfg = MR_Config::GetInstance();
	if(cfg == NULL) {
		return;
	}

	if(!ShouldUseAdaptiveRenderScale()) {
		mAdaptiveRenderScaleMaxPercent = 100;
		mAdaptiveRenderScaleUpBasePercent = 0;
		mAdaptiveRenderScaleDownChainCount = 0;
		mAdaptiveRenderScaleLastIncreaseTick = 0;
		mAdaptiveRenderScaleAccumulatedMs = 0;
		mAdaptiveRenderScaleSampleCount = 0;
		mAdaptiveRenderScaleGoodSampleCount = 0;
		if(mAdaptiveRenderScalePercent != 100 || mAppliedRenderScalePercent != 100) {
			RequestAdaptiveRenderScale(100);
		}
		return;
	}

	mAdaptiveRenderScaleAccumulatedMs += pFrameMs;
	mAdaptiveRenderScaleSampleCount++;

	if(mAdaptiveRenderScaleSampleCount < gAdaptiveRenderScaleSampleFrames) {
		return;
	}

	DWORD lAverageFrameMs = mAdaptiveRenderScaleAccumulatedMs / mAdaptiveRenderScaleSampleCount;
	DWORD lNow = GetTickCount();
	int lTargetPercent = mAdaptiveRenderScalePercent;
	const int lMinPercent = max(10, min(100, cfg->video.adaptiveRenderScaleMinPercent));
	const DWORD lDownThresholdMs =
		(mAdaptiveRenderScaleDownChainCount > 0) ?
		gAdaptiveRenderScaleDownContinueThresholdMs : gAdaptiveRenderScaleDownThresholdMs;
	const DWORD lDownFastThresholdMs =
		(mAdaptiveRenderScaleDownChainCount > 0) ?
		gAdaptiveRenderScaleDownContinueThresholdMs : gAdaptiveRenderScaleDownFastThresholdMs;
	const DWORD lCooldownMs =
		(mAdaptiveRenderScaleDownChainCount >= 1 && lAverageFrameMs > lDownFastThresholdMs) ?
		gAdaptiveRenderScaleDownFastCooldownMs : gAdaptiveRenderScaleCooldownMs;
	const BOOL lRecentUpscaleFailure =
		(mAdaptiveRenderScaleUpBasePercent > 0) &&
		(mAdaptiveRenderScalePercent > mAdaptiveRenderScaleUpBasePercent) &&
		((lNow - mAdaptiveRenderScaleLastIncreaseTick) < gAdaptiveRenderScaleRetryWindowMs);

	mAdaptiveRenderScaleAccumulatedMs = 0;
	mAdaptiveRenderScaleSampleCount = 0;

	if((lNow - mAdaptiveRenderScaleLastChangeTick) < lCooldownMs) {
		return;
	}

	if(lAverageFrameMs > lDownThresholdMs && lTargetPercent > lMinPercent) {
		mAdaptiveRenderScaleGoodSampleCount = 0;
		mAdaptiveRenderScaleDownChainCount++;
		if(lRecentUpscaleFailure) {
			mAdaptiveRenderScaleMaxPercent = min(mAdaptiveRenderScaleMaxPercent, mAdaptiveRenderScaleUpBasePercent);
			lTargetPercent = max(lMinPercent, mAdaptiveRenderScaleMaxPercent);
		}
		else {
			lTargetPercent = max(lMinPercent, lTargetPercent - gAdaptiveRenderScaleDownStep);
		}
		mAdaptiveRenderScaleUpBasePercent = 0;
	}
	else if(lAverageFrameMs < gAdaptiveRenderScaleUpThresholdMs && lTargetPercent < mAdaptiveRenderScaleMaxPercent) {
		mAdaptiveRenderScaleGoodSampleCount++;
		mAdaptiveRenderScaleDownChainCount = 0;
		if(mAdaptiveRenderScaleGoodSampleCount >= gAdaptiveRenderScaleUpSampleWindows) {
			mAdaptiveRenderScaleGoodSampleCount = 0;
			if(mAdaptiveRenderScaleUpBasePercent == 0) {
				mAdaptiveRenderScaleUpBasePercent = lTargetPercent;
			}
			lTargetPercent = min(mAdaptiveRenderScaleMaxPercent, lTargetPercent + gAdaptiveRenderScaleUpStep);
		}
	}
	else {
		mAdaptiveRenderScaleGoodSampleCount = 0;
		mAdaptiveRenderScaleDownChainCount = 0;
	}

	if(lTargetPercent != mAdaptiveRenderScalePercent) {
		mAdaptiveRenderScaleGoodSampleCount = 0;
		if(lTargetPercent > mAdaptiveRenderScalePercent) {
			mAdaptiveRenderScaleDownChainCount = 0;
		}
		mAdaptiveRenderScaleLastChangeTick = lNow;
		if(lTargetPercent > mAdaptiveRenderScalePercent) {
			mAdaptiveRenderScaleLastIncreaseTick = lNow;
		}
		TRACE("AdaptiveRenderScale request=%d avgFrameMs=%lu\n", lTargetPercent, lAverageFrameMs);
		RequestAdaptiveRenderScale(lTargetPercent);
	}
}

BOOL MR_GameApp::InitGame()
{
	// Check we're running from Steam and signed in
	if ( SteamAPI_RestartAppIfNecessary( 1862020 ) ) // Replace with your App ID
	{
		return false;
	}

	if(SteamAPI_Init()) {
		if(SteamUser()->BLoggedOn()) {
			mSteamIDLocalUser = SteamUser()->GetSteamID();
		}
	} else {
		return false;
	}

	BOOL lReturnValue = TRUE;
	MR_Config *cfg = MR_Config::GetInstance();
	ApplySteamPersonaName(cfg);

	InitCommonControls();						  // Allow some special and complex controls

	// Display a Flash window
	// TODO

	// Init needed modules
	MR_InitTrigoTables();
	MR_InitFuzzyModule();
	MR_DllObjectFactory::Init();
	MR_MainCharacter::RegisterFactory();

	// Load accelerators
	mAccelerators = LoadAccelerators(mInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	lReturnValue = CreateMainWindow();

	if(lReturnValue) {
		mVideoBuffer = new MR_VideoBuffer(mMainWindow,
			cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
	}

	// attempt to set the video mode
	if(lReturnValue) {
		if(!mVideoBuffer->SetVideoMode()) {		  // try to set the video mode
			BOOL lSwitchTo256 = FALSE;
			if(MessageBox(mMainWindow, MR_LoadString(IDS_MODE_SWITCH_TRY), MR_LoadString(IDS_GAME_NAME), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
				if(mVideoBuffer->TryToSetColorMode(8)) {
					if(mVideoBuffer->SetVideoMode())
						lSwitchTo256 = TRUE;
				}
				if(!lSwitchTo256) {				  // mode switch failed, tell the user
					MessageBox(mMainWindow, MR_LoadString(IDS_CANT_SWITCH_MODE), MR_LoadString(IDS_GAME_NAME), MB_OK);
				}
			}

			if(!lSwitchTo256) {					  // load the "Incompatible Video Mode" dialog
				mBadVideoModeDlg = CreateDialog(mInstance, MAKEINTRESOURCE(IDD_BAD_MODE), mMainWindow, BadModeDialogFunc);
			}
		}
	}

	// play the opening movie
	if(lReturnValue && cfg->misc.introMovie) {
		mMovieWnd = MCIWndCreate(
			mMainWindow, mInstance, 
			WS_CHILD | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, 
			cfg->GetMediaPath("Intro.wmv").c_str());

		MCIWndSetVolume(mMovieWnd, cfg->audio.sfxVolume * 1000);

		// Fill the client area.
		RECT clientRect;
		GetClientRect(mMainWindow, &clientRect);
		MoveWindow(mMovieWnd, 0, 0,
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top,
			TRUE);
		
		MCIWndPlay(mMovieWnd);

	}

	if(lReturnValue)
		OnDisplayChange();

	//if(lReturnValue)
	//{
	// Raise process priority
	// (Tests shows that it is not a good idea. It is not facter and
	//   it cause the animation to be less smooth )
	// SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
	// SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST /*THREAD_PRIORITY_TIME_CRITICAL*/ );
	//}

	// show "click OK to play on the internet" dialog
	if(lReturnValue && cfg->misc.displayFirstScreen) {
		int result = DialogBoxParam(mInstance, MAKEINTRESOURCE(IDD_FIRST_CHOICE), mMainWindow, FirstChoiceDialogFunc, (LPARAM)IDOK);

		if (result == IDOK) {
			SendMessage(mMainWindow, WM_COMMAND, ID_GAME_NETWORK_INTERNET, 0);
		}
	}

	return lReturnValue;
}

void MR_GameApp::RefreshView()
{
	static int lColor = 0;
	DWORD frameStartTick = GetTickCount();
	DWORD cpuEndTick = frameStartTick;
	DWORD presentEndTick = frameStartTick;
	BOOL sampledFrame = FALSE;

	// Game processing
	if(mVideoBuffer != NULL) {
		if(mVideoBuffer->Lock()) {
			if(mCurrentSession != NULL) {
				MR_SimulationTime lTime = mCurrentSession->GetSimulationTime();

				switch (mCurrentMode) {
					case e3DView:
						if(mClrScrTodo > 0) {
							mClrScrTodo--;
							DrawBackground();
						}

						if(mObserver1 != NULL)
							mObserver1->RenderNormalDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter(), lTime, mCurrentSession->GetBackImage());
						if(mObserver2 != NULL)
							mObserver2->RenderNormalDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter2(), lTime, mCurrentSession->GetBackImage());

						if(mObserver3 != NULL)
							mObserver3->RenderNormalDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter3(), lTime, mCurrentSession->GetBackImage());
						if(mObserver4 != NULL)
							mObserver4->RenderNormalDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter4(), lTime, mCurrentSession->GetBackImage());
						break;

					case eDebugView:
						if(mObserver1 != NULL)
							mObserver1->RenderDebugDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter(), lTime, mCurrentSession->GetBackImage());
						if(mObserver2 != NULL)
							mObserver2->RenderDebugDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter2(), lTime, mCurrentSession->GetBackImage());
						if(mObserver3 != NULL)
							mObserver3->RenderDebugDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter3(), lTime, mCurrentSession->GetBackImage());
						if(mObserver4 != NULL)
							mObserver4->RenderDebugDisplay(mVideoBuffer, mCurrentSession, mCurrentSession->GetMainCharacter4(), lTime, mCurrentSession->GetBackImage());
						break;
				}

#ifdef MR_AVI_CAPTURE
				CaptureScreen(mVideoBuffer);
#endif

			} else
			mVideoBuffer->Clear((MR_UInt8) (lColor++));
			cpuEndTick = GetTickCount();
			mVideoBuffer->Unlock();
			presentEndTick = GetTickCount();
			sampledFrame = TRUE;
		}
	}
	if(sampledFrame) {
		AccumulateRenderPerformanceSample(presentEndTick - frameStartTick,
			cpuEndTick - frameStartTick,
			presentEndTick - cpuEndTick);
	}
	// Sound refresh
	if(mCurrentSession != NULL) {
		if(mObserver1 != NULL)
			mObserver1->PlaySounds(mCurrentSession->GetCurrentLevel(), mCurrentSession->GetMainCharacter());
		if(mObserver2 != NULL)
			mObserver2->PlaySounds(mCurrentSession->GetCurrentLevel(), mCurrentSession->GetMainCharacter2());
		if(mObserver3 != NULL)
			mObserver3->PlaySounds(mCurrentSession->GetCurrentLevel(), mCurrentSession->GetMainCharacter3());
		if(mObserver4 != NULL)
			mObserver4->PlaySounds(mCurrentSession->GetCurrentLevel(), mCurrentSession->GetMainCharacter4());

		MR_SoundServer::ApplyContinuousPlay();
	}
}

void MR_GameApp::AccumulateRenderPerformanceSample(DWORD pFrameMs, DWORD pCpuMs, DWORD pPresentMs)
{
	enum { PERF_LOG_SAMPLE_COUNT = 120 };

	mRenderPerfAccumulatedFrameMs += pFrameMs;
	mRenderPerfAccumulatedCpuMs += pCpuMs;
	mRenderPerfAccumulatedPresentMs += pPresentMs;
	if(pFrameMs > mRenderPerfMaxFrameMs) {
		mRenderPerfMaxFrameMs = pFrameMs;
	}
	if(pCpuMs > mRenderPerfMaxCpuMs) {
		mRenderPerfMaxCpuMs = pCpuMs;
	}
	if(pPresentMs > mRenderPerfMaxPresentMs) {
		mRenderPerfMaxPresentMs = pPresentMs;
	}
	mRenderPerfSampleCount++;

	if((mVideoBuffer != NULL) && (mRenderPerfSampleCount >= PERF_LOG_SAMPLE_COUNT)) {
		mVideoBuffer->LogPerformanceSample(
			mRenderPerfAccumulatedFrameMs / mRenderPerfSampleCount,
			mRenderPerfAccumulatedCpuMs / mRenderPerfSampleCount,
			mRenderPerfAccumulatedPresentMs / mRenderPerfSampleCount,
			mRenderPerfMaxFrameMs,
			mRenderPerfMaxCpuMs,
			mRenderPerfMaxPresentMs,
			mRenderPerfSampleCount);
		mRenderPerfAccumulatedFrameMs = 0;
		mRenderPerfAccumulatedCpuMs = 0;
		mRenderPerfAccumulatedPresentMs = 0;
		mRenderPerfMaxFrameMs = 0;
		mRenderPerfMaxCpuMs = 0;
		mRenderPerfMaxPresentMs = 0;
		mRenderPerfSampleCount = 0;
	}
}

/**
 * Read the control state for a specific player.
 * @param playerIdx The player index (0 = first player, 1 = second, etc.).
 * @return The control state.
 */
int MR_GameApp::ReadAsyncInputControllerPlayer(int playerIdx)
{
	const MR_Config::cfg_controls_t &ctl = MR_Config::GetInstance()->controls[playerIdx];
	int retv = 0;

	if (CheckKeyState(ctl.motorOn))  retv |= MR_MainCharacter::eMotorOn;
	if (CheckKeyState(ctl.jump))     retv |= MR_MainCharacter::eJump;
	if (CheckKeyState(ctl.brake))    retv |= MR_MainCharacter::eBreakDirection;
	if (CheckKeyState(ctl.fire))     retv |= MR_MainCharacter::eFire;
	if (CheckKeyState(ctl.weapon))   retv |= MR_MainCharacter::eSelectWeapon;
	if (CheckKeyState(ctl.lookBack)) retv |= MR_MainCharacter::eLookBack;
	if (CheckKeyState(ctl.right))    retv |= MR_MainCharacter::eRight;
	if (CheckKeyState(ctl.left))     retv |= MR_MainCharacter::eLeft;

	return retv;
}

void MR_GameApp::ReadAsyncInputController()
{
	ResetCachedJoystickState();

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

void MR_GameApp::SetVideoMode(int pX, int pY)
{
	if(mVideoBuffer != NULL) {
		BOOL lSuccess;

		PauseGameThread();

		mClrScrTodo = 2;

		if(pX == 0) {
			if(mDesktopFullscreen) {
				ExitDesktopFullscreen();
				lSuccess = TRUE;
			}
			else {
				lSuccess = mVideoBuffer->SetVideoMode();
			}

			SetTimer(mMainWindow, MRM_RETURN2WINDOWMODE, 3000, NULL);
		}
		else {
			lSuccess = mVideoBuffer->SetVideoMode(pX, pY);
			AssignPalette();
		}

		RestartGameThread();

		// OnDisplayChange();
	}
}

void MR_GameApp::PauseGameThread()
{
	if(mGameThread != NULL) {
		mGameThread->Pause();
	}
}

void MR_GameApp::RestartGameThread()
{
	if(mGameThread != NULL) {
		mGameThread->Restart();
	}
}

void MR_GameApp::OnDisplayChange()
{
	// Show or hide movie and display mode warning
	RECT lClientRect;
	BOOL lWindowSizeChanged = FALSE;

	if(!IsIconic(mMainWindow)) {
		if(GetClientRect(mMainWindow, &lClientRect)) {
			int lClientWidth = lClientRect.right - lClientRect.left;
			int lClientHeight = lClientRect.bottom - lClientRect.top;
			if((mVideoBuffer != NULL) && mVideoBuffer->IsWindowMode()) {
				lWindowSizeChanged =
					(lClientWidth != mVideoBuffer->GetDisplayXRes()) ||
					(lClientHeight != mVideoBuffer->GetDisplayYRes());
				if(lWindowSizeChanged) {
					RememberAdaptiveRenderScaleForSize(
						mVideoBuffer->GetDisplayXRes(),
						mVideoBuffer->GetDisplayYRes(),
						mAppliedRenderScalePercent);
				}
			}

			POINT lUpperLeft = { lClientRect.left, lClientRect.top };
			POINT lLowerRight = { lClientRect.right, lClientRect.bottom };
			RECT lMovieRect;
			RECT lBadModeRect;

			ClientToScreen(mMainWindow, &lUpperLeft);
			ClientToScreen(mMainWindow, &lLowerRight);

			if((mBadVideoModeDlg != NULL) && GetWindowRect(mBadVideoModeDlg, &lBadModeRect)) {
				if((mVideoBuffer != NULL) && !mVideoBuffer->IsWindowMode()) {
					ShowWindow(mBadVideoModeDlg, SW_HIDE);
				}
				else {

					SetWindowPos(mBadVideoModeDlg, HWND_TOP, (( /*lUpperLeft.x+ */ lLowerRight.x) - (lBadModeRect.right - lBadModeRect.left)),
						(( /*lUpperLeft.y+ */ lLowerRight.y) - (lBadModeRect.bottom - lBadModeRect.top)),
						0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				}
			}

			if(GetWindowRect(mMovieWnd, &lMovieRect)) {
				SetWindowPos(mMovieWnd, HWND_TOP,
					0, 0,
					lClientRect.right - lClientRect.left,
					lClientRect.bottom - lClientRect.top,
					SWP_SHOWWINDOW);
			}
		}
	}
	// Adjust video buffer
	if(mVideoBuffer != NULL) {
		if(IsIconic(mMainWindow)) {

			//PauseGameThread();
			mVideoBuffer->EnterIconMode();
			//RestartGameThread();

		}
		else {
			if(!mVideoBuffer->IsModeSettingInProgress()) {
				if(mVideoBuffer->IsIconMode()) {

					PauseGameThread();
					mClrScrTodo = 2;
					mVideoBuffer->ExitIconMode();
					mVideoBuffer->SetVideoMode();
					RestartGameThread();
					PostMessage(mMainWindow, WM_QUERYNEWPALETTE, 0, 0);

				}
				else {
					if(mVideoBuffer->IsWindowMode()) {
						if(lWindowSizeChanged) {
							ResetAdaptiveRenderScaleForResize(
								GetCachedAdaptiveRenderScaleForSize(
									lClientRect.right - lClientRect.left,
									lClientRect.bottom - lClientRect.top));
						}

						PauseGameThread();
						mClrScrTodo = 2;
						mVideoBuffer->SetVideoMode();
						RestartGameThread();
					}
				}
			}
		}
	}
}

void MR_GameApp::AssignPalette()
{
	if(mPaletteChangeAllowed) {
		if(This->mGameThread == NULL) {
			if(mMovieWnd != NULL)
				MCIWndRealize(mMovieWnd, FALSE);
		}
		else {
			if(This->mVideoBuffer != NULL)
				This->mVideoBuffer->AssignPalette();
		}
	}
}

// Find the desktop resolution of the primary monitor.
// Returns FALSE if the operation failed.
BOOL MR_GameApp::GetDesktopResolution(POINT* lpPoint)
{
	lpPoint->x = GetSystemMetrics(SM_CXSCREEN);
	lpPoint->y = GetSystemMetrics(SM_CYSCREEN);
	return (lpPoint->x != 0 && lpPoint->y != 0);
}

void MR_GameApp::DeleteMovieWnd()
{
	if(mMovieWnd != NULL) {

		MCIWndClose(mMovieWnd);
		Sleep(1000);
		MCIWndDestroy(mMovieWnd);
		// DestroyWindow( mMovieWnd );
		mMovieWnd = NULL;
	}
	// MR_SoundServer::Init( mMainWindow );
}

void MR_GameApp::NewLocalSession()
{
	bool lSuccess = true;

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	// Prompt the user for a track name
	std::string lCurrentTrack;
	int lNbLap;
	bool lAllowWeapons;
	bool lAllowCans;
	bool lAllowMines;
	unsigned lAllowedCraftMask;

	lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap,
		lAllowWeapons, lAllowCans, lAllowMines, lAllowedCraftMask);

	if(lSuccess) {
		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);
		mObserver1 = MR_Observer::New();

		// Create the new session
		MR_ClientSession *lCurrentSession = new MR_ClientSession();

		// Load the selected track
		if(lSuccess) {
			MR_RecordFile *lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack.c_str());
			lSuccess = (lCurrentSession->LoadNew(lCurrentTrack.c_str(), lTrackFile,
				lNbLap, lAllowWeapons, lAllowCans, lAllowMines,
				lAllowedCraftMask, mVideoBuffer) != FALSE);
		}
		// Create the main character
		if(lSuccess)
			lCurrentSession->SetSimulationTime(-6000);

		// Create the main character
		if(lSuccess)
			lSuccess = (lCurrentSession->CreateMainCharacter() != FALSE);

		if(lSuccess) {
			mCurrentSession = lCurrentSession;
			mGameThread = MR_GameThread::New(this);

			if(mGameThread == NULL) {
				mCurrentSession = NULL;
				delete lCurrentSession;
			}
		}										  // it failed, abort
		else {
			// Clean everything
			Clean();
			delete lCurrentSession;
		}
	}

	AssignPalette();
}

void MR_GameApp::NewSplitSession(int pSplitPlayers)
{
	bool lSuccess = true;

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	// Prompt the user for a maze name
	std::string lCurrentTrack;
	int lNbLap;
	bool lAllowWeapons;
	bool lAllowCans;
	bool lAllowMines;
	unsigned lAllowedCraftMask;

	lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap,
		lAllowWeapons, lAllowCans, lAllowMines, lAllowedCraftMask);

	if(lSuccess) {
		// Create the new session
		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);

		mObserver1 = MR_Observer::New();
		mObserver2 = MR_Observer::New();
		if(pSplitPlayers > 2)
			mObserver3 = MR_Observer::New();
		if(pSplitPlayers > 3)
			mObserver4 = MR_Observer::New();

		if(pSplitPlayers == 2) {
			mObserver1->SetSplitMode(MR_Observer::eUpperSplit);
			mObserver2->SetSplitMode(MR_Observer::eLowerSplit);
		}
		if(pSplitPlayers == 3) {
			mObserver1->SetSplitMode(MR_Observer::eUpperLeftSplit);
			mObserver2->SetSplitMode(MR_Observer::eUpperRightSplit);
			mObserver3->SetSplitMode(MR_Observer::eLowerLeftSplit);
		}
		if(pSplitPlayers == 4) {
			mObserver1->SetSplitMode(MR_Observer::eUpperLeftSplit);
			mObserver2->SetSplitMode(MR_Observer::eUpperRightSplit);
			mObserver3->SetSplitMode(MR_Observer::eLowerLeftSplit);
			mObserver4->SetSplitMode(MR_Observer::eLowerRightSplit);
		}

		MR_ClientSession *lCurrentSession = new MR_ClientSession;

		// Load the selected maze
		if(lSuccess) {
			MR_RecordFile *lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack.c_str());
			lSuccess = (lCurrentSession->LoadNew(lCurrentTrack.c_str(), lTrackFile,
				lNbLap, lAllowWeapons, lAllowCans, lAllowMines,
				lAllowedCraftMask, mVideoBuffer) != FALSE);
		}

		if(lSuccess) {
			lCurrentSession->SetSimulationTime(-8000);

			// Create the main character2
			lSuccess = (lCurrentSession->CreateMainCharacter() != FALSE);
		}

		if(lSuccess) {
			lSuccess = (lCurrentSession->CreateMainCharacter2() != FALSE);
			if(pSplitPlayers > 2)
				lSuccess = (lCurrentSession->CreateMainCharacter3() != FALSE);
			if(pSplitPlayers > 3)
				lSuccess = (lCurrentSession->CreateMainCharacter4() != FALSE);
		}

		if(!lSuccess) {
			// Clean everytings
			Clean();
			delete lCurrentSession;
		}
		else {
			mCurrentSession = lCurrentSession;
			mGameThread = MR_GameThread::New(this);

			if(mGameThread == NULL) {
				mCurrentSession = NULL;
				delete lCurrentSession;
			}
		}
	}

	AssignPalette();
}

void MR_GameApp::NewNetworkSession(BOOL pServer)
{
	bool lSuccess = true;
	MR_NetworkSession *lCurrentSession = NULL;
	MR_Config *cfg = MR_Config::GetInstance();

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	std::string lCurrentTrack;
	int lNbLap;
	bool lAllowWeapons;
	bool lAllowCans;
	bool lAllowMines;
	unsigned lAllowedCraftMask = MR_GetDefaultAllowedCraftMask();
	// Prompt the user for a maze name fbm extensions

	if(pServer) {
		lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap,
			lAllowWeapons, lAllowCans, lAllowMines, lAllowedCraftMask);

		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);
		lCurrentSession = new MR_NetworkSession(FALSE, -1, -1, mMainWindow);
	}
	else {
		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);

		lCurrentSession = new MR_NetworkSession(FALSE, -1, -1, mMainWindow);
		lCurrentSession->SetPlayerName(cfg->player.nickName.c_str());

		CString lTrack;
		lSuccess = (lCurrentSession->PreConnectToServer(mMainWindow, lTrack) != FALSE);
		lCurrentTrack = static_cast<const char*>(lTrack);

		if(cfg->player.nickName != lCurrentSession->GetPlayerName()) {
			cfg->player.nickName = lCurrentSession->GetPlayerName();
			SaveRegistry();
		}
		ParseTcpGameSummary(lCurrentTrack, lNbLap, lAllowWeapons, lAllowCans,
			lAllowMines, lAllowedCraftMask);
	}

	MR_RecordFile *lTrackFile;
	if(lSuccess) {
		mObserver1 = MR_Observer::New();

		// Load the track
		lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack.c_str());
		if (lTrackFile == NULL) {
			lSuccess = TrackDownloadDialog(lCurrentTrack).ShowModal(mInstance, mMainWindow);
			if (lSuccess) {
				lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack.c_str());
				if (lTrackFile == NULL) {
					lSuccess = FALSE;
				}
			}
		}
	}

	if(lSuccess) {
		lSuccess = (lCurrentSession->LoadNew(lCurrentTrack.c_str(), lTrackFile,
			lNbLap, lAllowWeapons, lAllowCans, lAllowMines,
			lAllowedCraftMask, mVideoBuffer) != FALSE);
	}

	if(lSuccess) {
		if(pServer) {
			CString lNameBuffer;
			std::string lAllowedCrafts =
				MR_FormatAllowedCraftMask(lAllowedCraftMask);

			lNameBuffer.Format("%s %d %s %s cans %s mines %s crafts %s",
				lCurrentTrack.c_str(), lNbLap, lNbLap > 1 ? "laps" : "lap",
				lAllowWeapons ? "with weapons" : "no weapons",
				lAllowCans ? "on" : "off",
				lAllowMines ? "on" : "off",
				lAllowedCrafts.c_str());

			// Create a net server
			lCurrentSession->SetPlayerName(cfg->player.nickName.c_str());

			lSuccess = (lCurrentSession->WaitConnections(mMainWindow, lNameBuffer, TRUE,
				MR_Config::GetInstance()->net.tcpServPort, NULL, 0,
				lCurrentTrack.c_str(), lNbLap, TRUE, lAllowWeapons,
				TRUE, lAllowCans, TRUE, lAllowMines, TRUE,
				lAllowedCraftMask) != FALSE);
			if(cfg->player.nickName != lCurrentSession->GetPlayerName()) {
				cfg->player.nickName = lCurrentSession->GetPlayerName();
				SaveRegistry();
			}
		} else
			lSuccess = (lCurrentSession->ConnectToServer(mMainWindow) != FALSE);
	}

	if(lSuccess) {
												  // start in 13 seconds
		lCurrentSession->SetSimulationTime(lCurrentSession->GetNbPlayers() == 1 ? -6000 : -13000);
		lSuccess = (lCurrentSession->CreateMainCharacter() != FALSE);
	}

	if(!lSuccess) {
		// Clean everytings
		Clean();
		delete lCurrentSession;
	}
	else {
		if(GetActiveWindow() != mMainWindow) {
			FLASHWINFO lFlash;
			lFlash.cbSize = sizeof(lFlash);
			lFlash.hwnd = mMainWindow;
			lFlash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			lFlash.uCount = 5;
			lFlash.dwTimeout = 0;

			FlashWindowEx(&lFlash);
		}

		mCurrentSession = lCurrentSession;
		mGameThread = MR_GameThread::New(this);

		if(mGameThread == NULL) {
			mCurrentSession = NULL;
			delete lCurrentSession;
		}
	}

	AssignPalette();
}

void MR_GameApp::NewInternetSessionCall()
{
	This->NewInternetSession();
}

// void* gNewInternetSessionPtr = (void*)(unsigned int)((unsigned int)MR_GameApp::NewInternetSessionCall-112);
unsigned int gNewInternetSessionPtrZ = (unsigned int) MR_GameApp::NewInternetSessionCall;
int gDummyPadding = 125;
unsigned int gNewInternetSessionPtr = gNewInternetSessionPtrZ - 112;
// void* gNewInternetSessionPtr = (void*)MR_GameApp::NewInternetSessionCall;

void MR_GameApp::NewInternetSession()
{
	BOOL lSuccess = TRUE;
	MR_NetworkSession *lCurrentSession = NULL;
	MR_Config *cfg = MR_Config::GetInstance();
	// MR_InternetRoom    lInternetRoom( gKeyFilled, gKeyFilled?mMajorID:-1, gKeyFilled?mMinorID:-1, gKeyFilled?gKey.mKeySumHard2:0, gKeyFilled?gKey.mKeySumHard3:0 );
	MR_InternetRoom lInternetRoom(TRUE, -1, -1, 0, 0, mSteamIDLocalUser, cfg->net.mainServer);

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();
	DeleteMovieWnd();
	MR_SoundServer::Init(mMainWindow);

	lCurrentSession = new MR_NetworkSession(TRUE, -1, -1, mMainWindow);

	if(lSuccess) {
		lCurrentSession->SetPlayerName(cfg->player.nickName.c_str());

		lSuccess = lInternetRoom.DisplayChatRoom(mMainWindow, lCurrentSession, mVideoBuffer, mServerHasChanged);

		lCurrentSession->SetRoomList(lInternetRoom.GetRoomList());
		mServerHasChanged = FALSE;

		if(cfg->player.nickName != lCurrentSession->GetPlayerName()) {
			cfg->player.nickName = lCurrentSession->GetPlayerName();
			SaveRegistry();
		}
	}

	if(lSuccess) {
												  // start in 20 seconds (this time may be readjusted by the server)
		lCurrentSession->SetSimulationTime(lCurrentSession->GetNbPlayers() == 1 ? -6000 : -20000);
	}

	if(lSuccess) {
		mObserver1 = MR_Observer::New();
		lSuccess = lCurrentSession->CreateMainCharacter();
	}

	if(!lSuccess) {
		// Clean everytings
		Clean();
		delete lCurrentSession;
	}
	else {
		mCurrentSession = lCurrentSession;
		mGameThread = MR_GameThread::New(this);

		if(mGameThread == NULL) {
			mCurrentSession = NULL;
			delete lCurrentSession;
		}
	}
	AssignPalette();

}

void MR_GameApp::SetProperties()
{
	PROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = mInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_DISPLAY_INTENSITY);
	psp[0].pfnDlgProc = DisplayIntensityDialogFunc;
	psp[0].pszTitle = MAKEINTRESOURCE(IDS_COLOR_INTENSITY);
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = mInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_CONTROL);
	psp[1].pfnDlgProc = ControlDialogFunc;
	psp[1].pszTitle = MAKEINTRESOURCE(IDS_KEYS_SETTING);
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE;
	psp[2].hInstance = mInstance;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_MISC);
	psp[2].pfnDlgProc = MiscDialogFunc;
	psp[2].pszTitle = MAKEINTRESOURCE(IDS_MISC);
	psp[2].lParam = 0;
	psp[2].pfnCallback = NULL;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent = mMainWindow;
	psh.hInstance = mInstance;
	psh.pszCaption = MAKEINTRESOURCE(IDS_PROP_SETTING);
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE) & psp;
	psh.pfnCallback = NULL;

	PropertySheet(&psh);

}

void MR_GameApp::DrawBackground()
{

	MR_UInt8 *lDest = mVideoBuffer->GetBuffer();
	int lXRes = mVideoBuffer->GetXRes();
	int lYRes = mVideoBuffer->GetYRes();
	int lDestLineStep = mVideoBuffer->GetLineLen() - lXRes;

	int lColorIndex;

	for(int lY = 0; lY < lYRes; lY++) {
		lColorIndex = lY;
		for(int lX = 0; lX < lXRes; lX++) {
			*lDest = (lColorIndex & 16) ? 11 : 11;

			lColorIndex++;
			lDest++;
		}
		lDest += lDestLineStep;
	}
}

// Attempt to switch fullscreen, using the current desktop resolution.
void MR_GameApp::SwitchToDesktopFullscreen()
{
	if(mDesktopFullscreen) {
		SetVideoMode(0, 0);
	}
	else {
		EnterDesktopFullscreen();
	}
}

BOOL MR_GameApp::GetDesktopFullscreenRect(RECT *pRect)
{
	return GetMonitorRectFromWindowCenter(mMainWindow, pRect);
}

BOOL MR_GameApp::GetActiveDesktopFullscreenRect(RECT *pRect)
{
	return GetMonitorRectForDevice(mDesktopFullscreenDevice, pRect) ||
		GetDesktopFullscreenRect(pRect);
}

BOOL MR_GameApp::GetPrimaryMonitorRect(RECT *pRect, char *deviceName)
{
	return GetPrimaryMonitorRectImpl(pRect, deviceName);
}

BOOL MR_GameApp::GetWindowMonitorRect(const RECT &windowRect, RECT *pRect, char *deviceName)
{
	POINT center;
	center.x = windowRect.left + ((windowRect.right - windowRect.left) / 2);
	center.y = windowRect.top + ((windowRect.bottom - windowRect.top) / 2);

	HMONITOR monitor = MonitorFromPoint(center, MONITOR_DEFAULTTONEAREST);
	if(monitor == NULL) {
		return FALSE;
	}

	MONITORINFOEX monitorInfo;
	memset(&monitorInfo, 0, sizeof(monitorInfo));
	monitorInfo.cbSize = sizeof(monitorInfo);
	if(!GetMonitorInfo(monitor, &monitorInfo)) {
		return FALSE;
	}

	*pRect = monitorInfo.rcMonitor;
	if(deviceName != NULL) {
		strcpy(deviceName, monitorInfo.szDevice);
	}
	return TRUE;
}

void MR_GameApp::ClampWindowRectToMonitor(RECT *pRect, const RECT &monitorRect)
{
	int width = pRect->right - pRect->left;
	int height = pRect->bottom - pRect->top;
	int monitorWidth = monitorRect.right - monitorRect.left;
	int monitorHeight = monitorRect.bottom - monitorRect.top;
	int minVisibleWidth;
	int minVisibleHeight;
	int minLeft;
	int maxLeft;
	int minTop;
	int maxTop;

	if(width < 320) width = 320;
	if(height < 240) height = 240;

	minVisibleWidth = min(width, monitorWidth);
	minVisibleHeight = min(height, monitorHeight);
	if(minVisibleWidth > 320) minVisibleWidth = 320;
	if(minVisibleHeight > 240) minVisibleHeight = 240;

	minLeft = monitorRect.left + minVisibleWidth - width;
	maxLeft = monitorRect.right - minVisibleWidth;
	minTop = monitorRect.top + minVisibleHeight - height;
	maxTop = monitorRect.bottom - minVisibleHeight;

	if(pRect->left < minLeft) pRect->left = minLeft;
	if(pRect->top < minTop) pRect->top = minTop;
	if(pRect->left > maxLeft) pRect->left = maxLeft;
	if(pRect->top > maxTop) pRect->top = maxTop;

	pRect->right = pRect->left + width;
	pRect->bottom = pRect->top + height;
}

void MR_GameApp::ResolveInitialWindowRect(RECT *pRect)
{
	MR_Config *cfg = MR_Config::GetInstance();

	RECT monitorRect;
	BOOL haveMonitorRect = FALSE;
	if(GetSystemMetrics(SM_CMONITORS) <= 1) {
		haveMonitorRect = GetPrimaryMonitorRect(&monitorRect);
	}
	else if(!cfg->video.windowMonitor.empty() &&
		GetMonitorRectForDevice(cfg->video.windowMonitor.c_str(), &monitorRect))
	{
		haveMonitorRect = TRUE;
	}
	else {
		haveMonitorRect = GetPrimaryMonitorRect(&monitorRect);
	}

	if(haveMonitorRect) {
		pRect->left = monitorRect.left + cfg->video.windowMonitorPosX;
		pRect->top = monitorRect.top + cfg->video.windowMonitorPosY;
	}
	else {
		pRect->left = 0;
		pRect->top = 0;
	}

	pRect->right = pRect->left + cfg->video.windowSizeX;
	pRect->bottom = pRect->top + cfg->video.windowSizeY;

	if(haveMonitorRect) {
		ClampWindowRectToMonitor(pRect, monitorRect);
	}
}

void MR_GameApp::ApplyDesktopFullscreenRect(const RECT &rect)
{
	// Inflate 1px on every edge so the WS_BORDER frame (set in
	// EnterDesktopFullscreen) sits off-screen. The visible client
	// rect ends up exactly covering the monitor.
	RECT adjustedRect = rect;
	adjustedRect.left -= 1;
	adjustedRect.top -= 1;
	adjustedRect.right += 1;
	adjustedRect.bottom += 1;

	WINDOWPLACEMENT placement;
	memset(&placement, 0, sizeof(placement));
	placement.length = sizeof(placement);
	if(GetWindowPlacement(mMainWindow, &placement)) {
		placement.flags = 0;
		placement.showCmd = IsIconic(mMainWindow) ? SW_RESTORE : SW_SHOWNORMAL;
		placement.rcNormalPosition = adjustedRect;
		SetWindowPlacement(mMainWindow, &placement);
	}

	// HWND_TOP instead of HWND_TOPMOST — topmost + borderless + full monitor
	// is a strong FSO trigger.
	SetWindowPos(mMainWindow, HWND_TOP,
		adjustedRect.left, adjustedRect.top,
		adjustedRect.right - adjustedRect.left,
		adjustedRect.bottom - adjustedRect.top,
		SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

void MR_GameApp::NormalizeWindowedRect(const RECT &monitorRect)
{
	if(MonitorFromRect(&mWindowedRect, MONITOR_DEFAULTTONULL) != NULL) {
		return;
	}

	ClampWindowRectToMonitor(&mWindowedRect, monitorRect);
}

void MR_GameApp::EnterDesktopFullscreen()
{
	if(mDesktopFullscreen || mMainWindow == NULL || mVideoBuffer == NULL) {
		return;
	}

	RECT monitorRect;
	if(!GetMonitorRectFromWindowCenter(mMainWindow, &monitorRect, mDesktopFullscreenDevice)) {
		return;
	}

	PauseGameThread();
	mClrScrTodo = 2;

	mWindowedStyle = GetWindowLong(mMainWindow, GWL_STYLE);
	mWindowedExStyle = GetWindowLong(mMainWindow, GWL_EXSTYLE);
	mWindowedMenu = GetMenu(mMainWindow);
	WINDOWPLACEMENT windowedPlacement;
	memset(&windowedPlacement, 0, sizeof(windowedPlacement));
	windowedPlacement.length = sizeof(windowedPlacement);
	if(GetWindowPlacement(mMainWindow, &windowedPlacement)) {
		mWindowedShowCmd = (windowedPlacement.showCmd == SW_SHOWMAXIMIZED) ?
			SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
		mWindowedRect = windowedPlacement.rcNormalPosition;
	}
	else {
		mWindowedShowCmd = IsZoomed(mMainWindow) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
		GetWindowRect(mMainWindow, &mWindowedRect);
	}

	SetMenu(mMainWindow, NULL);
	// Use WS_BORDER (not WS_POPUP) so the OpenGL ICD does not classify
	// the window as "borderless fullscreen" and switch the present path
	// to "Hardware: Legacy Flip" (true exclusive fullscreen, bypassing
	// DWM entirely, invisible to OBS Display Capture). WS_BORDER adds a
	// thin 1px frame around the client area; we compensate by inflating
	// the window rect by 1px on all sides so the border lands
	// off-screen and the visible client is still exactly the monitor.
	//
	// We deliberately do NOT use WS_POPUP here. Diagnostic confirmed
	// that in Hardware: Legacy Flip mode DWM is completely bypassed —
	// any overlay/anchor window becomes invisible regardless of z-order
	// because DWM isn't compositing. The only defeat is to prevent the
	// ICD from entering that path in the first place.
	SetWindowLong(mMainWindow, GWL_STYLE, WS_VISIBLE | WS_BORDER);
	SetWindowLong(mMainWindow, GWL_EXSTYLE, WS_EX_APPWINDOW);
	ApplyDesktopFullscreenRect(monitorRect);

	mDesktopFullscreen = true;
	SetTimer(mMainWindow, MRM_VALIDATE_DESKTOP_FULLSCREEN, 500, NULL);
	RememberAdaptiveRenderScaleForSize(
		mVideoBuffer->GetDisplayXRes(),
		mVideoBuffer->GetDisplayYRes(),
		mAppliedRenderScalePercent);
	mVideoBuffer->SetVideoMode();
	AssignPalette();

	RestartGameThread();
}

void MR_GameApp::ExitDesktopFullscreen()
{
	if(!mDesktopFullscreen || mMainWindow == NULL || mVideoBuffer == NULL) {
		return;
	}

	RECT monitorRect;
	if(GetActiveDesktopFullscreenRect(&monitorRect)) {
		NormalizeWindowedRect(monitorRect);
	}

	KillTimer(mMainWindow, MRM_VALIDATE_DESKTOP_FULLSCREEN);
	mDesktopFullscreen = false;
	SetMenu(mMainWindow, mWindowedMenu);
	SetWindowLong(mMainWindow, GWL_STYLE, mWindowedStyle);
	SetWindowLong(mMainWindow, GWL_EXSTYLE, mWindowedExStyle);

	WINDOWPLACEMENT placement;
	memset(&placement, 0, sizeof(placement));
	placement.length = sizeof(placement);
	if(GetWindowPlacement(mMainWindow, &placement)) {
		placement.flags = 0;
		placement.showCmd = mWindowedShowCmd;
		placement.rcNormalPosition = mWindowedRect;
		SetWindowPlacement(mMainWindow, &placement);
	}

	if(mWindowedShowCmd == SW_SHOWMAXIMIZED) {
		SetWindowPos(mMainWindow, HWND_NOTOPMOST,
			0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		ShowWindow(mMainWindow, SW_SHOWMAXIMIZED);
	}
	else {
		SetWindowPos(mMainWindow, HWND_NOTOPMOST,
			mWindowedRect.left, mWindowedRect.top,
			mWindowedRect.right - mWindowedRect.left,
			mWindowedRect.bottom - mWindowedRect.top,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	}

	mDesktopFullscreenDevice[0] = '\0';
	RememberAdaptiveRenderScaleForSize(
		mVideoBuffer->GetDisplayXRes(),
		mVideoBuffer->GetDisplayYRes(),
		mAppliedRenderScalePercent);
	mVideoBuffer->SetVideoMode();
	AssignPalette();
}

void MR_GameApp::RefreshDesktopFullscreenPlacement()
{
	if(!mDesktopFullscreen || mMainWindow == NULL || mVideoBuffer == NULL) {
		return;
	}

	RECT monitorRect;
	if(!GetActiveDesktopFullscreenRect(&monitorRect)) {
		return;
	}

	RECT windowRect;
	if(GetWindowRect(mMainWindow, &windowRect) &&
		EqualRect(&windowRect, &monitorRect))
	{
		return;
	}

	PauseGameThread();
	mClrScrTodo = 2;

	ApplyDesktopFullscreenRect(monitorRect);

	RememberAdaptiveRenderScaleForSize(
		mVideoBuffer->GetDisplayXRes(),
		mVideoBuffer->GetDisplayYRes(),
		mAppliedRenderScalePercent);
	mVideoBuffer->SetVideoMode();
	AssignPalette();

	RestartGameThread();
}

void MR_GameApp::UpdateMenuItems()
{
	MENUITEMINFO lMenuInfo;
	memset(&lMenuInfo, 0, sizeof(lMenuInfo));
	lMenuInfo.cbSize = sizeof(lMenuInfo);

	lMenuInfo.fMask = MIIM_STATE | MIIM_STRING;
	lMenuInfo.fState = MFS_ENABLED;
	lMenuInfo.dwTypeData = "&Fullscreen\tF11";
	lMenuInfo.cch = strlen(lMenuInfo.dwTypeData);

	HMENU lMenu = GetMenu(mMainWindow);
	SetMenuItemInfo(lMenu, ID_SETTING_DESKTOP, FALSE, &lMenuInfo);
}

LRESULT CALLBACK MR_GameApp::DispatchFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	// Catch environment modification events
	// TRACE( "MESSAGE %d W %d L %d\n", pMsgId, pWParam, pLParam );

	switch (pMsgId) {
		// Catch environment modification events
		/*
		   case WM_SHOWWINDOW:
		   if( pWParam && pLParam == SW_PARENTOPENING )
			   {
			   if( This->mVideoBuffer != NULL )
			   {
			   This->mVideoBuffer->ExitIconMode();
			   }
			   }
			   break;
		 */

		case WM_DISPLAYCHANGE:
			This->RefreshDesktopFullscreenPlacement();
			This->OnDisplayChange();
			break;

		case MRM_APPLY_ADAPTIVE_RENDER_SCALE:
			This->ApplyAdaptiveRenderScale(static_cast<int>(pWParam));
			return 0;

		case WM_WINDOWPOSCHANGING:
			if(This->mDesktopFullscreen) {
				RECT monitorRect;
				if(This->GetActiveDesktopFullscreenRect(&monitorRect)) {
					WINDOWPOS *windowPos = reinterpret_cast<WINDOWPOS*>(pLParam);
					windowPos->flags &= ~(SWP_NOMOVE | SWP_NOSIZE);
					windowPos->x = monitorRect.left;
					windowPos->y = monitorRect.top;
					windowPos->cx = monitorRect.right - monitorRect.left;
					windowPos->cy = monitorRect.bottom - monitorRect.top;
					windowPos->hwndInsertAfter = HWND_TOPMOST;
				}
			}
			break;

		case WM_ENTERSIZEMOVE:
			// Windows enters a modal resize/move loop. Pause the game
			// thread so the world doesn't keep running (and, more
			// importantly, so we don't keep tearing down and rebuilding
			// the video mode on every pixel of drag — that's what makes
			// live resize sluggish). We'll do a single OnDisplayChange
			// when the user releases the mouse in WM_EXITSIZEMOVE.
			if(!This->mInResizeLoop) {
				This->mInResizeLoop = true;
				This->PauseGameThread();
			}
			break;

		case WM_EXITSIZEMOVE:
			if(This->mInResizeLoop) {
				This->mInResizeLoop = false;
				This->RestartGameThread();
				// Now apply the final window size in one shot.
				This->OnDisplayChange();
			}
			break;

		case WM_SIZE:
		case WM_MOVE:
			// Skip per-pixel video mode resets during a live drag.
			// WM_EXITSIZEMOVE will call OnDisplayChange() once at the end.
			if(!This->mInResizeLoop) {
				This->OnDisplayChange();
			}
			break;

		case WM_QUERYNEWPALETTE:
			This->AssignPalette();
			return TRUE;

		case WM_PALETTECHANGED:
			if(This->mMovieWnd != NULL) {
				MCIWndRealize(This->mMovieWnd, TRUE);
			}
			break;

		case WM_ENTERMENULOOP:
			This->mInMenuLoop = true;
			KillTimer(pWindow, 42 /*cursor hide timer*/);
			if(!This->mCursorVisible) {
				This->mCursorVisible = TRUE;
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			// Don't fall out of borderless fullscreen when the menu is
			// invoked (Alt / F10). Only F11 / ESC should exit fullscreen.
			if(!This->mDesktopFullscreen) {
				This->SetVideoMode(0, 0);
			}
			This->UpdateMenuItems();
			break;

		case WM_EXITMENULOOP:
			This->mInMenuLoop = false;
			break;

		case WM_TIMER:
			switch (pWParam) {
				case MRM_RETURN2WINDOWMODE:
					KillTimer(pWindow, MRM_RETURN2WINDOWMODE);
					SetWindowPos(This->mMainWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

					// Patch, sometime the palette is lost??
					if(GetFocus() != NULL) {
						This->AssignPalette();
					}

					return 0;

				case MRM_VALIDATE_DESKTOP_FULLSCREEN:
					This->RefreshDesktopFullscreenPlacement();
					return 0;

				case 42: // cursor hide timer
					// Only auto-hide the cursor during active gameplay.
					// In menus / lobby the cursor must stay visible so the
					// user can click UI elements.
					if(This->mCursorVisible && !This->mInMenuLoop && This->IsGameRunning()
						&& (GetTickCount() - This->mLastMouseMoveTick) >= 2000) {
						This->mCursorVisible = FALSE;
						SetCursor(NULL);
					}
					KillTimer(pWindow, 42);
					return 0;
			}
			break;

			/* 
			   case WM_STYLECHANGING:
			   if( This->mVideoBuffer != NULL )
				   {
				   STYLESTRUCT* lStyle = (LPSTYLESTRUCT)pLParam;
	
				   if( pWParam == GWL_EXSTYLE )
				   {
				   ASSERT( FALSE );
				   // if( This->mVideoBuffer->IsWindowMode() )
				   if( lStyle->styleNew & WS_EX_TOPMOST )
				   {
				   SetWindowPos( pWindow, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );
				   ASSERT( FALSE );
				   // return 0;
				   }
				   // else
				   {
	
				   // lStyle->styleNew &= ~(WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE);
				   return 0;
				   }
				   }
				   else if( pWParam == GWL_STYLE )
				   {
				   lStyle->styleNew &= ~(WS_THICKFRAME);
				   return 0;
				   }
				   }
				   break;;
			 */

			/*
			   case WM_SYSCOMMAND:
			   switch( pWParam )
				   {
				   case SC_RESTORE:
				   if( (This->mVideoBuffer != NULL)&&!This->mVideoBuffer->IsWindowMode() )
				   {
				   This->SetVideoMode( 0, 0 );
				   return 0;
				   }
				   }
				   break;
			 */

		case WM_ACTIVATEAPP:
			//TRACE("WM_ACTIVATE %d\n", pWParam);

			if(pWParam && (This->mVideoBuffer != NULL) && (This->mMainWindow == GetForegroundWindow())) {
				if(!This->mVideoBuffer->IsModeSettingInProgress()) {
					// Don't kick the user out of borderless fullscreen just
					// because they alt-tabbed back in or clicked the window.
					// Only F11 / ESC should exit fullscreen.
					if(This->mVideoBuffer->IsWindowMode() && !This->mDesktopFullscreen) {
						//TRACE("SetMode\n");

						This->SetVideoMode(0, 0);
						This->AssignPalette();
						return 0;
					}
				}
			}

			break;

		case WM_SETCURSOR:
			if(LOWORD(pLParam) == HTCLIENT && !This->mCursorVisible && !This->mInMenuLoop) {
				SetCursor(NULL);
				return TRUE;
			}
			break;

		case WM_MOUSEMOVE:
			if(!This->mCursorVisible) {
				This->mCursorVisible = TRUE;
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			This->mLastMouseMoveTick = GetTickCount();
			// Only arm the auto-hide timer during active gameplay.
			if(!This->mInMenuLoop && This->IsGameRunning()) {
				SetTimer(pWindow, 42 /*cursor hide timer*/, 2000, NULL);
			}
			break;

			// Menu options
		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				// Game control
				case ID_APP_EXIT:
					PostMessage(This->mMainWindow, WM_CLOSE, 0, 0);
					break;

				case ID_GAME_PAUSE:
					This->SetVideoMode(0, 0);
					return 0;

					// just pop the menu
					/*
					   if( This->mVideoBuffer != NULL )
						   {
						   if( This->mVideoBuffer->IsFullScreen() )
						   {
						   SendMessage( pWindow, WM_SYSCOMMAND, SC_KEYMENU, 0 );
						   return 0;
						   }
						   }
						 */
						break;

				case ID_GAME_NEW:
					This->SetVideoMode(0, 0);
					This->NewLocalSession();
					return 0;

				case ID_GAME_SPLITSCREEN:
					This->SetVideoMode(0, 0);
					This->NewSplitSession(2);
					return 0;

				case ID_GAME_SPLITSCREEN3:
					This->SetVideoMode(0, 0);
					This->NewSplitSession(3);
					return 0;

				case ID_GAME_SPLITSCREEN4:
					This->SetVideoMode(0, 0);
					This->NewSplitSession(4);
					return 0;

				case ID_GAME_NETWORK_SERVER:
					This->SetVideoMode(0, 0);
					This->NewNetworkSession(TRUE);
					return 0;

				case ID_GAME_NETWORK_CONNECT:
					This->SetVideoMode(0, 0);
					This->NewNetworkSession(FALSE);
					return 0;

				case ID_GAME_NETWORK_INTERNET:
					This->SetVideoMode(0, 0);
					// Registration key code checking removed, no longer necessary
					This->NewInternetSession();
					return 0;

				case ID_LEVEL_EDITOR:
					STARTUPINFO si;
					PROCESS_INFORMATION pi;

					ZeroMemory( &si, sizeof(si) );
					si.cb = sizeof(si);
					ZeroMemory( &pi, sizeof(pi) );

					// Start the child process. 
					if( !CreateProcess( NULL,   // No module name (use command line)
						"HoverCAD.exe",		// Command line
						NULL,		   // Process handle not inheritable
						NULL,		   // Thread handle not inheritable
						FALSE,		  // Set handle inheritance to FALSE
						0,			  // No creation flags
						NULL,		   // Use parent's environment block
						NULL,		   // Use parent's starting directory 
						&si,			// Pointer to STARTUPINFO structure
						&pi )		   // Pointer to PROCESS_INFORMATION structure
					) 
					{
						printf( "CreateProcess failed (%d)\n", GetLastError() );
						return 0;
					}

					return 0;

				case ID_SETTING_REFRESHCOLORS:
					if(GetFocus() != NULL)
						This->AssignPalette();
					break;

					// Video mode setting
				case ID_SETTING_320X200:
					This->SetVideoMode(320, 200);
					return 0;

					/* -- DEPRECATED, 09/19/06 --
					   case ID_SETTING_320X240:
					   This->SetVideoMode(320, 240);
					   return 0;

					   case ID_SETTING_400X300:
					   This->SetVideoMode(400, 300);
					   return 0;

					   case ID_SETTING_512X384:
					   This->SetVideoMode(512, 384);
					   return 0;

					   case ID_SETTING_640X400:
					   This->SetVideoMode(600, 400);
					   return 0;
					 */

				case ID_SETTING_640X480:
					This->SetVideoMode(640, 480);
					return 0;

					// Higher resolutions added to keep up with the world
				case ID_SETTING_800X600:
					This->SetVideoMode(800, 600);
					return 0;

				case ID_SETTING_1024X768:
					This->SetVideoMode(1024, 768);
					return 0;

				case ID_SETTING_1280X768:
					This->SetVideoMode(1280, 768);
					return 0;

				case ID_SETTING_1280X800:
					This->SetVideoMode(1280, 800);
					return 0;

				case ID_SETTING_1280X1024:
					This->SetVideoMode(1280, 1024);
					return 0;

				case ID_SETTING_1400X1050:
					This->SetVideoMode(1400, 1050);
					return 0;

				case ID_SETTING_1600X1200:
					This->SetVideoMode(1600, 1200);
					return 0;

				case ID_SETTING_DESKTOP:
					This->SwitchToDesktopFullscreen();
					return 0;

				case ID_SETTING_PROPERTIES:
					This->SetVideoMode(0, 0);
					This->SetProperties();
					break;

				case ID_SETTING_WINDOW:
					This->SetVideoMode(0, 0);
					return 0;

				case ID_VIEW_3DACTION:
					This->mCurrentMode = e3DView;
					if(This->mObserver1 != NULL) {
						This->mObserver1->SetCockpitView(FALSE);
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->SetCockpitView(FALSE);
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->SetCockpitView(FALSE);
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->SetCockpitView(FALSE);
					}
					return 0;

				case ID_VIEW_COCKPIT:
					This->mCurrentMode = e3DView;
					if(This->mObserver1 != NULL) {
						This->mObserver1->SetCockpitView(TRUE);
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->SetCockpitView(TRUE);
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->SetCockpitView(TRUE);
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->SetCockpitView(TRUE);
					}
					return 0;

				case ID_VIEW_DEBUG:
					This->mCurrentMode = eDebugView;
					return 0;

					/* DISABLED BY AUSTIN: No need for the +/- to be used.

					   case ID_VIEW_LARGER:
					   if( This->mObserver1 != NULL )
						   {
						   This->mObserver1->ReduceMargin();
						   }
						   if( This->mObserver2 != NULL )
						   {
						   This->mObserver2->ReduceMargin();
						   }
						   if( This->mObserver3 != NULL )
						   {
						   This->mObserver3->ReduceMargin();
						   }
						   if( This->mObserver4 != NULL )
						   {
						   This->mObserver4->ReduceMargin();
						   }
						   return 0;
	
						   case ID_VIEW_SMALLER:
						   if( This->mObserver1 != NULL )
						   {
						   This->mObserver1->EnlargeMargin();
						   }
						   if( This->mObserver2 != NULL )
						   {
						   This->mObserver2->EnlargeMargin();
						   }
						   if( This->mObserver3 != NULL )
						   {
						   This->mObserver3->EnlargeMargin();
						   }
						   if( This->mObserver4 != NULL )
						   {
						   This->mObserver4->EnlargeMargin();
						   }
						   This->mClrScrTodo = 2;
						   return 0;
						 */
	
					case ID_VIEW_PLAYERSLIST:
						if(This->mObserver1 != NULL) {
							This->mObserver1->PlayersListPageDn();
						}
						if(This->mObserver2 != NULL) {
							This->mObserver2->PlayersListPageDn();
						}
						if(This->mObserver3 != NULL) {
							This->mObserver3->PlayersListPageDn();
						}
						if(This->mObserver4 != NULL) {
							This->mObserver4->PlayersListPageDn();
						}
						return 0;
	
					case ID_VIEW_MOREMESSAGES:
						if(This->mObserver1 != NULL) {
							This->mObserver1->MoreMessages();
						}
						if(This->mObserver2 != NULL) {
							This->mObserver2->MoreMessages();
						}
						if(This->mObserver3 != NULL) {
							This->mObserver3->MoreMessages();
						}
						if(This->mObserver4 != NULL) {
							This->mObserver4->MoreMessages();
						}
						return 0;
	
				case ID_HELP_CONTENTS:
					This->DisplayHelp();
					break;

				case ID_HELP_THEBETAZONE:
					This->DisplayBetaZone();
					break;

				case ID_HELP_SITE:
					This->DisplaySite();
					break;

				case ID_APP_ABOUT:
					This->DisplayAbout();
					break;

			}
			break;

		case WM_CHAR:
			if(This->mCurrentSession != NULL) {
				This->mCurrentSession->AddMessageKey((char) pWParam);
			}
			return 0;

		case WM_KEYDOWN:
			switch (pWParam) {
				// Camera control
				case VK_HOME:
					if(This->mObserver1 != NULL) {
						This->mObserver1->Home();
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->Home();
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->Home();
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->Home();
					}
					return 0;

				case VK_PRIOR:
					if(This->mObserver1 != NULL) {
						This->mObserver1->Scroll(1);
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->Scroll(1);
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->Scroll(1);
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->Scroll(1);
					}
					return 0;

				case VK_NEXT:
					if(This->mObserver1 != NULL) {
						This->mObserver1->Scroll(-1);
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->Scroll(-1);
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->Scroll(-1);
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->Scroll(-1);
					}
					return 0;

				case VK_INSERT:
					if(This->mObserver1 != NULL) {
						This->mObserver1->ZoomIn();
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->ZoomIn();
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->ZoomIn();
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->ZoomIn();
					}
					return 0;

				case VK_DELETE:
					if(This->mObserver1 != NULL) {
						This->mObserver1->ZoomOut();
					}
					if(This->mObserver2 != NULL) {
						This->mObserver2->ZoomOut();
					}
					if(This->mObserver3 != NULL) {
						This->mObserver3->ZoomOut();
					}
					if(This->mObserver4 != NULL) {
						This->mObserver4->ZoomOut();
					}
					return 0;

					// Debug keys
					/*
					   case VK_F6:
					   SetWindowPos( This->mMainWindow,
					   HWND_TOPMOST,
					   0,0,0,0,
					   SWP_NOSIZE|SWP_NOMOVE );
					   return 0;

					   case VK_F7:
					   SetWindowPos( This->mMainWindow,
					   HWND_NOTOPMOST,
					   0,0,0,0,
					   SWP_NOSIZE|SWP_NOMOVE );
					   return 0;

					   case VK_F8:
					   #ifdef MR_AVI_CAPTURE
					   InitCapture( "demo.avi", This->mVideoBuffer );
					   #endif
					   This->mNbFrames = 0;
					   This->mNbFramesStartingTime = time( NULL );
					   return 0;

					   case VK_F9:
					   #ifdef MR_AVI_CAPTURE
					   CloseCapture();
					   #endif

					   TRACE( "Refresh rate = %d f/s( %d/%d)\n",
					   (int)((double)This->mNbFrames/(double(time(NULL)-This->mNbFramesStartingTime))),
					   This->mNbFrames,
					   (time(NULL)-This->mNbFramesStartingTime) );
					   return 0;
					 */

			}
			break;

		case WM_PAINT:
			// Nottng to paint (All done by video or DirectX
			PAINTSTRUCT lPs;
			BeginPaint(pWindow, &lPs);
			EndPaint(pWindow, &lPs);
			return 0;

		case WM_CLOSE:
		{
			if(This->IsGameRunning()) {
				This->SetVideoMode(0, 0);
				if(This->AskUserToAbortGame() != IDOK) {
					return 0;
				}
			}
			else if(This->mDesktopFullscreen) {
				This->ExitDesktopFullscreen();
			}
			This->Clean();
			delete This->mVideoBuffer;
			This->mVideoBuffer = NULL;

			RECT rect;
			BOOL windowMaximized = FALSE;
			if(This->mDesktopFullscreen) {
				rect = This->mWindowedRect;
				windowMaximized = (This->mWindowedShowCmd == SW_SHOWMAXIMIZED);
			}
			else {
				WINDOWPLACEMENT placement;
				memset(&placement, 0, sizeof(placement));
				placement.length = sizeof(placement);
				if(GetWindowPlacement(This->mMainWindow, &placement)) {
					windowMaximized = (placement.showCmd == SW_SHOWMAXIMIZED);
					rect = placement.rcNormalPosition;
				}
				else {
					windowMaximized = IsZoomed(This->mMainWindow);
					GetWindowRect(This->mMainWindow, &rect);
				}
			}

			MR_Config *cfg = MR_Config::GetInstance();
			RECT monitorRect;
			char monitorDevice[CCHDEVICENAME] = {0};
			if(This->GetWindowMonitorRect(rect, &monitorRect, monitorDevice) ||
				This->GetPrimaryMonitorRect(&monitorRect, monitorDevice))
			{
				cfg->video.windowMonitor = monitorDevice;
				cfg->video.windowMonitorPosX = rect.left - monitorRect.left;
				cfg->video.windowMonitorPosY = rect.top - monitorRect.top;
			}
			else {
				cfg->video.windowMonitor = "";
				cfg->video.windowMonitorPosX = 0;
				cfg->video.windowMonitorPosY = 0;
			}

			cfg->video.windowPosX = rect.left;
			cfg->video.windowPosY = rect.top;
			cfg->video.windowSizeX = rect.right - rect.left;
			cfg->video.windowSizeY = rect.bottom - rect.top;
			cfg->video.windowMaximized = (windowMaximized != FALSE);

			cfg->Save();

			DestroyWindow(This->mMainWindow);
			return 0;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

	}

	// Default return value
	return DefWindowProc(pWindow, pMsgId, pWParam, pLParam);
}

BOOL CALLBACK MR_GameApp::DisplayIntensityDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	ASSERT(This != NULL);
	ASSERT(This->mVideoBuffer != NULL);
	MR_Config *cfg = MR_Config::GetInstance();

	BOOL lReturnValue = FALSE;

	static double lOriginalGamma;
	static double lOriginalContrast;
	static double lOriginalBrightness;
	static float lOriginalSfxVolume;
   static int lOriginalSleepLength;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			This->mVideoBuffer->GetPaletteAttrib(lOriginalGamma, lOriginalContrast, lOriginalBrightness);
			lOriginalSfxVolume = cfg->audio.sfxVolume;
         lOriginalSleepLength = cfg->misc.mainLoopSleepLength;

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 200));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
         SendDlgItemMessage(pWindow, IDC_SLEEP_LENGTH, TBM_SETRANGE, 0, MAKELONG(0, 30));

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETPOS, TRUE, long (lOriginalGamma * 100));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETPOS, TRUE, long (lOriginalContrast * 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, long (lOriginalBrightness * 100));
			SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_SETPOS, TRUE, long (lOriginalSfxVolume * 100));
         SendDlgItemMessage(pWindow, IDC_SLEEP_LENGTH, TBM_SETPOS, TRUE, long (lOriginalSleepLength));

			UpdateIntensityDialogLabels(pWindow);
			break;

		case WM_HSCROLL:
			cfg->video.gamma = SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->video.contrast = SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->video.brightness = SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->audio.sfxVolume = SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_GETPOS, 0, 0) / 100.0f;
         cfg->misc.mainLoopSleepLength = SendDlgItemMessage(pWindow, IDC_SLEEP_LENGTH, TBM_GETPOS, 0, 0);

			UpdateIntensityDialogLabels(pWindow);

			This->mVideoBuffer->CreatePalette(cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
			This->AssignPalette();
			break;

		case TB_ENDTRACK:
			{
				double lGamma;
				double lContrast;
				double lBrightness;
	
				This->mVideoBuffer->GetPaletteAttrib(lGamma, lContrast, lBrightness);
	
				SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETPOS, TRUE, long (lGamma * 100));
				SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETPOS, TRUE, long (lContrast * 100));
				SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, long (lBrightness * 100));
			}
			break;

		// Menu options
		/*
		   case WM_COMMAND:
		   switch(LOWORD( pWParam))
			   {
			   // Game control
			   case IDCANCEL:
			   This->mVideoBuffer->CreatePalette( lOriginalGamma, lOriginalContrast, lOriginalBrightness );
			   EndDialog( pWindow, IDCANCEL );
			   lReturnValue = TRUE;
			   break;

		   case IDOK:
		   EndDialog( pWindow, IDOK );
		   lReturnValue = TRUE;
		   break;
		   }
		   break;
		 */

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_RESET:
					cfg->audio.sfxVolume = lOriginalSfxVolume;
               cfg->misc.mainLoopSleepLength = lOriginalSleepLength;
					This->mVideoBuffer->CreatePalette(lOriginalGamma, lOriginalContrast, lOriginalBrightness);
					This->AssignPalette();
					break;

				case PSN_APPLY:
					This->SaveRegistry();

					// SetWindowLong( ((NMHDR FAR *) lParam)->hwndFrom,  , );
					break;

			}
			break;

	}

	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::ControlDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	ASSERT(This != NULL);
	ASSERT(This->mVideoBuffer != NULL);
	MR_Config *cfg = MR_Config::GetInstance();
	ControlDialogState *state =
		reinterpret_cast<ControlDialogState *>(GetWindowLongPtr(pWindow, GWLP_USERDATA));

	BOOL lReturnValue = FALSE;
	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			state = new ControlDialogState;
			if(state == NULL) {
				return FALSE;
			}

			for(int playerIdx = 0; playerIdx < MR_Config::MAX_PLAYERS; ++playerIdx) {
				state->controls[playerIdx] = cfg->controls[playerIdx];
				for(int actionIdx = 0; actionIdx < NB_CONTROL_ACTIONS; ++actionIdx) {
					SetControlBinding(state->controls[playerIdx], actionIdx,
						ClampKeyChoiceIndex(playerIdx,
							GetControlBinding(state->controls[playerIdx], actionIdx)));
				}
			}

			SetWindowLongPtr(pWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
			RefreshControlButtons(pWindow, state);
			break;

		case WM_COMMAND:
			if((HIWORD(pWParam) == BN_CLICKED) && (state != NULL)) {
				const int controlId = LOWORD(pWParam);
				int playerIdx = 0;
				int actionIdx = 0;

				if(FindControlBindingById(controlId, &playerIdx, &actionIdx)) {
					char prompt[128];
					int bindingIndex = GetControlBinding(state->controls[playerIdx], actionIdx);

					BuildControlPrompt(prompt, sizeof(prompt), playerIdx, actionIdx);
					if(CaptureControlBinding(pWindow, playerIdx, prompt, &bindingIndex)) {
						SetControlBinding(state->controls[playerIdx], actionIdx, bindingIndex);
						UpdateControlButtonText(pWindow, playerIdx, actionIdx, bindingIndex);
						SendMessage(GetParent(pWindow), PSM_CHANGED, reinterpret_cast<WPARAM>(pWindow), 0);
					}
					return TRUE;
				}

				playerIdx = FindPlayerSetupButton(controlId);
				if(playerIdx >= 0) {
					BOOL changed = FALSE;

					for(actionIdx = 0; actionIdx < NB_CONTROL_ACTIONS; ++actionIdx) {
						char prompt[128];
						int bindingIndex = GetControlBinding(state->controls[playerIdx], actionIdx);

						BuildControlPrompt(prompt, sizeof(prompt), playerIdx, actionIdx,
							actionIdx + 1, NB_CONTROL_ACTIONS);
						if(!CaptureControlBinding(pWindow, playerIdx, prompt, &bindingIndex)) {
							break;
						}

						SetControlBinding(state->controls[playerIdx], actionIdx, bindingIndex);
						changed = TRUE;
					}

					if(changed) {
						RefreshControlButtons(pWindow, state);
						SendMessage(GetParent(pWindow), PSM_CHANGED, reinterpret_cast<WPARAM>(pWindow), 0);
					}
					return TRUE;
				}

				playerIdx = FindResetPlayerButton(controlId);
				if(playerIdx >= 0) {
					MR_Config::cfg_controls_t defaultControls[MR_Config::MAX_PLAYERS];
					LoadDefaultControls(defaultControls);
					state->controls[playerIdx] = defaultControls[playerIdx];
					RefreshControlButtons(pWindow, state);
					SendMessage(GetParent(pWindow), PSM_CHANGED, reinterpret_cast<WPARAM>(pWindow), 0);
					return TRUE;
				}
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					if(state != NULL) {
						for(int playerIdx = 0; playerIdx < MR_Config::MAX_PLAYERS; ++playerIdx) {
							cfg->controls[playerIdx] = state->controls[playerIdx];
						}
					}

					This->SaveRegistry();
					break;
			}
			break;

		case WM_DESTROY:
			delete state;
			SetWindowLongPtr(pWindow, GWLP_USERDATA, 0);
			break;
	}

	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::MiscDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	ASSERT(This != NULL);
	MR_Config *cfg = MR_Config::GetInstance();

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			// Set default values correctly
			SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_SETCHECK, !cfg->misc.introMovie, 0);
			SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_SETCHECK, !cfg->misc.displayFirstScreen, 0);
			SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_SETCHECK, cfg->video.nativeBppFullscreen, 0);
			SendDlgItemMessage(pWindow, IDC_USE_ORIGINAL_CAMERA_PARAMS, BM_SETCHECK, cfg->video.useOriginalCameraParams, 0);
			SendDlgItemMessage(pWindow, IDC_DIRECT_CONNECT, BM_SETCHECK, cfg->misc.directConnect, 0);

			SetDlgItemText(pWindow, IDC_MAINSERVER, cfg->net.mainServer.c_str());
			{
				char lBuffer[20];
				sprintf(lBuffer, "%d", cfg->net.tcpServPort);
				SetDlgItemText(pWindow, IDC_TCP_SERV_PORT, lBuffer);
				
				sprintf(lBuffer, "%d", cfg->net.tcpRecvPort);
				SetDlgItemText(pWindow, IDC_TCP_RECV_PORT, lBuffer);

				sprintf(lBuffer, "%d", cfg->net.udpRecvPort);
				SetDlgItemText(pWindow, IDC_UDP_RECV_PORT, lBuffer);
			}

			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					{
						const BOOL lOldUseOriginalCameraParams = cfg->video.useOriginalCameraParams;

						cfg->misc.introMovie = !SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_GETCHECK, 0, 0);
						cfg->misc.displayFirstScreen = !SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_GETCHECK, 0, 0);
						cfg->video.nativeBppFullscreen = (SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_GETCHECK, 0, 0) != FALSE);
						cfg->video.useOriginalCameraParams = (SendDlgItemMessage(pWindow, IDC_USE_ORIGINAL_CAMERA_PARAMS, BM_GETCHECK, 0, 0) != FALSE);
						cfg->misc.directConnect = (SendDlgItemMessage(pWindow, IDC_DIRECT_CONNECT, BM_GETCHECK, 0, 0) != FALSE);

					{
						char lBuffer[80];
						if(GetDlgItemText(pWindow, IDC_MAINSERVER, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.mainServer = lBuffer;
						This->mServerHasChanged = TRUE;

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

						if(lOldUseOriginalCameraParams != cfg->video.useOriginalCameraParams &&
							This->mVideoBuffer != NULL &&
							!This->mVideoBuffer->IsModeSettingInProgress())
						{
							if(This->mObserver1 != NULL) This->mObserver1->InvalidateViewportMetrics();
							if(This->mObserver2 != NULL) This->mObserver2->InvalidateViewportMetrics();
							if(This->mObserver3 != NULL) This->mObserver3->InvalidateViewportMetrics();
							if(This->mObserver4 != NULL) This->mObserver4->InvalidateViewportMetrics();

							This->PauseGameThread();
							This->mClrScrTodo = 2;

							if(This->mVideoBuffer->IsWindowMode()) {
								This->mVideoBuffer->SetVideoMode();
								This->RestartGameThread();
								if(This->mMainWindow != NULL) {
									PostMessage(This->mMainWindow, WM_QUERYNEWPALETTE, 0, 0);
								}
							}
							else {
								int lXRes = This->mVideoBuffer->GetDisplayXRes();
								int lYRes = This->mVideoBuffer->GetDisplayYRes();
								if(lXRes <= 0) {
									lXRes = This->mVideoBuffer->GetXRes();
								}
								if(lYRes <= 0) {
									lYRes = This->mVideoBuffer->GetYRes();
								}
								This->mVideoBuffer->SetVideoMode(lXRes, lYRes);
								This->AssignPalette();
								This->RestartGameThread();
							}
						}

						This->SaveRegistry();
					}
					break;

			}
			break;

	}

	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::BadModeDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				// Game control
				case IDC_FULL_MENU_MODE:
					if(This->mGameThread == NULL) {
						// This->SetVideoMode( 320, 200 );
						// This->mVideoBuffer->PushMenuMode();
					}
					else {
						This->SetVideoMode(320, 200);
						// This->mVideoBuffer->PushMenuMode();
					}
					break;
			}

	}

	return FALSE;
}

BOOL CALLBACK MR_GameApp::FirstChoiceDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
	{
		BOOL lReturnValue = FALSE;
		HWND okButton;
	
		switch (pMsgId) {
			// Catch environment modification events
			case WM_INITDIALOG:
				CheckDlgButton(pWindow, IDC_CHECK, BST_CHECKED);

				okButton = GetDlgItem(pWindow, IDOK);

				// Check if a custom LPARAM was passed (optional)
				if (pLParam == (LPARAM)IDOK) {
					SetFocus(okButton);  // Set focus to the OK button if IDOK was passed
				}

				lReturnValue = TRUE;
				break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				// Game control
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:

					EndDialog(pWindow, IDOK);
					lReturnValue = TRUE;
					break;
			}
			break;
	}

	return FALSE;
}

BOOL CALLBACK MR_GameApp::AboutDlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	static HBITMAP lBitmap = NULL;
	static HPALETTE lPalette = NULL;

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events

		case WM_INITDIALOG:
			{
				SetDlgItemText(pWindow, IDC_ABOUT_TXT, (const char*)MR_LoadString(IDS_ABOUT));
				/*
				This->mPaletteChangeAllowed = FALSE;
	
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);
	
				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);
	
					lBitmap = LoadResourceBitmap(This->mInstance, MAKEINTRESOURCE(IDB_ABOUT), &lPalette);
	
					ASSERT(lBitmap != NULL);
	
					SelectPalette(hdc, lPalette, FALSE);
					int lNbColors = RealizePalette(hdc);
					TRACE("Colors0 %d  %d\n", lNbColors, GetLastError());
					ReleaseDC(lBitmapCtl, hdc);
					// SelectPalette(hMemDC,hPalette,FALSE);
					// RealizePalette(hMemDC);
	
					HANDLE lOldHandle = (HANDLE) SendMessage(lBitmapCtl, STM_SETIMAGE, IMAGE_BITMAP, (long) lBitmap);
				}
				else {
					lBitmap = NULL;
					lPalette = NULL;
				}
				*/
				lReturnValue = TRUE;
			}
			break;

		case WM_DESTROY:
			if(lBitmap != NULL) {
				DeleteObject(lBitmap);
				lBitmap = NULL;
			}

			if(lPalette != NULL) {
				UnrealizeObject(lPalette);
				DeleteObject(lPalette);
				lPalette = NULL;
			}
			This->mPaletteChangeAllowed = TRUE;
			break;

		case WM_QUERYNEWPALETTE:
			if(lPalette != NULL) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);

				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);

					HPALETTE lOldPalette = SelectPalette(hdc, lPalette, FALSE);
					RealizePalette(hdc);

					InvalidateRgn(pWindow, NULL, TRUE);
					InvalidateRgn(lBitmapCtl, NULL, TRUE);
					UpdateWindow(pWindow);
					UpdateWindow(lBitmapCtl);

					ReleaseDC(lBitmapCtl, hdc);

					TRACE("PAL_SET\n");

				}
				lReturnValue = TRUE;
			}
			break;

		case WM_PALETTECHANGED:
			if((lPalette != NULL) && ((HWND) pWParam != pWindow)) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);

				if((lBitmapCtl != NULL) && ((HWND) pWParam != lBitmapCtl)) {
					HDC hdc = GetDC(lBitmapCtl);

					HPALETTE lOldPalette = SelectPalette(hdc, lPalette, FALSE);
					RealizePalette(hdc);

					UpdateColors(hdc);

					ReleaseDC(lBitmapCtl, hdc);

					TRACE("PAL_CHANGE\n");
				}
				lReturnValue = TRUE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {

				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					EndDialog(pWindow, IDOK);
					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

// Microsoft bitmap stuff
HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE FAR * lphPalette)
{
	HRSRC hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;
	HDC hdc;
	int iNumColors;

	if(hRsrc = FindResource(hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource(hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER) LockResource(hGlobal);

		hdc = GetDC(NULL);
		*lphPalette = CreateDIBPalette((LPBITMAPINFO) lpbi, &iNumColors);
		if(*lphPalette) {
			SelectPalette(hdc, *lphPalette, FALSE);
			RealizePalette(hdc);
		}

		hBitmapFinal = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER) lpbi, (LONG) CBM_INIT, (LPSTR) lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD), (LPBITMAPINFO) lpbi, DIB_RGB_COLORS);

		ReleaseDC(NULL, hdc);
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
	}
	return (hBitmapFinal);
}

HPALETTE CreateDIBPalette(LPBITMAPINFO lpbmi, LPINT lpiNumColors)
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	int i;

	lpbi = (LPBITMAPINFOHEADER) lpbmi;
	if(lpbi->biBitCount <= 8)
		*lpiNumColors = (1 << lpbi->biBitCount);
	else
		*lpiNumColors = 0;						  // No palette needed for 24 BPP DIB

	if(*lpiNumColors) {
		hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock(hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;

		for(i = 0; i < *lpiNumColors; i++) {
			lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette(lpPal);
		GlobalUnlock(hLogPal);
		GlobalFree(hLogPal);
	}
	return hPal;
}

/**
 * Update the labels for the intensity sliders.
 * @param pWindow Dialog handle.
 */
void MR_GameApp::UpdateIntensityDialogLabels(HWND pWindow)
{
	char buf[5];
	buf[4] = '\0';

	long gamma = SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0);
	long contrast = SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0);
	long brightness = SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0);
	long sfxVolume = SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_GETPOS, 0, 0);
   long sleepLength = SendDlgItemMessage(pWindow, IDC_SLEEP_LENGTH, TBM_GETPOS, 0, 0);

	_snprintf(buf, 4, "%01d.%02d", gamma / 100, gamma % 100);
	SetDlgItemText(pWindow, IDC_GAMMA_TXT, buf);
	_snprintf(buf, 4, "%01d.%02d", contrast / 100, contrast % 100);
	SetDlgItemText(pWindow, IDC_CONTRAST_TXT, buf);
	_snprintf(buf, 4, "%01d.%02d", brightness / 100, brightness % 100);
	SetDlgItemText(pWindow, IDC_BRIGHTNESS_TXT, buf);
	_snprintf(buf, 4, "%01d.%02d", sfxVolume / 100, sfxVolume % 100);
	SetDlgItemText(pWindow, IDC_SFX_VOLUME_TXT, buf);
   _snprintf(buf, 4, "%02d", sleepLength);
   SetDlgItemText(pWindow, IDC_SLEEP_TXT, buf);
}

/////////////////////////////////   AVI SECTION  //////////////////////////////////////

#ifdef MR_AVI_CAPTURE

void InitCapture(const char *pFileName, MR_VideoBuffer * pVideoBuffer)
{

	AVISTREAMINFO lStreamInfo;

	CloseCapture();
	AVIFileInit();

	gCaptureFrameNo = 0;

	// Create new AVI file.
	AVIFileOpen(&gCaptureFile, pFileName, OF_WRITE | OF_CREATE, NULL);

	// Set parameters for the new stream
	lStreamInfo.fccType = streamtypeVIDEO;
	lStreamInfo.fccHandler = NULL;
	lStreamInfo.dwFlags = 0;
	lStreamInfo.dwCaps = 0;
	lStreamInfo.wPriority = 0;
	lStreamInfo.wLanguage = 0;
	lStreamInfo.dwScale = 1;
	lStreamInfo.dwRate = gCaptureFrameRate;
	lStreamInfo.dwStart = 0;
	lStreamInfo.dwLength = 30;					  // ?
	lStreamInfo.dwInitialFrames = 0;
	lStreamInfo.dwSuggestedBufferSize = pVideoBuffer->GetXRes() * pVideoBuffer->GetYRes() * 4;
	lStreamInfo.dwQuality = -1;
	lStreamInfo.dwSampleSize = pVideoBuffer->GetXRes() * pVideoBuffer->GetYRes();
	SetRect(&lStreamInfo.rcFrame, 0, 0, pVideoBuffer->GetXRes(), pVideoBuffer->GetYRes());
	lStreamInfo.dwEditCount = 0;
	lStreamInfo.dwFormatChangeCount = 0;
	strcpy(lStreamInfo.szName, "(C)GrokkSoft 1996");

	// Create a stream.
	AVIFileCreateStream(gCaptureFile, &gCaptureStream, &lStreamInfo);

	// Set format of new stream.
	int lInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (MR_NB_COLORS - MR_RESERVED_COLORS);

	BITMAPINFO *lBitmapInfo = (BITMAPINFO *) new char[lInfoSize];

	lBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lBitmapInfo->bmiHeader.biWidth = pVideoBuffer->GetXRes();
	lBitmapInfo->bmiHeader.biHeight = pVideoBuffer->GetYRes();
	lBitmapInfo->bmiHeader.biPlanes = 1;
	lBitmapInfo->bmiHeader.biBitCount = 8;
	lBitmapInfo->bmiHeader.biCompression = BI_RGB;
	lBitmapInfo->bmiHeader.biSizeImage = pVideoBuffer->GetXRes() * pVideoBuffer->GetYRes();
	lBitmapInfo->bmiHeader.biXPelsPerMeter = 2048;
	lBitmapInfo->bmiHeader.biYPelsPerMeter = 2048;
	lBitmapInfo->bmiHeader.biClrUsed = MR_NB_COLORS - MR_RESERVED_COLORS;
	lBitmapInfo->bmiHeader.biClrImportant = 0;

	// Create the palette
	PALETTEENTRY *lOurEntries = MR_GetColors(0.75, 0.75, 0.05);

	for(int lCounter = 0; lCounter < MR_NB_COLORS - MR_RESERVED_COLORS; lCounter++) {
		lBitmapInfo->bmiColors[lCounter].rgbRed = lOurEntries[lCounter].peRed;
		lBitmapInfo->bmiColors[lCounter].rgbGreen = lOurEntries[lCounter].peGreen;
		lBitmapInfo->bmiColors[lCounter].rgbBlue = lOurEntries[lCounter].peBlue;
		lBitmapInfo->bmiColors[lCounter].rgbReserved = 0;
	};
	delete lOurEntries;

	/*
	   for( lCounter = 0; lCounter<MR_RESERVED_COLORS; lCounter++ )
	   {
	   lBitmapInfo->bmiColors[ lCounter ].rgbRed   = 1;
	   lBitmapInfo->bmiColors[ lCounter ].rgbGreen = lCounter;
	   lBitmapInfo->bmiColors[ lCounter ].rgbBlue  = 0;
	   lBitmapInfo->bmiColors[ lCounter ].rgbReserved = 0;
	   };
	 */

	AVIStreamSetFormat(gCaptureStream, 0, lBitmapInfo, lInfoSize);

	delete[](char *) lBitmapInfo;
}

void CloseCapture()
{
	if(gCaptureStream != NULL) {
		AVIStreamRelease(gCaptureStream);
		AVIFileRelease(gCaptureFile);

		gCaptureStream = NULL;
		gCaptureFile = NULL;

		AVIFileExit();

	}
}

void CaptureScreen(MR_VideoBuffer * pVideoBuffer)
{

	if(gCaptureStream != NULL) {
		// Create a ttemporaary buffer for the image
		int lXRes = pVideoBuffer->GetXRes();
		int lYRes = pVideoBuffer->GetYRes();

		MR_UInt8 *lBuffer = new MR_UInt8[lXRes * lYRes];

		MR_UInt8 *lDest = lBuffer;

		for(int lY = 0; lY < lYRes; lY++) {
			MR_UInt8 *lSrc = pVideoBuffer->GetBuffer() + (lYRes - lY - 1) * pVideoBuffer->GetLineLen();

			for(int lX = 0; lX < lXRes; lX++) {
				*lDest = *lSrc - MR_RESERVED_COLORS;
				lDest++;
				lSrc++;
			}
		}

		// Append the bitmap to the AVI file
		AVIStreamWrite(gCaptureStream, gCaptureFrameNo++, 1, lBuffer, lXRes * lYRes, AVIIF_KEYFRAME, NULL, NULL);

		delete[]lBuffer;

	}
}
#endif
