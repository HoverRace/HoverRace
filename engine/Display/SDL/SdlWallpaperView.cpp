
// SdlWallpaperView.cpp
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

#include "StdAfx.h"

#include <SDL2/SDL.h>

#include "../Wallpaper.h"
#include "SdlTexture.h"

#include "SdlWallpaperView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlWallpaperView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Wallpaper::Props::FILL:
			fillChanged = true;
			break;
		case Wallpaper::Props::OPACITY:
			opacityChanged = true;
			break;
	}
}

Vec3 SdlWallpaperView::Measure()
{
	return Vec3(display.GetScreenWidth(), display.GetScreenHeight(), 0);
}

void SdlWallpaperView::PrepareRender()
{
	if (fillChanged) {
		//TODO
	}
	if (opacityChanged) {
		computedAlpha = static_cast<MR_UInt8>(model.GetOpacity() * 255.0);
	}
	if (!texture) {
		texture = display.LoadRes(model.GetTexture());
	}
}

void SdlWallpaperView::Render()
{
	if (computedAlpha > 0) {
		SDL_Renderer *renderer = display.GetRenderer();
		SDL_Texture *tex = texture->Get();
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(tex, computedAlpha);
		SDL_SetTextureColorMod(tex, 255, 255, 255);
		SDL_RenderCopy(renderer, tex, nullptr, nullptr);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
