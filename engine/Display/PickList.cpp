
// PickList.cpp
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

#include "../Util/Symbol.h"
#include "SymbolIcon.h"

#include "PickList.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

PickListItem::PickListItem(Display &display, const std::string &text,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, text, layoutFlags)
{
	SetTextAlignment(Alignment::W);
	Init();
}

void PickListItem::Init()
{
	InitIcon(false, false);
	InitIcon(false, true);
	InitIcon(true, false);
	InitIcon(true, true);
}

/**
 * Generate and register the icon for a given combination of states.
 * @param enabled @c true for the enabled state, @c false for disabled.
 * @param checked @c true for the checked state, @c false for unchecked.
 */
void PickListItem::InitIcon(bool enabled, bool checked)
{
	// Using the same symbols as radio buttons for now.

	auto icon = std::make_shared<SymbolIcon>(
		1, 1,  // Size will be set in the superclass in Layout().
		checked ? Symbol::DOT_CIRCLE_O : Symbol::CIRCLE_O,
		enabled ? COLOR_WHITE : 0x7fffffff);
	icon->AttachView(display);

	SetStateIcon(enabled, checked, icon);
}

}  // namespace Display
}  // namespace HoverRace
