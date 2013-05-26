
// SdlScreenFadeView.cpp
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

#include "../ScreenFade.h"

#include "SdlScreenFadeView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlScreenFadeView::OnModelUpdate(int prop)
{
	switch (prop) {
		case ScreenFade::Props::COLOR:
		case ScreenFade::Props::OPACITY:
			opacityChanged = true;
			break;
	}
}

Vec3 SdlScreenFadeView::Measure()
{
	return Vec3(disp.GetScreenWidth(), disp.GetScreenHeight(), 0);
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
		SDL_Renderer *renderer = disp.GetRenderer();
		SDL_SetRenderDrawColor(renderer,
			color.bits.r, color.bits.g, color.bits.b, computedAlpha);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		SDL_Rect rect = { 0, 0, disp.GetScreenWidth(), disp.GetScreenHeight() };
		SDL_RenderFillRect(renderer, &rect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
