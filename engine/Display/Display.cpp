
// Display.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../StdAfx.h"

#include "../Util/Config.h"

#include "Display.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const double MIN_VIRT_WIDTH = 1280.0;
	const double MIN_VIRT_HEIGHT = 720.0;
}

void Display::OnDisplayConfigChanged()
{
	Config::cfg_video_t &vidCfg = Config::GetInstance()->video;
	int w = vidCfg.xRes;
	int h = vidCfg.yRes;
	double wd = static_cast<double>(w);
	double hd = static_cast<double>(h);

	// Recalculate the UI coordinates.
	double newUiScale;
	double uiScaleW = wd / MIN_VIRT_WIDTH;
	double uiScaleH = hd / MIN_VIRT_HEIGHT;
	if (uiScaleW < uiScaleH) {
		newUiScale = uiScaleW;
		uiOffset.x = 0;
		uiOffset.y = floor((hd - (MIN_VIRT_HEIGHT * uiScaleW)) / 2);
	}
	else {
		newUiScale = uiScaleH;
		uiOffset.x = floor((wd - (MIN_VIRT_WIDTH * uiScaleH)) / 2);
		uiOffset.y = 0;
	}

	FireDisplayConfigChangedSignal(w, h);

	if (uiScale != newUiScale) {
		uiScale = newUiScale;
		FireUiScaleChangedSignal(uiScale);
	}
}

void Display::FireDisplayConfigChangedSignal(int width, int height) const
{
	displayConfigChangedSignal(width, height);
}

void Display::FireUiScaleChangedSignal(double scale) const
{
	uiScaleChangedSignal(scale);
}

//{{{ styles_t /////////////////////////////////////////////////////////////////

Display::styles_t::styles_t() :
	gridMargin(0, 0), gridPadding(0, 0)
{
	Reload();
}

void Display::styles_t::Reload()
{
	const Config *cfg = Config::GetInstance();
	const std::string &defaultFontName = cfg->GetDefaultFontName();
	const std::string &monospaceFontName = cfg->GetDefaultMonospaceFontName();

	bodyFont.Set(defaultFontName, 30, 0);
	bodyFg = Color(0xffbfbfbf);
	bodyHeadFont.Set(defaultFontName, 30, 0);
	bodyHeadFg = Color(0xffffffff);
	bodyAsideFont.Set(defaultFontName, 20, 0);
	bodyAsideFg = Color(0xffbfbfbf);

	announcementHeadFont.Set(defaultFontName, 30);
	announcementHeadFg = COLOR_WHITE;
	announcementBodyFont.Set(defaultFontName, 20);
	announcementBodyFg = Color(0xff7f7f7f);
	announcementSymbolFg = Color(0xbfffffff);
	announcementBg = Color(0xbf000000);

	consoleFont.Set(monospaceFontName, 30);
	consoleFg = COLOR_WHITE;
	consoleCursorFg = 0xffbfbfbf;
	consoleBg = 0xbf000000;

	formFont = bodyFont;
	formFg = COLOR_WHITE;
	formDisabledFg = 0x7fffffff;

	dialogBg = 0xcc000000;

	gridMargin.x = 6;
	gridMargin.y = 6;
	gridPadding.x = 1;
	gridPadding.y = 1;

	buttonBg = 0x3f00007f;
	buttonDisabledBg = 0x3f7f7f7f;
	buttonPressedBg = 0x7f00007f;

	headingFont.Set(defaultFontName, 40, UiFont::BOLD);
	headingFg = COLOR_WHITE;
}

//}}} styles_t

}  // namespace Display
}  // namespace HoverRace
