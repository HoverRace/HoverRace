
// SdlPictureView.cpp
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

#include "../Picture.h"
#include "SdlTexture.h"

#include "SdlPictureView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

SdlPictureView::SdlPictureView(SdlDisplay &disp, Picture &model) :
	SUPER(disp, model),
	textureChanged(true)
{
}

void SdlPictureView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Picture::Props::TEXTURE:
			textureChanged = true;
			break;
	}
}

Vec3 SdlPictureView::Measure()
{
	return model.GetSize().Promote();
}

void SdlPictureView::PrepareRender()
{
	if (textureChanged) {
		auto modelTexture = model.GetTexture();

		if (modelTexture) {
			texture = display.LoadRes(modelTexture);
		}
		else {
			texture.reset();
		}

		textureChanged = false;
	}
}

void SdlPictureView::Render()
{
	const Color color = model.GetColor();
	if (texture && color.bits.a > 0) {
		SDL_Renderer *renderer = display.GetRenderer();

		SDL_Texture *tex = texture->Get();
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(tex, color.bits.a);
		SDL_SetTextureColorMod(tex, color.bits.r, color.bits.g, color.bits.b);

		Vec2 pos = display.LayoutUiPosition(
			model.GetAlignedPos(model.GetSize().x, model.GetSize().y));

		const Vec2 &size = model.GetSize();
		double w = size.x;
		double h = size.y;
		if (!model.IsLayoutUnscaled()) {
			double uiScale = display.GetUiScale();
			w *= uiScale;
			h *= uiScale;
		}

		SDL_Rect rect = {
			static_cast<int>(pos.x), static_cast<int>(pos.y),
			static_cast<int>(w), static_cast<int>(h) };
		SDL_RenderCopy(renderer, tex, nullptr, &rect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
