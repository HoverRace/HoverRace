
// IntroMovie.cpp
// The intro movie that plays at startup.
//
// Copyright (c) 2010 Michael Imamura.
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

#ifdef WITH_DIRECTSHOW
#	include <dshow.h>
#	pragma comment(lib, "Strmiids.lib")
#else
#	include <Vfw.h>
#	pragma comment(lib, "vfw32.lib")
#endif

#include "../../engine/Util/Config.h"

#include "IntroMovie.h"

using namespace HoverRace;
using namespace HoverRace::Client;
using HoverRace::Util::Config;

IntroMovie::IntroMovie(HWND hwnd, HINSTANCE hinst) :
	hwnd(hwnd)
{
	Config *cfg = Config::GetInstance();
	std::string movieFilename = cfg->GetMediaPath("Intro.avi");

#ifdef WITH_DIRECTSHOW
	//TODO
#else
	movieWnd = MCIWndCreate(
		hwnd, hinst, 
		WS_CHILD | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, 
		movieFilename.c_str());

	// Fill the client area.
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	MoveWindow(movieWnd, 0, 0,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		TRUE);
#endif
}

IntroMovie::~IntroMovie()
{
#ifdef WITH_DIRECTSHOW
	//TODO
#else
	if (movieWnd != NULL) {
		MCIWndClose(movieWnd);
		Sleep(1000);  // Is this necessary?
		MCIWndDestroy(movieWnd);
	}
#endif
}

void IntroMovie::Play()
{
#ifdef WITH_DIRECTSHOW
	//TODO
#else
	if (movieWnd != NULL)
		MCIWndPlay(movieWnd);
#endif
}

void IntroMovie::ResetSize()
{
#ifndef WITH_DIRECTSHOW
	RECT clientRect, movieRect;

	if (GetClientRect(hwnd, &clientRect)) {
		if (GetWindowRect(movieWnd, &movieRect)) {
			SetWindowPos(movieWnd, HWND_TOP,
				0, 0,
				clientRect.right - clientRect.left,
				clientRect.bottom - clientRect.top,
				SWP_SHOWWINDOW);
		}
	}
#endif
}

void IntroMovie::ResetPalette(bool background)
{
#ifndef WITH_DIRECTSHOW
	MCIWndRealize(movieWnd, background);
#endif
}
