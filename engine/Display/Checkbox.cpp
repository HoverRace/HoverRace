
// Checkbox.cpp
//
// Copyright (c) 2013 Michael Imamura.
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
#include "../Util/Log.h"
#include "SymbolIcon.h"

#include "Checkbox.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized checkbox.
 * @param display The display child elements will be attached to.
 * @param text The text for the checkbox label.
 * @param layoutFlags Optional layout flags.
 */
Checkbox::Checkbox(Display &display, const std::string &text,
                   uiLayoutFlags_t layoutFlags) :
	SUPER(display, text, layoutFlags), display(display),
	checked(false)
{
	Init();
}

/**
 * Constructor for fixed-sized checkbox.
 * @param display The display child elements will be attached to.
 * @param size The fixed checkbox size.
 * @param text The text for the checkbox label.
 * @param layoutFlags Optional layout flags.
 */
Checkbox::Checkbox(Display &display, const Vec2 &size, const std::string &text,
                   uiLayoutFlags_t layoutFlags) :
	SUPER(display, size, text, layoutFlags), display(display),
	checked(false)
{
	Init();
}

Checkbox::~Checkbox()
{
}

void Checkbox::Init()
{
	InitIcon(false, false);
	InitIcon(false, true);
	InitIcon(true, false);
	InitIcon(true, true);

	UpdateIcon();
}

/**
 * Generate and register the icon for a given combination of states.
 * @param enabled @c true for the enabled state, @c false for disabled.
 * @param checked @c true for the checked state, @c false for unchecked.
 */
void Checkbox::InitIcon(bool enabled, bool checked)
{
	//TODO: Share this set of icons with all checkbox instances.

	auto icon = std::make_shared<SymbolIcon>(
		1, 1,  // Size will be set the superclass in Layout().
		checked ? 0xf046 : 0xf096,
		enabled ? COLOR_WHITE : 0x7fffffff);
	icon->AttachView(display);

	icons.insert(icons_t::value_type(iconsKey_t(enabled, checked), icon));
}

void Checkbox::FireClickedSignal()
{
	SetChecked(!IsChecked());
	SUPER::FireClickedSignal();
}

/**
 * Set the checkbox state.
 * @param checked @c true for checked, @c false for unchecked.
 */
void Checkbox::SetChecked(bool checked)
{
	if (this->checked != checked) {
		this->checked = checked;
		FireModelUpdate(Props::CHECKED);
		UpdateIcon();
	}
}

/**
 * Set the appropriate icon for the current widget state.
 */
void Checkbox::UpdateIcon()
{
	SetIcon(icons[iconsKey_t(IsEnabled(), IsChecked())]);
}

void Checkbox::FireModelUpdate(int prop)
{
	if (prop == ClickRegion::Props::ENABLED) {
		UpdateIcon();
	}

	SUPER::FireModelUpdate(prop);
}

}  // namespace Display
}  // namespace HoverRace
