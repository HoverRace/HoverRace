
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
#include "../../engine/VideoServices/Viewport2D.h"
#include "../../engine/VideoServices/Font.h"
#include "../../engine/VideoServices/StaticText.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "FullscreenTest.h"

using boost::format;
using boost::str;

using namespace HoverRace::Util;
using namespace HoverRace::VideoServices;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param oldX The old fullscreen width.
 * @param oldY The old fullscreen height.
 * @param oldMonitor The old monitor.
 */
FullscreenTest::FullscreenTest(int oldX, int oldY, const std::string &oldMonitor) :
	oldX(oldX), oldY(oldY), oldMonitor(oldMonitor),
	viewport(new VideoServices::Viewport2D()), widgetsInitialized(false),
	heading(NULL), subheading(NULL),
	timeRemaining(5)
{
}

FullscreenTest::~FullscreenTest()
{
	delete subheading;
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

int FullscreenTest::ScaleFont(int i, int resY)
{
	// Assume that font sizes given are relative to a 640x480 screen.
	return i * resY / 480;
}

void FullscreenTest::UpdateSubheading(int resY)
{
	// Keep these translations short -- the user doesn't have much time to read them!
	const char *text = ngettext(
		"Returning in %d second.",
		"Returning in %d seconds.",
		timeRemaining);
	std::string subheadingStr = str(format(text) % timeRemaining);
	if (subheading == NULL) {
		Font subheadingFont("Arial", ScaleFont(16, resY), true);
		subheading = new StaticText(subheadingStr, subheadingFont, 0x10);
	}
	else {
		subheading->SetText(subheadingStr);
	}
}

void FullscreenTest::InitWidgets(int resX, int resY)
{
	Font headingFont("Arial", ScaleFont(20, resY), true);
	
	// Keep these translations short -- the user doesn't have much time to read them!
	std::string headingStr = _("HoverRace fullscreen test");
	headingStr += ": ";
	headingStr += str(format("%dx%d", OS::stdLocale) % resX % resY);
	heading = new StaticText(headingStr, headingFont, 0x0c);
}

/**
 * Render the test screen.
 * This expects to have exclusive control over the screen while the
 * test is active.
 * @param dest The destination video buffer.
 */
void FullscreenTest::Render(VideoServices::VideoBuffer *dest)
{
	int resX = dest->GetXRes();
	int resY = dest->GetYRes();
	if (!widgetsInitialized) {
		InitWidgets(resX, resY);
		widgetsInitialized = true;
	}
	UpdateSubheading(resY);

	dest->Lock();
	dest->Clear(0);
	viewport->Setup(dest, 0, 0, resX, resY);
	
	int leftMargin = resX / 20;
	int curY = resY / 2 - heading->GetHeight();
	heading->Blt(leftMargin, curY, viewport);
	curY += heading->GetHeight();

	subheading->Blt(leftMargin, curY, viewport);

	dest->Unlock();
}

}  // namespace Client
}  // namespace HoverRace
