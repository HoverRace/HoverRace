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

#include "GameApp.h"
#include "resource.h"
#include "CommonDialog.h"
#include "NetworkSession.h"
#include "TrackSelect.h"
#include "Sponsor.h"
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

// global registration variables
static BOOL gKeyEncriptFilled = FALSE;
static BOOL gKeyFilled = FALSE;
static KeyStructure gKeyEncript;
static KeyStructure gKey;
static char gKeyPassword[20];

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
BOOL gFirstKDBResetJoy1 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy2 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy3 = TRUE;					  // Set to TRUE on each new scan
BOOL gFirstKDBResetJoy4 = TRUE;					  // Set to TRUE on each new scan

BOOL CheckKeyState(int pKeyIndex);

BOOL CheckKeyState(int pKeyIndex)
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

				/*
				   TRACE( "Joy %d %d %d %d %d %d \n",
				   lJoystick1.dwXpos, lJoystick1.dwYpos,
				   lJoystick1.dwButtons&1,
				   lJoystick1.dwButtons&2,
				   lJoystick1.dwButtons&4,
				   lJoystick1.dwButtons&8              );
				 */
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

MR_GameApp *MR_GameApp::This;

// Local prototypes
static HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE FAR * lphPalette);
static HPALETTE CreateDIBPalette(LPBITMAPINFO lpbmi, LPINT lpiNumColors);

// class MR_GameThread

unsigned long MR_GameThread::Loop(LPVOID pThread)
{
	MR_GameThread *lThis = (MR_GameThread *) pThread;

	while(true) {
		EnterCriticalSection(&lThis->mMutex);

		if(lThis->mTerminate)
			break;

		ASSERT(lThis->mGameApp->mCurrentSession != NULL);

		// Game processing
		MR_SAMPLE_START(ReadInputs, "ReadInputs");
		lThis->mGameApp->ReadAsyncInputController();
		MR_SAMPLE_END(ReadInputs);
		MR_SAMPLE_START(Process, "Process");

#ifdef MR_AVI_CAPTURE
		lThis->mGameApp->mCurrentSession->Process(1000 / gCaptureFrameRate);
#else
		lThis->mGameApp->mCurrentSession->Process();
#endif

		MR_SAMPLE_END(Process);
		MR_SAMPLE_START(Refresh, "Refresh");
		lThis->mGameApp->RefreshView();
		lThis->mGameApp->mNbFrames++;
		MR_SAMPLE_END(Refresh);

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
	mAccelerators = NULL;
	mVideoBuffer = NULL;
	mObserver1 = NULL;
	mObserver2 = NULL;
	mObserver3 = NULL;
	mObserver4 = NULL;
	mCurrentSession = NULL;
	mGameThread = NULL;

	mCurrentMode = e3DView;

	mClrScrTodo = 2;

	mPaletteChangeAllowed = TRUE;

	//TODO: If LoadRegistry found registry entries, then ask to migrate later.
	LoadRegistry();
	
	// Load the configuration, using the default OS-specific path.
	MR_Config::Init();
	MR_Config *cfg = MR_Config::GetInstance();
	OutputDebugString("Loaded config.\n");

	mServerHasChanged = FALSE;
}

MR_GameApp::~MR_GameApp()
{
	Clean();
	MR_DllObjectFactory::Clean(FALSE);
	MR_SoundServer::Close();
	delete mVideoBuffer;

	MR_Config::Shutdown();
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

}

void MR_GameApp::LoadRegistry()
{
	mDisplayFirstScreen = FALSE;
	mIntroMovie = FALSE;

	// Built-in defaults
	// Controls
	mMotorOn1 = 1;
	mRight1 = 5;
	mLeft1 = 6;
	mJump1 = 3;
	mFire1 = 2;
	mBreak1 = 4;
	mWeapon1 = 11;
	mLookBack1 = 10;

	mMotorOn2 = 66;
	mRight2 = 64;
	mLeft2 = 61;
	mJump2 = 83;
	mFire2 = 78;
	mBreak2 = 79;
	mWeapon2 = 77;
	mLookBack2 = 65;

	mMotorOn3 = 0;
	mRight3 = 0;
	mLeft3 = 0;
	mJump3 = 0;
	mFire3 = 0;
	mBreak3 = 0;
	mWeapon3 = 0;
	mLookBack3 = 0;

	mMotorOn4 = 0;
	mRight4 = 0;
	mLeft4 = 0;
	mJump4 = 0;
	mFire4 = 0;
	mBreak4 = 0;
	mWeapon4 = 0;
	mLookBack4 = 0;

	// Screen
	mGamma = 1.2;
	mContrast = 0.95;
	mBrightness = 0.95;

	// Nickname
	char lBuffer[80];
	DWORD lBufferSize = sizeof(lBuffer);

	if(GetUserName(lBuffer, &lBufferSize)) {
		lBuffer[30] = 0;
		mNickName = lBuffer;
	} else
	mNickName.LoadString(IDS_DEFAULT_ALIAS);

	// Registration info
	mMajorID = -1;
	mMinorID = -1;

	mMainServer = "66.197.183.245/~sirbrock/imr/rl.php";


	// Prerecorded messages
	// Joystick stuff

	// Now verify in the registry if this information can not be retrieved
	HKEY lProgramKey;

	int lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\HoverRace.com\\HoverRace",
	// "SOFTWARE",
	// NULL,
		0,
		KEY_EXECUTE,
		&lProgramKey);

	// if(lError == ERROR_SUCCESS) {
	MR_UInt8 lControlBuffer[32];
	DWORD lControlBufferSize = sizeof(lControlBuffer);

	if(RegQueryValueEx(lProgramKey, "Control", 0, NULL, lControlBuffer, &lControlBufferSize) == ERROR_SUCCESS) {
		// mLookBack1 and mLookBack2 added 12/1/2006
		mMotorOn1 = lControlBuffer[0];
		mRight1 = lControlBuffer[1];
		mLeft1 = lControlBuffer[2];
		mJump1 = lControlBuffer[3];
		mFire1 = lControlBuffer[4];
		mBreak1 = lControlBuffer[5];
		mWeapon1 = lControlBuffer[6];
		mLookBack1 = lControlBuffer[7];

		mMotorOn2 = lControlBuffer[8];
		mRight2 = lControlBuffer[9];
		mLeft2 = lControlBuffer[10];
		mJump2 = lControlBuffer[11];
		mFire2 = lControlBuffer[12];
		mBreak2 = lControlBuffer[13];
		mWeapon2 = lControlBuffer[14];
		mLookBack2 = lControlBuffer[15];

		mMotorOn3 = lControlBuffer[16];
		mRight3 = lControlBuffer[17];
		mLeft3 = lControlBuffer[18];
		mJump3 = lControlBuffer[19];
		mFire3 = lControlBuffer[20];
		mBreak3 = lControlBuffer[21];
		mWeapon3 = lControlBuffer[22];
		mLookBack3 = lControlBuffer[23];

		mMotorOn4 = lControlBuffer[24];
		mRight4 = lControlBuffer[25];
		mLeft4 = lControlBuffer[26];
		mJump4 = lControlBuffer[27];
		mFire4 = lControlBuffer[28];
		mBreak4 = lControlBuffer[29];
		mWeapon4 = lControlBuffer[30];
		mLookBack4 = lControlBuffer[31];
	}

	double lVideoSetting[3];
	DWORD lVideoSettingSize = sizeof(lVideoSetting);

	if(RegQueryValueEx(lProgramKey, "VideoColors", 0, NULL, (MR_UInt8 *) lVideoSetting, &lVideoSettingSize) == ERROR_SUCCESS) {
		mGamma = lVideoSetting[0];
		mContrast = lVideoSetting[1];
		mBrightness = lVideoSetting[2];
	}

	lBufferSize = sizeof(lBuffer);
	if(RegQueryValueEx(lProgramKey, "Alias", 0, NULL, (MR_UInt8 *) lBuffer, &lBufferSize) == ERROR_SUCCESS)
		mNickName = lBuffer;

	lBufferSize = sizeof(lBuffer);
	if(RegQueryValueEx(lProgramKey, "Owner", 0, NULL, (MR_UInt8 *) lBuffer, &lBufferSize) == ERROR_SUCCESS)
		mOwner = lBuffer;
#ifdef _DEMO_
	else
		mOwner = "Demo Key";
#endif

	lBufferSize = sizeof(mDisplayFirstScreen);
	if(RegQueryValueEx(lProgramKey, "DisplayFirstScreen", 0, NULL, (MR_UInt8 *) & mDisplayFirstScreen, &lBufferSize) != ERROR_SUCCESS)
		mDisplayFirstScreen = FALSE;

	lBufferSize = sizeof(mIntroMovie);
	if(RegQueryValueEx(lProgramKey, "IntroMovie", 0, NULL, (MR_UInt8 *) & mIntroMovie, &lBufferSize) != ERROR_SUCCESS)
		mIntroMovie = FALSE;

	lBufferSize = sizeof(mNativeBppFullscreen);
	if(RegQueryValueEx(lProgramKey, "NativeBppFullscreen", 0, NULL, (MR_UInt8 *) &mNativeBppFullscreen, &lBufferSize) != ERROR_SUCCESS)
		mNativeBppFullscreen = FALSE;

	lBufferSize = sizeof(lBuffer);
	if(RegQueryValueEx(lProgramKey, "Company", 0, NULL, (MR_UInt8 *) lBuffer, &lBufferSize) == ERROR_SUCCESS)
		mCompany = lBuffer;

	int lID[3];
	DWORD lIDSize = sizeof(lID);

	if(RegQueryValueEx(lProgramKey, "RegistrationID", 0, NULL, (MR_UInt8 *) lID, &lIDSize) == ERROR_SUCCESS) {
		mMajorID = lID[0];
		mMinorID = lID[1];
	}
#ifdef _DEMO_
	else {
		mMajorID = 1;
		mMinorID = 1138;
	}
#endif

	lBufferSize = sizeof(gKeyEncript);
	if(RegQueryValueEx(lProgramKey, "Key", 0, NULL, (MR_UInt8 *) & gKeyEncript, &lBufferSize) == ERROR_SUCCESS)
		gKeyEncriptFilled = TRUE;
#ifdef _DEMO_
	else {
		static unsigned char sKeyBuffer[20] = {
			0x57, 0xCC, 0x3D, 0x21, 0xCC,
			0x20, 0xD7, 0x34, 0x62, 0x01,
			0x50, 0x0E, 0x65, 0x90, 0xE9,
			0x9B, 0x39, 0x70, 0x5B, 0x99
		};
		memcpy(&gKeyEncript, sKeyBuffer, sizeof(sKeyBuffer));
		gKeyEncriptFilled = TRUE;
	}
#endif

	// Get the address of the server (we need a larger buffer)
	{
		char  lServerBuffer[500];
		DWORD lServerBufferSize = sizeof(lServerBuffer);
		if(RegQueryValueEx(lProgramKey, "MainServer", 0, NULL, (MR_UInt8 *) lServerBuffer, &lServerBufferSize) == ERROR_SUCCESS) {
			mMainServer = lServerBuffer;
		}
	}
}

