
// KeycapIcon.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include "KeycapIcon.h"

namespace HoverRace {
namespace Display {

/**
 * Construcotr.
 * @param height The height of the icon (the width is automatic).
 * @param keyHash The input hash for the keyboard key or gamepad button.
 * @param color The color of the icon (including alpha).  Note that some parts
 *              of the icon may use a different color depending on the
 *              rules for gamepads.
 * @param layoutFlags Optional layout flags.
 */
KeycapIcon::KeycapIcon(double height, int keyHash,
	const Color color, uiLayoutFlags_t layoutFlags) :
	SUPER(height, height, color, layoutFlags),
	keyHash(keyHash)
{
}

void KeycapIcon::SetKeyHash(int keyHash)
{
	if (this->keyHash != keyHash) {
		this->keyHash = keyHash;
		FireModelUpdate(Props::KEY_HASH);
	}
}

void KeycapIcon::AdjustHeight(double h)
{
	// The view only pays attention to the height.
	SetSize(h, h);
}

void KeycapIcon::AdjustWidth(double w)
{
	// The view only pays attention to the height.
	//TODO: Measure and preserve aspect ratio.
	SetSize(w, w);
}

}  // namespace Display
}  // namespace HoverRace
