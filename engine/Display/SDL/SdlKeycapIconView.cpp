
// SdlKeycapIconView.cpp
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

#include "../../StdAfx.h"

#include <SDL2/SDL.h>

#include "../../Control/Controller.h"
#include "../KeycapIcon.h"
#include "SdlSurfaceText.h"

#include "SdlKeycapIconView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

SdlKeycapIconView::SdlKeycapIconView(SdlDisplay &disp, KeycapIcon &model) :
	SUPER(disp, model),
	texture(), colorChanged(true), width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&decltype(texture)::release, &texture));
}

SdlKeycapIconView::~SdlKeycapIconView()
{
}

void SdlKeycapIconView::OnModelUpdate(int prop)
{
	switch (prop) {
		case FillBox::Props::COLOR:
			colorChanged = true;
			break;

		case FillBox::Props::SIZE:
		case KeycapIcon::Props::KEY_HASH:
			texture.release();
			break;
	}
}

Vec3 SdlKeycapIconView::Measure()
{
	PrepareRender();

	return { unscaledWidth, unscaledHeight, 0 };
}

void SdlKeycapIconView::PrepareRender()
{
	if (!texture) {
		UpdateTexture();
	} else if (colorChanged) {
		UpdateTextureColor();
	}
}

void SdlKeycapIconView::Render()
{
	display.DrawUiTexture(texture->Get(),
		model.GetAlignedPos(unscaledWidth, unscaledHeight),
		model.GetLayoutFlags());
}

void SdlKeycapIconView::UpdateTexture()
{
	double scale = 1.0;

	UiFont font("", model.GetSize().y);
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = display.GetUiScale());
	}

	std::ostringstream oss;
	oss << '[' <<
		Control::InputEventController::HashToString(model.GetKeyHash()) << ']';
	std::string text = oss.str();

	// Render the text onto a fresh new surface.
	SdlSurfaceText textRenderer(display, font);
	textRenderer.SetFixedScale(true);
	SDL_Surface *tempSurface = textRenderer.RenderToNewSurface(text);
	realWidth = width = textRenderer.GetWidth();
	realHeight = height = textRenderer.GetHeight();

	// Convert the surface to the display format.
	SDL_Texture *texture = SDL_CreateTextureFromSurface(
		display.GetRenderer(), tempSurface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(tempSurface);
	this->texture.reset(new SdlTexture(display, texture));

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlKeycapIconView::UpdateTextureColor()
{
	const Color cm = model.GetColor();
	SDL_SetTextureColorMod(texture->Get(), cm.bits.r, cm.bits.g, cm.bits.b);
	SDL_SetTextureAlphaMod(texture->Get(), cm.bits.a);

	colorChanged = false;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
