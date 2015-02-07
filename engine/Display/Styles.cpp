
// Styles.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../Util/Config.h"

#include "Styles.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

Styles::Styles() :
	gridMargin(0, 0), gridPadding(0, 0)
{
	Reload();
}

void Styles::Reload()
{
	const Config *cfg = Config::GetInstance();
	const std::string &monospaceFontName = cfg->GetDefaultMonospaceFontName();

	// Hack fix for broken alpha blending when HW acceleration is off.
	// Only full-brightness, fully-opaque white is affected.
	const Color WHITE(cfg->runtime.noAccel ? 0xfffefefe : COLOR_WHITE);

	bodyFont.Set(30, 0);
	bodyFg = 0xffbfbfbf;
	bodyHeadFont.Set(30, 0);
	bodyHeadFg = WHITE;
	bodyAsideFont.Set(20, 0);
	bodyAsideFg = 0xffbfbfbf;

	announcementHeadFont.Set(30);
	announcementHeadFg = WHITE;
	announcementBodyFont.Set(20);
	announcementBodyFg = 0xff7f7f7f;
	announcementSymbolFg = 0xbfffffff;
	announcementBg = 0xbf000000;

	// 100% HUD scale.
	hudNormalFont.Set(30, UiFont::BOLD);
	hudNormalHeadFont.Set(20, 0);
	// 50% HUD scale.
	hudSmallFont.Set(20, UiFont::BOLD);
	hudSmallHeadFont.Set(15, 0);

	consoleFont.Set(monospaceFontName, 30);
	consoleFg = WHITE;
	consoleCursorFg = 0xffbfbfbf;
	consoleBg = 0xbf000000;

	formFont = bodyFont;
	formFg = WHITE;
	formDisabledFg = 0x7fffffff;

	dialogBg = 0xcc000000;

	gridMargin.x = 6;
	gridMargin.y = 6;
	gridPadding.x = 1;
	gridPadding.y = 1;

	buttonBg = 0x3f00007f;
	buttonFocusedBg = 0xff00007f;
	buttonDisabledBg = 0x3f7f7f7f;
	buttonPressedBg = 0x7f00007f;

	headingFont.Set(40, UiFont::BOLD);
	headingFg = WHITE;
}

}  // namespace Display
}  // namespace HoverRace
