
// SdlSliderView.cpp
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

#include <SDL2/SDL.h>

#include "../RuleLine.h"
#include "../Slider.h"
#include "../FillBox.h"

#include "SdlSliderView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

Vec2 SdlSliderView::GetScreenPos() const
{
	// We assume that the background box is sized to the button.
	auto bgView = model.GetBackgroundChild()->GetView();
	return bgView ? bgView->GetScreenPos() : Vec2(0, 0);
}

Vec2 SdlSliderView::GetScreenSize() const
{
	// We assume that the background box is sized to the button.
	auto bgView = model.GetBackgroundChild()->GetView();
	return bgView ? bgView->GetScreenSize() : Vec2(0, 0);
}

Vec3 SdlSliderView::Measure()
{
	return model.GetSize().Promote();
}

void SdlSliderView::PrepareRender()
{
	model.GetBackgroundChild()->PrepareRender();
	if (model.GetValue() != 0) {
		model.GetIndicatorChild()->PrepareRender();
	}
	if (auto zeroLine = model.GetZeroLineChild()) {
		zeroLine->PrepareRender();
	}
}

void SdlSliderView::Render()
{
	const Vec2 &size = model.GetSize();
	Vec2 oldOrigin = display.AddUiOrigin(model.GetAlignedPos(size.x, size.y));

	model.GetBackgroundChild()->Render();
	if (model.GetValue() != 0) {
		model.GetIndicatorChild()->Render();
	}
	if (auto zeroLine = model.GetZeroLineChild()) {
		zeroLine->Render();
	}

	display.SetUiOrigin(oldOrigin);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
