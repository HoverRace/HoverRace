
// SdlLabelView.cpp
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

#include <SDL2/SDL.h>

#include "../Label.h"
#include "SdlSurfaceText.h"

#include "SdlLabelView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlLabelView::SdlLabelView(SdlDisplay &disp, Label &model) :
	SUPER(disp, model),
	texture(), colorChanged(true), width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&SdlLabelView::OnUiScaleChanged, this));
}

SdlLabelView::~SdlLabelView()
{
	uiScaleChangedConnection.disconnect();
	if (texture) {
		SDL_DestroyTexture(texture);
	}
}

void SdlLabelView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Label::Props::COLOR:
			colorChanged = true;
			break;

		case Label::Props::FONT:
		case Label::Props::TEXT:
		case Label::Props::WRAP_WIDTH:
			if (texture) {
				SDL_DestroyTexture(texture);
				texture = nullptr;
			}
			break;
	}
}

void SdlLabelView::OnUiScaleChanged()
{
	if (texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

void SdlLabelView::PrepareRender()
{
	if (!texture) {
		UpdateTexture();
	} else if (colorChanged) {
		UpdateTextureColor();
	}
}

void SdlLabelView::Render()
{
	if (model.GetText().empty()) return;

	int w, h;
	SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
	display.DrawUiTexture(texture,
		model.GetAlignedPos(unscaledWidth, unscaledHeight),
		model.GetLayoutFlags());
}

/**
 * Update the state when the text is an empty string.
 * In this case, we measure the height of the string, and set the width to be 1
 * (to prevent accidental divisions by zero).
 */
void SdlLabelView::UpdateBlank()
{
	double scale = 1.0;

	UiFont font = model.GetFont();
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = display.GetUiScale());
	}

	height = SdlSurfaceText(display, font).MeasureLineHeight();

	unscaledWidth = width = 1;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlLabelView::UpdateTexture()
{
	if (texture) SDL_DestroyTexture(texture);

	if (model.GetText().empty()) {
		UpdateBlank();
		return;
	}

	double scale = 1.0;

	UiFont font = model.GetFont();
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = display.GetUiScale());
	}

	bool fixedWidth = !model.IsAutoWidth();
	double uiWrapWidth = model.GetWrapWidth();
	int wrapWidth = model.IsLayoutUnscaled() ?
		(int)uiWrapWidth :
		(int)(uiWrapWidth * scale);

	// Render the text onto a fresh new surface.
	SdlSurfaceText textRenderer(display, font);
	textRenderer.SetWrapWidth(fixedWidth ? wrapWidth : -1);
	SDL_Surface *tempSurface = textRenderer.RenderToNewSurface(
#		ifdef _WIN32
			model.GetWText()
#		else
			model.GetText()
#		endif
		);
	realWidth = width = textRenderer.GetWidth();
	realHeight = height = textRenderer.GetHeight();

	// Convert the surface to the display format.
	texture = SDL_CreateTextureFromSurface(display.GetRenderer(), tempSurface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(tempSurface);

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlLabelView::UpdateTextureColor()
{
	if (!model.GetText().empty()) {
		const Color cm = model.GetColor();
		SDL_SetTextureColorMod(texture, cm.bits.r, cm.bits.g, cm.bits.b);
		SDL_SetTextureAlphaMod(texture, cm.bits.a);
	}

	colorChanged = false;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
