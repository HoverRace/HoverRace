
// Styles.h
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

#pragma once

#include "../Vec.h"
#include "UiFont.h"
#include "Color.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Display {

/**
 * Shared styles for UI components.
 * @author Michael Imamura
 */
class Styles {

public:
	Styles();

public:
	void Reload();

public:
	// Standard text.
	UiFont bodyFont;
	Color bodyFg;
	UiFont bodyHeadFont;
	Color bodyHeadFg;
	UiFont bodyAsideFont;
	Color bodyAsideFg;

	// Announcements.
	UiFont announcementHeadFont;
	Color announcementHeadFg;
	UiFont announcementBodyFont;
	Color announcementBodyFg;
	Color announcementSymbolFg;
	Color announcementBg;

	// HUD text.
	UiFont hudNormalFont;
	UiFont hudNormalHeadFont;
	UiFont hudSmallFont;
	UiFont hudSmallHeadFont;

	// Console text.
	UiFont consoleFont;
	Color consoleFg;
	Color consoleCursorFg;
	Color consoleBg;

	// UI widgets (buttons, etc.).
	UiFont formFont;
	Color formFg;
	Color formDisabledFg;

	// Dialog background shading color.
	Color dialogBg;

	// Attributes for grid containers.
	Vec2 gridMargin;
	Vec2 gridPadding;

	// Colors specific to button-like widgets.
	Color buttonBg;
	Color buttonFocusedBg;
	Color buttonDisabledBg;
	Color buttonPressedBg;

	// Headings, titles.
	UiFont headingFont;
	Color headingFg;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
