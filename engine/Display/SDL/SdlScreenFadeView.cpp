
// SdlScreenFadeView.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../ScreenFade.h"

#include "SdlScreenFadeView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlScreenFadeView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Background::Props::OPACITY:
		case ScreenFade::Props::COLOR:
			opacityChanged = true;
			break;
	}
}

Vec3 SdlScreenFadeView::Measure()
{
	return Vec3(display.GetScreenWidth(), display.GetScreenHeight(), 0);
}

void SdlScreenFadeView::PrepareRender()
{
	if (opacityChanged) {
		double alpha = static_cast<double>(model.GetColor().bits.a);
		double opacity = model.GetOpacity();
		computedAlpha = static_cast<MR_UInt8>(opacity * alpha);

		opacityChanged = false;
	}
}

void SdlScreenFadeView::Render()
{
	if (computedAlpha > 0) {
		const Color color = model.GetColor();
		SDL_Renderer *renderer = display.GetRenderer();
		SDL_SetRenderDrawColor(renderer,
			color.bits.r, color.bits.g, color.bits.b, computedAlpha);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		SDL_Rect rect = { 0, 0, display.GetScreenWidth(), display.GetScreenHeight() };
		SDL_RenderFillRect(renderer, &rect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
