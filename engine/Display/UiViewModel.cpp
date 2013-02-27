
// UiViewModel.cpp
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

#include "StdAfx.h"

#include "UiViewModel.h"

namespace HoverRace {
namespace Display {

/**
 * Set the position of the component.
 * @param pos The position, relative to the container (if any).
 */
void UiViewModel::SetPos(const Vec2 &pos)
{
	if (this->pos != pos) {
		this->pos = pos;
		FireModelUpdate(Props::POS);
	}
}


}  // namespace Display
}  // namespace HoverRace
