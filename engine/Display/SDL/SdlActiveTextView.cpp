
// SdlActiveTextView.cpp
//
// Copyright (c) 2015-2016 Michael Imamura.
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

#include "../../Util/Config.h"

#include "../ActiveText.h"
#include "../TypeCase.h"

#include "SdlActiveTextView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlActiveTextView::SdlActiveTextView(SdlDisplay &disp, ActiveText &model) :
	SUPER(disp, model),
	needsUpdate(true), width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&SdlActiveTextView::OnUiScaleChanged, this));
}

SdlActiveTextView::~SdlActiveTextView()
{
}

void SdlActiveTextView::OnUiScaleChanged()
{
	needsUpdate = true;
	typeCase.reset();
}

void SdlActiveTextView::OnModelUpdate(int prop)
{
	switch (prop) {
		case BaseText::Props::FONT:
		case BaseText::Props::TEXT:
			needsUpdate = true;
			break;
	}
}

Vec3 SdlActiveTextView::Measure()
{
	PrepareRender();

	return { unscaledWidth, unscaledHeight, 0 };
}

void SdlActiveTextView::PrepareRender()
{
	if (needsUpdate) {
		UpdateTexture();
		needsUpdate = false;
	}
}

void SdlActiveTextView::Render()
{
	Vec2 aligned = display.LayoutUiPosition(
		model.GetAlignedPos(unscaledWidth, unscaledHeight));
	typeLine.Render(
		model.GetColor(),
		static_cast<int>(aligned.x),
		static_cast<int>(aligned.y),
		model.IsCaretVisible() ?
			model.GetCaretPos() :
			boost::optional<size_t>{});
}

void SdlActiveTextView::UpdateTexture()
{
	double scale = 1.0;

	UiFont font = model.GetFont();
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = display.GetUiScale());
	}

	// Apply text scale since SdlTypeCase always uses unscaled fonts for
	// cache reasons.
	if (!model.IsFixedScale()) {
		font.size *= Config::GetInstance()->video.textScale;
	}

	if (!typeCase) {
		typeCase = display.GetTypeCase(font);
	}
	typeCase->Prepare(model.GetText(), &typeLine);

	width = typeLine.width;
	height = typeLine.height;

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