void MR_GameApp::SaveRegistry()
{
	BOOL lReturnValue = TRUE;
	HKEY lProgramKey;

	/*
	   int lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
	   "SOFTWARE\\HoverRace.com\\HoverRace",
	   0,
	   KEY_WRITE,
	   &lProgramKey          );
	 */

	DWORD lDummy;
	int lError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\HoverRace.com\\HoverRace",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&lProgramKey,
		&lDummy);

	if(lError == ERROR_SUCCESS) {
		MR_UInt8 lControlBuffer[32];

		lControlBuffer[0] = (MR_UInt8) mMotorOn1;
		lControlBuffer[1] = (MR_UInt8) mRight1;
		lControlBuffer[2] = (MR_UInt8) mLeft1;
		lControlBuffer[3] = (MR_UInt8) mJump1;
		lControlBuffer[4] = (MR_UInt8) mFire1;
		lControlBuffer[5] = (MR_UInt8) mBreak1;
		lControlBuffer[6] = (MR_UInt8) mWeapon1;
		lControlBuffer[7] = (MR_UInt8) mLookBack1;

		lControlBuffer[8] = (MR_UInt8) mMotorOn2;
		lControlBuffer[9] = (MR_UInt8) mRight2;
		lControlBuffer[10] = (MR_UInt8) mLeft2;
		lControlBuffer[11] = (MR_UInt8) mJump2;
		lControlBuffer[12] = (MR_UInt8) mFire2;
		lControlBuffer[13] = (MR_UInt8) mBreak2;
		lControlBuffer[14] = (MR_UInt8) mWeapon2;
		lControlBuffer[15] = (MR_UInt8) mLookBack2;

		lControlBuffer[16] = (MR_UInt8) mMotorOn3;
		lControlBuffer[17] = (MR_UInt8) mRight3;
		lControlBuffer[18] = (MR_UInt8) mLeft3;
		lControlBuffer[19] = (MR_UInt8) mJump3;
		lControlBuffer[20] = (MR_UInt8) mFire3;
		lControlBuffer[21] = (MR_UInt8) mBreak3;
		lControlBuffer[22] = (MR_UInt8) mWeapon3;
		lControlBuffer[23] = (MR_UInt8) mLookBack3;

		lControlBuffer[24] = (MR_UInt8) mMotorOn4;
		lControlBuffer[25] = (MR_UInt8) mRight4;
		lControlBuffer[26] = (MR_UInt8) mLeft4;
		lControlBuffer[27] = (MR_UInt8) mJump4;
		lControlBuffer[28] = (MR_UInt8) mFire4;
		lControlBuffer[29] = (MR_UInt8) mBreak4;
		lControlBuffer[30] = (MR_UInt8) mWeapon4;
		lControlBuffer[31] = (MR_UInt8) mLookBack4;

		if(RegSetValueEx(lProgramKey, "Control", 0, REG_BINARY, lControlBuffer, sizeof(lControlBuffer)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		double lVideoSetting[3];

		lVideoSetting[0] = mGamma;
		lVideoSetting[1] = mContrast;
		lVideoSetting[2] = mBrightness;

		if(RegSetValueEx(lProgramKey, "VideoColors", 0, REG_BINARY, (MR_UInt8 *) lVideoSetting, sizeof(lVideoSetting)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "Alias", 0, REG_SZ, (const unsigned char *) (const char *) mNickName, mNickName.GetLength() + 1) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "DisplayFirstScreen", 0, REG_BINARY, (MR_UInt8 *) & mDisplayFirstScreen, sizeof(mDisplayFirstScreen)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "IntroMovie", 0, REG_BINARY, (MR_UInt8 *) & mIntroMovie, sizeof(mIntroMovie)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "NativeBppFullscreen", 0, REG_BINARY, (MR_UInt8 *) &mNativeBppFullscreen, sizeof(mNativeBppFullscreen)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "Owner", 0, REG_SZ, (const unsigned char *) (const char *) mOwner, mOwner.GetLength() + 1) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(RegSetValueEx(lProgramKey, "Company", 0, REG_SZ, (const unsigned char *) (const char *) mCompany, mCompany.GetLength() + 1) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		int lID[2];

		lID[0] = mMajorID;
		lID[1] = mMinorID;

		if(RegSetValueEx(lProgramKey, "RegistrationID", 0, REG_BINARY, (MR_UInt8 *) lID, sizeof(lID)) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}

		if(gKeyEncriptFilled) {
			if(RegSetValueEx(lProgramKey, "Key", 0, REG_BINARY, (const unsigned char *) &gKeyEncript, sizeof(gKeyEncript)) != ERROR_SUCCESS) {
				lReturnValue = FALSE;
				ASSERT(FALSE);
			}
		}

		// Save the main server key
		if(RegSetValueEx(lProgramKey, "MainServer", 0, REG_SZ, (const unsigned char *) (const char *) mMainServer, mMainServer.GetLength() + 1) != ERROR_SUCCESS) {
			lReturnValue = FALSE;
			ASSERT(FALSE);
		}
	}

	MR_Config::GetInstance()->Save();
}

BOOL MR_GameApp::DisplayNotice()
{
	return (DialogBox(mInstance, MAKEINTRESOURCE(IDD_LEGAL_NOT), NULL, NoticeDlgFunc) == IDOK);
}

BOOL MR_GameApp::DisplayLoginWindow()
{
	// The demo/registered versions are no longer necessary, as HoverRace is
	// now free.  Therefore we just tell the game that we're registered.
	return TRUE;

	// the old function, no longer necessary
	//BOOL lReturnValue = FALSE;
	//DisplaySponsorWindow(NULL);
	//if(gKeyEncriptFilled) {
	//   gKey = gKeyEncript;
	//   lReturnValue = (DialogBox(mInstance, MAKEINTRESOURCE(IDD_LOGIN_PASSWD), NULL, LoginPasswdFunc) != IDCANCEL );
	//}
	//else {
	//   lReturnValue = (DialogBox(mInstance, MAKEINTRESOURCE(IDD_LOGIN), NULL, LoginFunc) != IDCANCEL);
	//}
	//return lReturnValue;
}

void MR_GameApp::DisplayRegistrationInfo(HWND pWindow)
{
	BOOL lDisplayIt = TRUE;

	// first return to window mode
	SetVideoMode(0, 0);

	gKeyPassword[0] = 0;

	if(gKeyEncriptFilled) {
		if(DialogBox(mInstance, MAKEINTRESOURCE(IDD_REGKEY_PASSWD), pWindow, RegistrationPasswdFunc) == IDCANCEL) {
			lDisplayIt = FALSE;
		}
	}

	if(lDisplayIt) {
		DialogBox(mInstance, MAKEINTRESOURCE(IDD_REGISTER), pWindow, RegistrationFunc);
	}
	gKeyPassword[0] = 0;

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

void MR_GameApp::DisplayRegistrationSite()
{

	// first return to window mode
	SetVideoMode(0, 0);

	LoadURLShortcut(mMainWindow, MR_LoadString(IDS_REGSITE));

}

void MR_GameApp::DisplayAbout()
{
	SetVideoMode(0, 0);
	DialogBox(mInstance, MAKEINTRESOURCE(IDD_ABOUT), mMainWindow, AboutDlgFunc);
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

	// attempt to make the main window
	mMainWindow = CreateWindowEx(WS_EX_APPWINDOW, MR_APP_CLASS_NAME, MR_LoadString(IDS_CAPTION), (WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_EX_CLIENTEDGE) & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, NULL, NULL, mInstance, NULL);

	if(mMainWindow == NULL)
		lReturnValue = FALSE;					  // making of window failed
	else {
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

BOOL MR_GameApp::InitGame()
{
	BOOL lReturnValue = TRUE;

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
		mVideoBuffer = new MR_VideoBuffer(mMainWindow, mGamma, mContrast, mBrightness);
		mVideoBuffer->SetNativeBppFullscreen(mNativeBppFullscreen);
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
	if(lReturnValue && mIntroMovie == FALSE) {
		mMovieWnd = MCIWndCreate(mMainWindow, mInstance, WS_CHILD | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, "Intro.avi");
		MCIWndPlay(mMovieWnd);
		// the function of this is currently unknown but it was not being used
		/* CreateDialog( mInstance,
		   MAKEINTRESOURCE( IDD_BACK_ANIM ),
		   mMainWindow,
		   MovieDialogFunc ); */
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
	if(lReturnValue && mDisplayFirstScreen == FALSE) {
		if(DialogBox(mInstance, MAKEINTRESOURCE(IDD_FIRST_CHOICE), mMainWindow, FirstChoiceDialogFunc) == IDOK)
			SendMessage(mMainWindow, WM_COMMAND, ID_GAME_NETWORK_INTERNET, 0);
	}
	gKeyFilled = TRUE;

	return lReturnValue;
}

void MR_GameApp::RefreshView()
{
	static int lColor = 0;

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
			mVideoBuffer->Unlock();
		}
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

void MR_GameApp::ReadAsyncInputController()
{
	gFirstKDBResetJoy1 = TRUE;
	gFirstKDBResetJoy2 = TRUE;

	if(mCurrentSession != NULL) {
#ifdef _DEBUG
		if(GetForegroundWindow() == mMainWindow)
#endif
		{
			static BOOL lFirstCall = TRUE;
			int lControlState1 = 0;
			int lControlState2 = 0;
			int lControlState3 = 0;
			int lControlState4 = 0;

			if(CheckKeyState(mMotorOn1))
				lControlState1 |= MR_MainCharacter::eMotorOn;
			if(CheckKeyState(mJump1))
				lControlState1 |= MR_MainCharacter::eJump;
			if(CheckKeyState(mBreak1))
				lControlState1 |= MR_MainCharacter::eBreakDirection;
			if(CheckKeyState(mFire1))
				lControlState1 |= MR_MainCharacter::eFire;
			if(CheckKeyState(mWeapon1))
				lControlState1 |= MR_MainCharacter::eSelectWeapon;
			if(CheckKeyState(mLookBack1))
				lControlState1 |= MR_MainCharacter::eLookBack;

			/*
			   if( GetAsyncKeyState( KeyChoice[ mStraffle1 ].mKeyValue ) )
			   {
			   if( GetAsyncKeyState( KeyChoice[ mRight1 ].mKeyValue ) )
			   {
			   lControlState1 |= MR_MainCharacter::eStraffleRight;
			   }
			   if( GetAsyncKeyState( KeyChoice[ mLeft1 ].mKeyValue ) )
			   {
			   lControlState1 |= MR_MainCharacter::eStraffleLeft;
			   }
			   }
			   else
			 */
			if(CheckKeyState(mRight1))
				lControlState1 |= MR_MainCharacter::eRight;
			if(CheckKeyState(mLeft1))
				lControlState1 |= MR_MainCharacter::eLeft;

			// If we're in two player mode we need to check those keys too
			if(mCurrentSession->GetMainCharacter2() != NULL) {
				if(CheckKeyState(mMotorOn2))
					lControlState2 |= MR_MainCharacter::eMotorOn;
				if(CheckKeyState(mJump2))
					lControlState2 |= MR_MainCharacter::eJump;
				if(CheckKeyState(mBreak2))
					lControlState2 |= MR_MainCharacter::eBreakDirection;
				if(CheckKeyState(mFire2))
					lControlState2 |= MR_MainCharacter::eFire;
				if(CheckKeyState(mWeapon2))
					lControlState2 |= MR_MainCharacter::eSelectWeapon;
				if(CheckKeyState(mLookBack2))
					lControlState2 |= MR_MainCharacter::eLookBack;

				/*
				   if( GetAsyncKeyState( KeyChoice[ mStraffle2 ].mKeyValue ) )
				   {
				   if( GetAsyncKeyState( KeyChoice[ mRight2 ].mKeyValue ) )
				   {
				   lControlState2 |= MR_MainCharacter::eStraffleRight;
				   }
				   if( GetAsyncKeyState( KeyChoice[ mLeft2 ].mKeyValue ) )
				   {
				   lControlState2 |= MR_MainCharacter::eStraffleLeft;
				   }
				   }
				   else
				 */
				if(CheckKeyState(mRight2))
					lControlState2 |= MR_MainCharacter::eRight;
				if(CheckKeyState(mLeft2))
					lControlState2 |= MR_MainCharacter::eLeft;

			}
			// If we're in three player mode we need to check those keys too
			if(mCurrentSession->GetMainCharacter3() != NULL) {
				if(CheckKeyState(mMotorOn3))
					lControlState3 |= MR_MainCharacter::eMotorOn;
				if(CheckKeyState(mJump3))
					lControlState3 |= MR_MainCharacter::eJump;
				if(CheckKeyState(mBreak3))
					lControlState3 |= MR_MainCharacter::eBreakDirection;
				if(CheckKeyState(mFire3))
					lControlState3 |= MR_MainCharacter::eFire;
				if(CheckKeyState(mWeapon3))
					lControlState3 |= MR_MainCharacter::eSelectWeapon;
				if(CheckKeyState(mLookBack3))
					lControlState3 |= MR_MainCharacter::eLookBack;

				/*
				   if( GetAsyncKeyState( KeyChoice[ mStraffle3 ].mKeyValue ) )
				   {
				   if( GetAsyncKeyState( KeyChoice[ mRight3 ].mKeyValue ) )
				   {
				   lControlState3 |= MR_MainCharacter::eStraffleRight;
				   }
				   if( GetAsyncKeyState( KeyChoice[ mLeft3 ].mKeyValue ) )
				   {
				   lControlState3 |= MR_MainCharacter::eStraffleLeft;
				   }
				   }
				   else
				 */
				if(CheckKeyState(mRight3))
					lControlState3 |= MR_MainCharacter::eRight;
				if(CheckKeyState(mLeft3))
					lControlState3 |= MR_MainCharacter::eLeft;

			}
			// If we're in four player mode we need to check those keys too
			if(mCurrentSession->GetMainCharacter4() != NULL) {
				if(CheckKeyState(mMotorOn4))
					lControlState4 |= MR_MainCharacter::eMotorOn;
				if(CheckKeyState(mJump4))
					lControlState4 |= MR_MainCharacter::eJump;
				if(CheckKeyState(mBreak4))
					lControlState4 |= MR_MainCharacter::eBreakDirection;
				if(CheckKeyState(mFire4))
					lControlState4 |= MR_MainCharacter::eFire;
				if(CheckKeyState(mWeapon4))
					lControlState4 |= MR_MainCharacter::eSelectWeapon;
				if(CheckKeyState(mLookBack4))
					lControlState4 |= MR_MainCharacter::eLookBack;

				/*
				   if( GetAsyncKeyState( KeyChoice[ mStraffle4 ].mKeyValue ) )
				   {
				   if( GetAsyncKeyState( KeyChoice[ mRight4 ].mKeyValue ) )
				   {
				   lControlState4 |= MR_MainCharacter::eStraffleRight;
				   }
				   if( GetAsyncKeyState( KeyChoice[ mLeft4 ].mKeyValue ) )
				   {
				   lControlState4 |= MR_MainCharacter::eStraffleLeft;
				   }
				   }
				   else
				 */
				if(CheckKeyState(mRight4))
					lControlState4 |= MR_MainCharacter::eRight;
				if(CheckKeyState(mLeft4))
					lControlState4 |= MR_MainCharacter::eLeft;

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
			lSuccess = mVideoBuffer->SetVideoMode();

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

	if(!IsIconic(mMainWindow)) {
		if(GetClientRect(mMainWindow, &lClientRect)) {

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
				SetWindowPos(mMovieWnd, HWND_TOP, (lClientRect.right - (lMovieRect.right - lMovieRect.left)) / 2, (lClientRect.bottom - (lMovieRect.bottom - lMovieRect.top)) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

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

//void MR_GameApp::EnterMenuLoop()
//{
//   mMenuStack++;
//   if( /*(mMenuStack==1)&&*/(mVideoBuffer != NULL)&&!mVideoBuffer->IsWindowMode() )
//   {
//      PauseGameThread();
//      mClrScrTodo = 2;
//      mVideoBuffer->PushMenuMode();
//      AssignPalette();
//      RestartGameThread();
//   }
//}

//void MR_GameApp::EndMenuLoop()
//{
//   mMenuStack--;
//   if( (mMenuStack<=0)&&(mVideoBuffer != NULL)&&!mVideoBuffer->IsWindowMode() )
//   {
//      mMenuStack = 0;
//      PauseGameThread();
//      mClrScrTodo = 2;
//      mVideoBuffer->PopMenuMode();
//      AssignPalette();
//      RestartGameThread();
//   }
//}

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
	BOOL lSuccess = TRUE;

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	// Prompt the user for a track name
	CString lCurrentTrack;
	int lNbLap;
	BOOL lAllowWeapons;

	lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap, lAllowWeapons);

	if(lSuccess) {
		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);
		mObserver1 = MR_Observer::New();

		// Create the new session
		MR_ClientSession *lCurrentSession = new MR_ClientSession();

		// Load the selected track
		if(lSuccess) {
			MR_RecordFile *lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack);
			lSuccess = lCurrentSession->LoadNew(lCurrentTrack, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
		}
		// Create the main character
		if(lSuccess)
			lCurrentSession->SetSimulationTime(-6000);

		// Create the main character
		if(lSuccess)
			lSuccess = lCurrentSession->CreateMainCharacter();

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
	BOOL lSuccess = TRUE;

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	// Prompt the user for a maze name
	CString lCurrentTrack;
	int lNbLap;
	BOOL lAllowWeapons;

	lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap, lAllowWeapons);

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
			MR_RecordFile *lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack);
			lSuccess = lCurrentSession->LoadNew(lCurrentTrack, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
		}

		if(lSuccess) {
			lCurrentSession->SetSimulationTime(-8000);

			// Create the main character2
			lSuccess = lCurrentSession->CreateMainCharacter();
		}

		if(lSuccess) {
			lSuccess = lCurrentSession->CreateMainCharacter2();
			if(pSplitPlayers > 2)
				lSuccess = lCurrentSession->CreateMainCharacter3();
			if(pSplitPlayers > 3)
				lSuccess = lCurrentSession->CreateMainCharacter4();
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
	BOOL lSuccess = TRUE;
	MR_NetworkSession *lCurrentSession = NULL;
	MR_Config *cfg = MR_Config::GetInstance();

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();

	CString lCurrentTrack;
	int lNbLap;
	BOOL lAllowWeapons;
	// Prompt the user for a maze name fbm extensions

	if(pServer) {
		lSuccess = MR_SelectTrack(mMainWindow, lCurrentTrack, lNbLap, lAllowWeapons);

		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);
		lCurrentSession = new MR_NetworkSession(FALSE, gKeyFilled ? mMajorID : -1, gKeyFilled ? mMinorID : -1, mMainWindow);
	}
	else {
		DeleteMovieWnd();
		MR_SoundServer::Init(mMainWindow);

		lCurrentSession = new MR_NetworkSession(FALSE, gKeyFilled ? mMajorID : -1, gKeyFilled ? mMinorID : -1, mMainWindow);
		lCurrentSession->SetPlayerName(mNickName);

		lSuccess = lCurrentSession->PreConnectToServer(mMainWindow, lCurrentTrack);

		if(mNickName != lCurrentSession->GetPlayerName()) {
			cfg->player.nickName = mNickName = lCurrentSession->GetPlayerName();
			SaveRegistry();
		}
		// Extract the lap count from the track name and weapon use
		// From the end of the string find the two last space
		int lSpaceCount = 0;

		lNbLap = 5;								  // Default
		lAllowWeapons = FALSE;

		for(int lCounter = lCurrentTrack.GetLength() - 1; lCounter >= 0; lCounter--) {
			if(lCurrentTrack[lCounter] == ' ') {
				lSpaceCount++;

				if(lSpaceCount == 2)
					lAllowWeapons = strncmp(((const char *) lCurrentTrack) + lCounter + 1, "no", 2);

				if(lSpaceCount == 4) {
					lNbLap = atoi(((const char *) lCurrentTrack) + lCounter + 1);

					if(lNbLap < 1)
						lNbLap = 5;

					lCurrentTrack = CString(lCurrentTrack, lCounter);
					break;
				}
			}
		}
	}

	if(lSuccess) {
		mObserver1 = MR_Observer::New();

		// Load the track
		MR_RecordFile *lTrackFile = MR_TrackOpen(mMainWindow, lCurrentTrack);
		lSuccess = lCurrentSession->LoadNew(lCurrentTrack, lTrackFile, lNbLap, lAllowWeapons, mVideoBuffer);
	}

	if(lSuccess) {
		if(pServer) {
			CString lNameBuffer;

			lNameBuffer.Format("%s %d %s %s", (const char *) lCurrentTrack, lNbLap, lNbLap > 1 ? "laps" : "lap", lAllowWeapons ? "with weapons" : "no weapons");

			// Create a net server
			lCurrentSession->SetPlayerName(mNickName);

			lSuccess = lCurrentSession->WaitConnections(mMainWindow, lNameBuffer);
			if(mNickName != lCurrentSession->GetPlayerName()) {
				cfg->player.nickName = mNickName = lCurrentSession->GetPlayerName();
				SaveRegistry();
			}
		} else
		lSuccess = lCurrentSession->ConnectToServer(mMainWindow);
	}

	if(lSuccess) {
												  // start in 13 seconds
		lCurrentSession->SetSimulationTime(-13000);
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
	MR_InternetRoom lInternetRoom(gKeyFilled, gKeyFilled ? mMajorID : -1, gKeyFilled ? mMinorID : -1, gKeyFilled ? gKey.mIDSum : 0, 0, mMainServer);

	// Verify is user acknowledge
	if(AskUserToAbortGame() != IDOK)
		return;

	// Delete the current session
	Clean();
	DeleteMovieWnd();
	MR_SoundServer::Init(mMainWindow);

	lCurrentSession = new MR_NetworkSession(TRUE, gKeyFilled ? mMajorID : -1, gKeyFilled ? mMinorID : -1, mMainWindow);

	if(lSuccess) {
		lCurrentSession->SetPlayerName(mNickName);

		lSuccess = lInternetRoom.DisplayChatRoom(mMainWindow, lCurrentSession, mVideoBuffer, mServerHasChanged);
		mServerHasChanged = FALSE;

		if(mNickName != lCurrentSession->GetPlayerName()) {
			cfg->player.nickName = mNickName = lCurrentSession->GetPlayerName();
			SaveRegistry();
		}
	}

	if(lSuccess) {
												  // start in 20 seconds (this time may be readjusted by the server)
		lCurrentSession->SetSimulationTime(-20000);
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
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
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
	POINT lRes;
	if (GetDesktopResolution(&lRes)) {
		SetVideoMode(lRes.x, lRes.y);
	}
}

void MR_GameApp::UpdateMenuItems()
{
	MENUITEMINFO lMenuInfo;
	memset(&lMenuInfo, 0, sizeof(lMenuInfo));
	lMenuInfo.cbSize = sizeof(lMenuInfo);

	POINT lRes;
	if (GetDesktopResolution(&lRes)) {
		char s[256] = {0};
		int lLen = _snprintf(s, 255, "&0 Desktop (%dx%d)", lRes.x, lRes.y);
		if (lLen < 0) {
			strcpy(s, "&0 Desktop");
		}

		lMenuInfo.fMask = MIIM_STATE | MIIM_STRING;
		lMenuInfo.fState = MFS_ENABLED;
		lMenuInfo.dwTypeData = s;
		lMenuInfo.cch = lLen;
	} else {
		lMenuInfo.fMask = MIIM_STATE | MIIM_STRING;
		lMenuInfo.fState = MFS_GRAYED;
		lMenuInfo.dwTypeData = "&0 Desktop";
		lMenuInfo.cch = strlen(lMenuInfo.dwTypeData);
	}

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
			break;

		case WM_SIZE:
		case WM_MOVE:
			This->OnDisplayChange();
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
			This->SetVideoMode(0, 0);
			This->UpdateMenuItems();
			break;

			/*
			   case WM_EXITMENULOOP:
			   SetTimer( pWindow, MRM_EXIT_MENU_LOOP, 1000, NULL );
			   // This->EndMenuLoop();
			   break;
			 */

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

					/*
					   case MRM_EXIT_MENU_LOOP:
					   KillTimer( pWindow, MRM_EXIT_MENU_LOOP );
					   This->EndMenuLoop();
					   return 0;
					 */

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
			TRACE("WM_ACTIVATE %d\n", pWParam);
			if(pWParam && (This->mVideoBuffer != NULL) && (This->mMainWindow == GetForegroundWindow())) {
				if(!This->mVideoBuffer->IsModeSettingInProgress()) {
					if(This->mVideoBuffer->IsWindowMode()) {
						TRACE("SetMode\n");

						This->SetVideoMode(0, 0);
						This->AssignPalette();
						return 0;
					}
				}
			}

			break;

		case WM_SETCURSOR:
			if((This->mVideoBuffer != NULL) && !This->mVideoBuffer->IsWindowMode()) {
				SetCursor(NULL);
				return TRUE;
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
	
					case ID_GAME_REGISTER:
						This->DisplayRegistrationInfo(pWindow);
						break;

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
			if(This->IsGameRunning()) {
				This->SetVideoMode(0, 0);
				if(This->AskUserToAbortGame() != IDOK) {
					return 0;
				}
			}
			This->Clean();
			delete This->mVideoBuffer;
			This->mVideoBuffer = NULL;
			DestroyWindow(This->mMainWindow);
			return 0;

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

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			This->mVideoBuffer->GetPaletteAttrib(lOriginalGamma, lOriginalContrast, lOriginalBrightness);

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 200));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETPOS, TRUE, long (lOriginalGamma * 100));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETPOS, TRUE, long (lOriginalContrast * 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, long (lOriginalBrightness * 100));

			UpdateIntensityDialogLabels(pWindow);
			break;

		case WM_HSCROLL:
			{
				cfg->video.gamma = SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
				cfg->video.contrast = SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
				cfg->video.brightness = SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0) / 100.0;

				UpdateIntensityDialogLabels(pWindow);

				This->mVideoBuffer->CreatePalette(cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
				This->AssignPalette();
			}
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
					This->mVideoBuffer->CreatePalette(lOriginalGamma, lOriginalContrast, lOriginalBrightness);
					This->AssignPalette();
					break;

				case PSN_APPLY:
					This->mVideoBuffer->GetPaletteAttrib(This->mGamma, This->mContrast, This->mBrightness);
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

	BOOL lReturnValue = FALSE;
	int lCounter;
	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			// Initialize the lists
			for(lCounter = 0; lCounter < NB_KEY_PLAYER_1; lCounter++) {
				char lBuffer[50];

				LoadString(NULL, KeyChoice[lCounter].mStringId, lBuffer, sizeof(lBuffer));

				SendDlgItemMessage(pWindow, IDC_MOTOR_ON1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_RIGHT1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LEFT1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_JUMP1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_FIRE1, CB_ADDSTRING, 0, (LONG) lBuffer);
				// SendDlgItemMessage( pWindow, IDC_STRAFFLE1, CB_ADDSTRING, 0, (LONG)lBuffer );
				SendDlgItemMessage(pWindow, IDC_BREAK1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_SELWEAPON1, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LOOKBACK1, CB_ADDSTRING, 0, (LONG) lBuffer);
			}

			for(lCounter = 0; lCounter < NB_KEY_PLAYER_2; lCounter++) {
				char lBuffer[50];
				LoadString(NULL, KeyChoice[lCounter].mStringId, lBuffer, sizeof(lBuffer));

				SendDlgItemMessage(pWindow, IDC_MOTOR_ON2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_RIGHT2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LEFT2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_JUMP2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_FIRE2, CB_ADDSTRING, 0, (LONG) lBuffer);
				// SendDlgItemMessage( pWindow, IDC_STRAFFLE2, CB_ADDSTRING, 0, (LONG)lBuffer );
				SendDlgItemMessage(pWindow, IDC_BREAK2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_SELWEAPON2, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LOOKBACK2, CB_ADDSTRING, 0, (LONG) lBuffer);
			}

			for(lCounter = 0; lCounter < NB_KEY_PLAYER_3; lCounter++) {
				char lBuffer[50];
				LoadString(NULL, KeyChoice[lCounter].mStringId, lBuffer, sizeof(lBuffer));

				SendDlgItemMessage(pWindow, IDC_MOTOR_ON3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_RIGHT3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LEFT3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_JUMP3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_FIRE3, CB_ADDSTRING, 0, (LONG) lBuffer);
				// SendDlgItemMessage( pWindow, IDC_STRAFFLE3, CB_ADDSTRING, 0, (LONG)lBuffer );
				SendDlgItemMessage(pWindow, IDC_BREAK3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_SELWEAPON3, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LOOKBACK3, CB_ADDSTRING, 0, (LONG) lBuffer);
			}

			for(lCounter = 0; lCounter < NB_KEY_PLAYER_4; lCounter++) {
				char lBuffer[50];
				LoadString(NULL, KeyChoice[lCounter].mStringId, lBuffer, sizeof(lBuffer));

				SendDlgItemMessage(pWindow, IDC_MOTOR_ON4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_RIGHT4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LEFT4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_JUMP4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_FIRE4, CB_ADDSTRING, 0, (LONG) lBuffer);
				// SendDlgItemMessage( pWindow, IDC_STRAFFLE4, CB_ADDSTRING, 0, (LONG)lBuffer );
				SendDlgItemMessage(pWindow, IDC_BREAK4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_SELWEAPON4, CB_ADDSTRING, 0, (LONG) lBuffer);
				SendDlgItemMessage(pWindow, IDC_LOOKBACK4, CB_ADDSTRING, 0, (LONG) lBuffer);
			}

			SendDlgItemMessage(pWindow, IDC_MOTOR_ON1, CB_SETCURSEL, This->mMotorOn1, 0);
			SendDlgItemMessage(pWindow, IDC_RIGHT1, CB_SETCURSEL, This->mRight1, 0);
			SendDlgItemMessage(pWindow, IDC_LEFT1, CB_SETCURSEL, This->mLeft1, 0);
			SendDlgItemMessage(pWindow, IDC_JUMP1, CB_SETCURSEL, This->mJump1, 0);
			SendDlgItemMessage(pWindow, IDC_FIRE1, CB_SETCURSEL, This->mFire1, 0);
			// SendDlgItemMessage( pWindow, IDC_STRAFFLE1, CB_SETCURSEL, This->mStraffle1,0 );
			SendDlgItemMessage(pWindow, IDC_BREAK1, CB_SETCURSEL, This->mBreak1, 0);
			SendDlgItemMessage(pWindow, IDC_SELWEAPON1, CB_SETCURSEL, This->mWeapon1, 0);
			SendDlgItemMessage(pWindow, IDC_LOOKBACK1, CB_SETCURSEL, This->mLookBack1, 0);

			SendDlgItemMessage(pWindow, IDC_MOTOR_ON2, CB_SETCURSEL, This->mMotorOn2, 0);
			SendDlgItemMessage(pWindow, IDC_RIGHT2, CB_SETCURSEL, This->mRight2, 0);
			SendDlgItemMessage(pWindow, IDC_LEFT2, CB_SETCURSEL, This->mLeft2, 0);
			SendDlgItemMessage(pWindow, IDC_JUMP2, CB_SETCURSEL, This->mJump2, 0);
			SendDlgItemMessage(pWindow, IDC_FIRE2, CB_SETCURSEL, This->mFire2, 0);
			// SendDlgItemMessage( pWindow, IDC_STRAFFLE2, CB_SETCURSEL, This->mStraffle2,0 );
			SendDlgItemMessage(pWindow, IDC_BREAK2, CB_SETCURSEL, This->mBreak2, 0);
			SendDlgItemMessage(pWindow, IDC_SELWEAPON2, CB_SETCURSEL, This->mWeapon2, 0);
			SendDlgItemMessage(pWindow, IDC_LOOKBACK2, CB_SETCURSEL, This->mLookBack2, 0);

			SendDlgItemMessage(pWindow, IDC_MOTOR_ON3, CB_SETCURSEL, This->mMotorOn3, 0);
			SendDlgItemMessage(pWindow, IDC_RIGHT3, CB_SETCURSEL, This->mRight3, 0);
			SendDlgItemMessage(pWindow, IDC_LEFT3, CB_SETCURSEL, This->mLeft3, 0);
			SendDlgItemMessage(pWindow, IDC_JUMP3, CB_SETCURSEL, This->mJump3, 0);
			SendDlgItemMessage(pWindow, IDC_FIRE3, CB_SETCURSEL, This->mFire3, 0);
			// SendDlgItemMessage( pWindow, IDC_STRAFFLE3, CB_SETCURSEL, This->mStraffle3,0 );
			SendDlgItemMessage(pWindow, IDC_BREAK3, CB_SETCURSEL, This->mBreak3, 0);
			SendDlgItemMessage(pWindow, IDC_SELWEAPON3, CB_SETCURSEL, This->mWeapon3, 0);
			SendDlgItemMessage(pWindow, IDC_LOOKBACK3, CB_SETCURSEL, This->mLookBack3, 0);

			SendDlgItemMessage(pWindow, IDC_MOTOR_ON4, CB_SETCURSEL, This->mMotorOn4, 0);
			SendDlgItemMessage(pWindow, IDC_RIGHT4, CB_SETCURSEL, This->mRight4, 0);
			SendDlgItemMessage(pWindow, IDC_LEFT4, CB_SETCURSEL, This->mLeft4, 0);
			SendDlgItemMessage(pWindow, IDC_JUMP4, CB_SETCURSEL, This->mJump4, 0);
			SendDlgItemMessage(pWindow, IDC_FIRE4, CB_SETCURSEL, This->mFire4, 0);
			// SendDlgItemMessage( pWindow, IDC_STRAFFLE4, CB_SETCURSEL, This->mStraffle4,0 );
			SendDlgItemMessage(pWindow, IDC_BREAK4, CB_SETCURSEL, This->mBreak4, 0);
			SendDlgItemMessage(pWindow, IDC_SELWEAPON4, CB_SETCURSEL, This->mWeapon4, 0);
			SendDlgItemMessage(pWindow, IDC_LOOKBACK4, CB_SETCURSEL, This->mLookBack4, 0);

			break;
		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					cfg->controls[0].motorOn  = This->mMotorOn1 = SendDlgItemMessage(pWindow, IDC_MOTOR_ON1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].right    = This->mRight1 = SendDlgItemMessage(pWindow, IDC_RIGHT1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].left     = This->mLeft1 = SendDlgItemMessage(pWindow, IDC_LEFT1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].motorOn  = This->mJump1 = SendDlgItemMessage(pWindow, IDC_JUMP1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].fire     = This->mFire1 = SendDlgItemMessage(pWindow, IDC_FIRE1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].brake    = This->mBreak1 = SendDlgItemMessage(pWindow, IDC_BREAK1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].weapon   = This->mWeapon1 = SendDlgItemMessage(pWindow, IDC_SELWEAPON1, CB_GETCURSEL, 0, 0);
					cfg->controls[0].lookBack = This->mLookBack1 = SendDlgItemMessage(pWindow, IDC_LOOKBACK1, CB_GETCURSEL, 0, 0);

					cfg->controls[1].motorOn  = This->mMotorOn2 = SendDlgItemMessage(pWindow, IDC_MOTOR_ON2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].right    = This->mRight2 = SendDlgItemMessage(pWindow, IDC_RIGHT2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].left     = This->mLeft2 = SendDlgItemMessage(pWindow, IDC_LEFT2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].motorOn  = This->mJump2 = SendDlgItemMessage(pWindow, IDC_JUMP2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].fire     = This->mFire2 = SendDlgItemMessage(pWindow, IDC_FIRE2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].brake    = This->mBreak2 = SendDlgItemMessage(pWindow, IDC_BREAK2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].weapon   = This->mWeapon2 = SendDlgItemMessage(pWindow, IDC_SELWEAPON2, CB_GETCURSEL, 0, 0);
					cfg->controls[1].lookBack = This->mLookBack2 = SendDlgItemMessage(pWindow, IDC_LOOKBACK2, CB_GETCURSEL, 0, 0);

					cfg->controls[2].motorOn  = This->mMotorOn3 = SendDlgItemMessage(pWindow, IDC_MOTOR_ON3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].right    = This->mRight3 = SendDlgItemMessage(pWindow, IDC_RIGHT3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].left     = This->mLeft3 = SendDlgItemMessage(pWindow, IDC_LEFT3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].motorOn  = This->mJump3 = SendDlgItemMessage(pWindow, IDC_JUMP3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].fire     = This->mFire3 = SendDlgItemMessage(pWindow, IDC_FIRE3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].brake    = This->mBreak3 = SendDlgItemMessage(pWindow, IDC_BREAK3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].weapon   = This->mWeapon3 = SendDlgItemMessage(pWindow, IDC_SELWEAPON3, CB_GETCURSEL, 0, 0);
					cfg->controls[2].lookBack = This->mLookBack3 = SendDlgItemMessage(pWindow, IDC_LOOKBACK3, CB_GETCURSEL, 0, 0);

					cfg->controls[3].motorOn  = This->mMotorOn4 = SendDlgItemMessage(pWindow, IDC_MOTOR_ON4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].right    = This->mRight4 = SendDlgItemMessage(pWindow, IDC_RIGHT4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].left     = This->mLeft4 = SendDlgItemMessage(pWindow, IDC_LEFT4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].motorOn  = This->mJump4 = SendDlgItemMessage(pWindow, IDC_JUMP4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].fire     = This->mFire4 = SendDlgItemMessage(pWindow, IDC_FIRE4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].brake    = This->mBreak4 = SendDlgItemMessage(pWindow, IDC_BREAK4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].weapon   = This->mWeapon4 = SendDlgItemMessage(pWindow, IDC_SELWEAPON4, CB_GETCURSEL, 0, 0);
					cfg->controls[3].lookBack = This->mLookBack4 = SendDlgItemMessage(pWindow, IDC_LOOKBACK4, CB_GETCURSEL, 0, 0);

					This->SaveRegistry();
					break;
			}
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
			SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_SETCHECK, This->mIntroMovie, 0);
			SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_SETCHECK, This->mDisplayFirstScreen, 0);
			SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_SETCHECK, This->mNativeBppFullscreen, 0);

			SetDlgItemText(pWindow, IDC_MAINSERVER, This->mMainServer);

			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_APPLY:
					cfg->misc.introMovie = This->mIntroMovie = SendDlgItemMessage(pWindow, IDC_INTRO_MOVIE, BM_GETCHECK, 0, 0);
					cfg->misc.displayFirstScreen = This->mDisplayFirstScreen = SendDlgItemMessage(pWindow, IDC_SHOW_INTERNET, BM_GETCHECK, 0, 0);
					cfg->video.nativeBppFullscreen = This->mNativeBppFullscreen = SendDlgItemMessage(pWindow, IDC_NATIVE_BPP_FULLSCREEN, BM_GETCHECK, 0, 0);

					This->mVideoBuffer->SetNativeBppFullscreen(This->mNativeBppFullscreen);

					{
						char lBuffer[80];
						if(GetDlgItemText(pWindow, IDC_MAINSERVER, lBuffer, sizeof(lBuffer)) == 0)
							ASSERT(FALSE);

						cfg->net.mainServer = This->mMainServer = lBuffer;
						This->mServerHasChanged = TRUE;
					}

					This->SaveRegistry();
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

BOOL CALLBACK MR_GameApp::MovieDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	/*
	   switch( pMsgId )
	   {
	   // Catch environment modification events
	   case WM_INITDIALOG:
	   // Create a MCIWnd
	   HWND lMCIWindow = MCIWndCreate( pWindow, This->mInstance, WS_CHILD|WS_VISIBLE, "Intro.avi" );
	   // Load the Intro video
	   // Animate_Open( GetDlgItem( pWindow, IDC_BACK_ANIM ) , "Intro.avi" );
	   return TRUE;
	   }
	 */

	return FALSE;
}

BOOL CALLBACK MR_GameApp::FirstChoiceDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
	{
		BOOL lReturnValue = FALSE;
	
		switch (pMsgId) {
			// Catch environment modification events
			case WM_INITDIALOG:
				CheckDlgButton(pWindow, IDC_CHECK, BST_CHECKED);
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

BOOL CALLBACK MR_GameApp::NoticeDlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	static HBITMAP lBitmap = NULL;
	static HPALETTE lPalette = NULL;
	static time_t lInitTime = 0;

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events

		case WM_INITDIALOG:
			{
				lInitTime = time(NULL);
	
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);
	
				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);
	
					lBitmap = LoadResourceBitmap(This->mInstance, MAKEINTRESOURCE(IDB_LEGAL_NOT), &lPalette);
	
					ASSERT(lBitmap != NULL);
	
					SelectPalette(hdc, lPalette, FALSE);
					RealizePalette(hdc);
	
					HANDLE lOldHandle = (HANDLE) SendMessage(lBitmapCtl, STM_SETIMAGE, IMAGE_BITMAP, (long) lBitmap);
				}
				else {
					lBitmap = NULL;
					lPalette = NULL;
				}
				lReturnValue = TRUE;
			}
			break;

		case WM_DESTROY:
			if(lBitmap != NULL) {
				DeleteObject(lBitmap);
				lBitmap = NULL;
			}

			if(lPalette != NULL) {
				DeleteObject(lPalette);
				lPalette = NULL;
			}

		case WM_QUERYNEWPALETTE:
			if(lPalette != NULL) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);

				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);

					SelectPalette(hdc, lPalette, FALSE);
					RealizePalette(hdc);
				}
				lReturnValue = TRUE;
			}
			break;

		case WM_PALETTECHANGED:
			if((lPalette != NULL) && ((HWND) pWParam != pWindow)) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);

				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);

					SelectPalette(hdc, lPalette, TRUE);
					RealizePalette(hdc);
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
					struct tm *lTime;

					lTime = localtime(&lInitTime);

					if((lInitTime + 1) > time(NULL)) {
						MessageBox(pWindow, MR_LoadString(IDS_READ_NOTICE), MR_LoadString(IDS_GAME_NAME), MB_ICONWARNING | MB_APPLMODAL | MB_OK);
					} else if(!(((lTime->tm_year == 96) && (lTime->tm_mon == 11)) || ((lTime->tm_year == 97) && (lTime->tm_mon == 0))
						)
					) {

						MessageBox(pWindow, "This beta version is expired\n" "Look at www.hoverrace.com for a new version", "Hover Race", MB_ICONSTOP | MB_APPLMODAL | MB_OK);
					}
					else {
						EndDialog(pWindow, IDOK);
					}
					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::LoginFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	static HBITMAP lBitmap = NULL;
	static HPALETTE lPalette = NULL;

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				// Image setting
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);
	
				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);
	
					lBitmap = LoadResourceBitmap(This->mInstance, MAKEINTRESOURCE(IDB_INTRO), &lPalette);
					ASSERT(lBitmap != NULL);
	
					SelectPalette(hdc, lPalette, FALSE);
					int lNbColors = RealizePalette(hdc);
					ReleaseDC(lBitmapCtl, hdc);
	
					TRACE("Colors0 %d  %d\n", lNbColors, GetLastError());
	
					HANDLE lOldHandle = (HANDLE) SendMessage(lBitmapCtl, STM_SETIMAGE, IMAGE_BITMAP, (long) lBitmap);
				}
				else {
					lBitmap = NULL;
					lPalette = NULL;
				}
			}
			lReturnValue = TRUE;
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
				case ID_GAME_REGISTER:
					// Nothing to do here for now
					/*This->DisplayRegistrationInfo(pWindow);
					   if(gKeyFilled)
					   EndDialog(pWindow, IDOK);
					   lReturnValue = TRUE; */
					break;
				case ID_HELP_SITE:
					This->DisplaySite();
					break;
			}
			break;
	}
	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::LoginPasswdFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	static HBITMAP lBitmap = NULL;
	static HPALETTE lPalette = NULL;

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events

		case WM_INITDIALOG:
			{
				// Fill the fields
				SetDlgItemText(pWindow, IDC_OWNER, This->mOwner);
				SetDlgItemText(pWindow, IDC_COMPANY, This->mCompany);
				lReturnValue = TRUE;
	
				// Image setting
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);
	
				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);
	
					lBitmap = LoadResourceBitmap(This->mInstance, MAKEINTRESOURCE(IDB_INTRO), &lPalette);
	
					ASSERT(lBitmap != NULL);
	
					SelectPalette(hdc, lPalette, FALSE);
					int lNbColors = RealizePalette(hdc);
					TRACE("Colors0 %d\n", lNbColors);
					ReleaseDC(lBitmapCtl, hdc);
	
					HANDLE lOldHandle = (HANDLE) SendMessage(lBitmapCtl, STM_SETIMAGE, IMAGE_BITMAP, (long) lBitmap);
				}
				else {
					lBitmap = NULL;
					lPalette = NULL;
				}
	
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

		case WM_QUERYNEWPALETTE:
			if(lPalette != NULL) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_BITMAP);

				if(lBitmapCtl != NULL) {
					HDC hdc = GetDC(lBitmapCtl);

					// SelectPalette( hdc, NULL, FALSE );
					// UnrealizeObject( lPalette );
					HPALETTE lOldPalette = SelectPalette(hdc, lPalette, FALSE);
					RealizePalette(hdc);

					// UpdateColors( hdc );
					InvalidateRgn(pWindow, NULL, TRUE);
					InvalidateRgn(lBitmapCtl, NULL, TRUE);
					UpdateWindow(pWindow);
					UpdateWindow(lBitmapCtl);

					if(lOldPalette != NULL) {
						// SelectPalette( hdc, lOldPalette, FALSE );
					}
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

					// SelectPalette( hdc, NULL, TRUE );
					// UnrealizeObject( lPalette );
					HPALETTE lOldPalette = SelectPalette(hdc, lPalette, FALSE);
					// UnrealizeObject( lPalette );
					RealizePalette(hdc);

					// InvalidateRgn( lBitmapCtl, NULL, TRUE );
					// UpdateWindow( lBitmapCtl );
					UpdateColors(hdc);

					if(lOldPalette != NULL) {
						// SelectPalette( hdc, lOldPalette, FALSE );
					}
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

				case IDC_DEMO:
					EndDialog(pWindow, IDOK);
					lReturnValue = TRUE;
					break;

				case IDOK:
					{
						lReturnValue = TRUE;
	
						char lBuffer[20];
						lBuffer[0] = 0;
	
						if(GetDlgItemText(pWindow, IDC_PASSWD, lBuffer, sizeof(lBuffer)) >= 0) {
							gKey = gKeyEncript;
							PasswordApply(lBuffer, &gKey);
	
							// now verify that the key is valid
							if(gKey.mKeySumK1 == GetKey1Sum(&gKey)) {
								if(gKey.mKeySumK2 == GetKey2Sum(&gKey)) {
									gKeyFilled = TRUE;
								}
							}
						}
	
						if(!gKeyFilled) {
							MessageBox(pWindow, MR_LoadString(IDS_BAD_PASSWD), MR_LoadString(IDS_GAME_NAME), MB_OK);
							This->RefreshTitleBar();
						}
						else {
							gKeyFilled = FALSE;
							This->RefreshTitleBar();
	
							// Verify that the owner name fit with the key
							// Verify that the IDS fits
							// verify that the product fit
							// Should normally be ok if the registry have not been moodified by hand
	
							if(gKey.mNameSum != ComputeUserNameSum(NormalizeUser(This->mOwner))) {
								MessageBox(pWindow, MR_LoadString(IDS_BAD_PASSWD), MR_LoadString(IDS_GAME_NAME), MB_OK);
								// MessageBox( pWindow, "Owner name does not match with the key", "HoverRace", MB_OK );
								break;
						}

						if(gKey.mIDSum != ComputeIDSum(This->mMajorID, This->mMinorID)) {
							MessageBox(pWindow, MR_LoadString(IDS_BAD_PASSWD), MR_LoadString(IDS_GAME_NAME), MB_OK);
							// MessageBox( pWindow, "User ID does not match with the key", "HoverRace", MB_OK );
							break;
						}

						if(gKey.mProduct != HOVERRACE_ID) {
							// MessageBox( pWindow, "Wrong password", "HoverRace", MB_OK );
							MessageBox(pWindow, MR_LoadString(IDS_NOT_HR_KEY), MR_LoadString(IDS_GAME_NAME), MB_OK);
							break;
						}

						gKeyFilled = TRUE;
						This->RefreshTitleBar();

						EndDialog(pWindow, IDOK);
					}
				}
				break;

				case ID_GAME_REGISTER:
					This->DisplayRegistrationInfo(pWindow);
					lReturnValue = TRUE;
					break;

			}
			break;
	}
	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::RegistrationFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			char lKey1[21];
			char lKey2[21];

			GetToKeys(lKey1, lKey2, &gKey);

			SetDlgItemText(pWindow, IDC_OWNER, This->mOwner);
			SetDlgItemText(pWindow, IDC_COMPANY, This->mCompany);
			SetDlgItemInt(pWindow, IDC_ID_MAJOR, This->mMajorID, TRUE);
			SetDlgItemInt(pWindow, IDC_ID_MINOR, This->mMinorID, TRUE);
			SetDlgItemText(pWindow, IDC_KEY1, lKey1);
			SetDlgItemText(pWindow, IDC_KEY2, lKey2);
			SetDlgItemText(pWindow, IDC_PASSWD, gKeyPassword);

			lReturnValue = TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					break;
				case IDOK:
					lReturnValue = TRUE;

					// Verify that all fields have correctly be filled
					char lOwner[60];
					char lCompany[60];
					int lMajorID;
					int lMinorID;
					char lKey1[21];
					char lKey2[21];
					char lPassword[20];
					KeyStructure lStruct;

					if(GetDlgItemText(pWindow, IDC_OWNER, lOwner, sizeof(lOwner)) <= 0) {
						MessageBox(pWindow, MR_LoadString(IDS_FILL_OWNER), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(GetDlgItemText(pWindow, IDC_COMPANY, lCompany, sizeof(lCompany)) <= 0)
						lCompany[0] = 0;

					lMajorID = GetDlgItemInt(pWindow, IDC_ID_MAJOR, NULL, FALSE);
					lMinorID = GetDlgItemInt(pWindow, IDC_ID_MINOR, NULL, FALSE);

					if(GetDlgItemText(pWindow, IDC_PASSWD, lPassword, sizeof(lPassword)) <= 0) {
						// MessageBox( pWindow, "Password must be at least 4 character long", "HoverRace", MB_OK );
						// break;
						lPassword[0] = 0;
					}
					else if(strlen(lPassword) > 10) {
						MessageBox(pWindow, MR_LoadString(IDS_PASSWD_LEN), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(GetDlgItemText(pWindow, IDC_KEY1, lKey1, sizeof(lKey1)) != 20) {
						MessageBox(pWindow, MR_LoadString(IDS_KEY1_LEN), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(GetDlgItemText(pWindow, IDC_KEY2, lKey2, sizeof(lKey2)) != 20) {
						MessageBox(pWindow, MR_LoadString(IDS_KEY2_LEN), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}
					// Verify that the keys have correctly been entered
					SetFromKeys(lKey1, lKey2, &lStruct);

					// Verify that the user name fit with the key
					if(lStruct.mKeySumK1 != GetKey1Sum(&lStruct)) {
						MessageBox(pWindow, MR_LoadString(IDS_BAD_KEY1), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(lStruct.mKeySumK2 != GetKey2Sum(&lStruct)) {
						MessageBox(pWindow, MR_LoadString(IDS_BAD_KEY2), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(lStruct.mNameSum != ComputeUserNameSum(NormalizeUser(lOwner))) {
						MessageBox(pWindow, MR_LoadString(IDS_BAD_USER), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(lStruct.mIDSum != ComputeIDSum(lMajorID, lMinorID)) {
						MessageBox(pWindow, MR_LoadString(IDS_BAD_ID), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}

					if(lStruct.mProduct != HOVERRACE_ID) {
						MessageBox(pWindow, MR_LoadString(IDS_NOT_HR_KEY), MR_LoadString(IDS_GAME_NAME), MB_OK);
						break;
					}
					// Make the new key the current key and save it
					gKey = lStruct;
					gKeyEncript = lStruct;
					gKeyFilled = TRUE;
					gKeyEncriptFilled = TRUE;
					PasswordApply(lPassword, &gKeyEncript);

					This->mOwner = lOwner;
					This->mCompany = lCompany;
					This->mMajorID = lMajorID;
					This->mMinorID = lMinorID;

					This->RefreshTitleBar();
					This->SaveRegistry();
					EndDialog(pWindow, IDOK);
					break;
			}
			break;
		case IDC_REGISTER_PAGE:
			This->DisplayRegistrationSite();
			lReturnValue = TRUE;
			break;
	}
	return lReturnValue;
}

BOOL CALLBACK MR_GameApp::RegistrationPasswdFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{

	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events

		case WM_INITDIALOG:
			{
				lReturnValue = TRUE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {

				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					break;

				case IDOK:
					{
						lReturnValue = TRUE;
	
						// Verify that all fields have correctly be filled
						KeyStructure lStruct = gKeyEncript;
	
						if(GetDlgItemText(pWindow, IDC_PASSWD, gKeyPassword, sizeof(gKeyPassword)) <= 0) {
							gKeyPassword[0] = 0;
						}
	
						PasswordApply(gKeyPassword, &lStruct);
	
						if((lStruct.mKeySumK1 == GetKey1Sum(&lStruct)) && (lStruct.mKeySumK2 == GetKey2Sum(&lStruct))) {
							// Verify that the owner name fit with the key
							// Verify that the IDS fits
							// verify that the product fit
							// Should normally be ok if the registry have not been moodified by hand
	
							if((lStruct.mNameSum == ComputeUserNameSum(NormalizeUser(This->mOwner))) && (lStruct.mIDSum == ComputeIDSum(This->mMajorID, This->mMinorID)) && (lStruct.mProduct == HOVERRACE_ID)) {
								gKey = lStruct;
								gKeyFilled = TRUE;
								This->RefreshTitleBar();
	
								EndDialog(pWindow, IDOK);
								break;
						}
					}
					// Wrong password or corrupted key
					MessageBox(pWindow, MR_LoadString(IDS_BAD_PASSWD), MR_LoadString(IDS_GAME_NAME), MB_OK);
				}
				break;

				case IDC_CLEAR:
					{
						lReturnValue = TRUE;
						if(MessageBox(pWindow, MR_LoadString(IDS_CLEAR_REGINFO), MR_LoadString(IDS_GAME_NAME), MB_OKCANCEL) == IDOK) {
							gKeyFilled = FALSE;
							gKeyEncriptFilled = FALSE;
							This->RefreshTitleBar();
							EndDialog(pWindow, IDOK);
	
						}
					}
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

	_snprintf(buf, 4, "%d.%d", gamma / 100, gamma % 100);
	SetDlgItemText(pWindow, IDC_GAMMA_TXT, buf);
	_snprintf(buf, 4, "%d.%d", contrast / 100, contrast % 100);
	SetDlgItemText(pWindow, IDC_CONTRAST_TXT, buf);
	_snprintf(buf, 4, "%d.%d", brightness / 100, brightness % 100);
	SetDlgItemText(pWindow, IDC_BRIGHTNESS_TXT, buf);
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
