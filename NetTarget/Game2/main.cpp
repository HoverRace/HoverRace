// main.cpp              Fireball game entry point
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
#include "stdafx.h"		// likely unnecessary; can we get rid of it?
#include "GameApp.h"

// Entry point
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE pPrevInstance, LPSTR /* pCmdLine */ , int pCmdShow)
{
    // initialize return variables
    BOOL lReturnValue = TRUE;
    int lErrorCode = -1;

    MR_GameApp lGame(pInstance);

    // Allow only one instance of HoverRace; press CAPS_LOCK to bypass
    GetAsyncKeyState(VK_CAPITAL);	// Reset the function
    if(!GetAsyncKeyState(VK_CAPITAL))
	lReturnValue = lGame.IsFirstInstance();

    if(lReturnValue) {
	//lReturnValue = lGame.DisplayNotice(); // seems to crash when it does this
	lReturnValue = lGame.DisplayLoginWindow();
    }

    GetAsyncKeyState(VK_CAPITAL);	// Reset the function
    if(lReturnValue && !GetAsyncKeyState(VK_CAPITAL))
	lReturnValue = lGame.IsFirstInstance();

    if(lReturnValue && (pPrevInstance == NULL))
	lReturnValue = lGame.InitApplication();

    if(lReturnValue)
	lReturnValue = lGame.InitGame();

    // this is where the game actually takes control
    if(lReturnValue)
	lErrorCode = lGame.MainLoop();

    return lErrorCode;
}
