
// FullscreenTest.cpp
// Fullscreen test display.
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

#include "../../engine/Util/Config.h"
#include "../../engine/Util/Str.h"
#include "../../engine/VideoServices/2DViewPort.h"
#include "../../engine/VideoServices/Font.h"
#include "../../engine/VideoServices/MultipartText.h"
#include "../../engine/VideoServices/NumericGlyphs.h"
#include "../../engine/VideoServices/StaticText.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "FullscreenTest.h"

using namespace HoverRace::Client;
using namespace HoverRace::Util;
using namespace HoverRace::VideoServices;

/**
 * Constructor.
 * @param oldX The old fullscreen width.
 * @param oldY The old fullscreen height.
 * @param oldMonitor The old monitor.
 */
FullscreenTest::FullscreenTest(int oldX, int oldY, const std::string &oldMonitor) :
	oldX(oldX), oldY(oldY), oldMonitor(oldMonitor),
	viewport(new MR_2DViewPort()), widgetsInitialized(false),
	heading(NULL),
	timeRemaining(5)
{
}

FullscreenTest::~FullscreenTest()
{
	delete heading;
	delete viewport;
}

/**
 * Check if the fullscreen test should be activated.
 * @param parent The parent window (may not be @c NULL).
 * @return @c true if should activate, @c false otherwise.
 */
bool FullscreenTest::ShouldActivateTest(HWND parent) const
{
	const Config *cfg = Config::GetInstance();

	if (oldX != cfg->video.xResFullscreen ||
		oldY != cfg->video.yResFullscreen ||
		oldMonitor != cfg->video.fullscreenMonitor)
	{
		if (MessageBoxW(parent,
			Str::UW(_("Fullscreen settings have changed.  "
			"Would you like to test these changes?")),
			PACKAGE_NAME_L, MB_YESNO) == IDYES)
		{
			return true;
		}
	}
	return false;
}

/**
 * Decrement the timer by one second.
 * @return @c true if the timer has reached zero.
 */
bool FullscreenTest::TickTimer()
{
	--timeRemaining;
	return timeRemaining == 0;
}

void FullscreenTest::InitWidgets(int resY)
{
	Font headingFont("Arial", 20 * resY / 480, true);
	
	heading = new StaticText(_("HoverRace fullscreen settings test"), headingFont, 0x08);
}

/**
 * Render the test screen.
 * This expects to have exclusive control over the screen while the
 * test is active.
 * @param dest The destination video buffer.
 */
void FullscreenTest::Render(MR_VideoBuffer *dest)
{
	int resX = dest->GetXRes();
	int resY = dest->GetYRes();
	if (!widgetsInitialized) {
		InitWidgets(resY);
		widgetsInitialized = true;
	}

	dest->Lock();
	dest->Clear(0);
	viewport->Setup(dest, 0, 0, resX, resY);
	
	int leftMargin = resX / 20;
	int curY = resY / 2 - heading->GetHeight();
	heading->Blt(leftMargin, curY, viewport);
	//curY += heading->GetHeight();

	dest->Unlock();
}

