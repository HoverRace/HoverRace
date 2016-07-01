
// SdlButtonView.cpp
//
// Copyright (c) 2013, 2016 Michael Imamura.
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

#include "../BaseContainer.h"
#include "../Button.h"
#include "../Box.h"
#include "../FillBox.h"
#include "../Label.h"
#include "../Picture.h"

#include "SdlButtonView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

Vec2 SdlButtonView::GetScreenPos() const
{
	// We assume that the background box is sized to the button.
	auto bgView = model.GetBackgroundChild()->GetView();
	return bgView ? bgView->GetScreenPos() : Vec2(0, 0);
}

Vec2 SdlButtonView::GetScreenSize() const
{
	// We assume that the background box is sized to the button.
	auto bgView = model.GetBackgroundChild()->GetView();
	return bgView ? bgView->GetScreenSize() : Vec2(0, 0);
}

Vec3 SdlButtonView::Measure()
{
	return model.GetSize().Promote();
}

void SdlButtonView::PrepareRender()
{
	auto picture = model.GetPictureChild();
	if (picture) picture->PrepareRender();

	model.GetBackgroundChild()->PrepareRender();

	if (auto contents = model.GetContentsChild()) {
		contents->PrepareRender();
	}

	model.GetLabelChild()->PrepareRender();

	Box *icon = model.GetIconChild();
	if (icon) icon->PrepareRender();

	if (model.IsFocused()) {
		model.GetHighlightChild()->PrepareRender();
	}
}

void SdlButtonView::Render()
{
	const Vec2 &size = model.GetSize();
	Vec2 oldOrigin = display.AddUiOrigin(model.GetAlignedPos(size.x, size.y));

	auto picture = model.GetPictureChild();
	if (picture) picture->Render();

	model.GetBackgroundChild()->Render();

	if (auto contents = model.GetContentsChild()) {
		contents->Render();
	}

	model.GetLabelChild()->Render();

	Box *icon = model.GetIconChild();
	if (icon) icon->Render();

	if (model.IsFocused()) {
		model.GetHighlightChild()->Render();
	}

	display.SetUiOrigin(oldOrigin);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
