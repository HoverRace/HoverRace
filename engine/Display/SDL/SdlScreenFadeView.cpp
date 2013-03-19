
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

#include <SDL/SDL.h>

#include "../ScreenFade.h"

#include "SdlScreenFadeView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlScreenFadeView::SdlScreenFadeView(SdlDisplay &disp, ScreenFade &model) :
	SUPER(disp, model),
	surface(), opacityChanged(true), opacityVisible(false)
{
	// Be notified of window resizes so we can update the internal surface.
	displayConfigChangedConn =
		disp.GetDisplayConfigChangedSignal().connect(
			std::bind(&SdlScreenFadeView::OnWindowResChange, this));
}

SdlScreenFadeView::~SdlScreenFadeView()
{
	displayConfigChangedConn.disconnect();
	if (surface) {
		SDL_FreeSurface(surface);
	}
}

void SdlScreenFadeView::OnModelUpdate(int prop)
{
	switch (prop) {
		case ScreenFade::Props::COLOR:
			if (surface) {
				SDL_FreeSurface(surface);
				surface = nullptr;
			}
			break;

		case ScreenFade::Props::OPACITY:
			opacityChanged = true;
			break;
	}
}

void SdlScreenFadeView::OnWindowResChange()
{
	if (surface) {
		SDL_FreeSurface(surface);
		surface = nullptr;
	}
}

void SdlScreenFadeView::PrepareRender()
{
	if (!surface) {
		surface = disp.CreateHardwareSurface(
			disp.GetScreenWidth(), disp.GetScreenHeight());

		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

		const Color color = model.GetColor();
		SDL_FillRect(surface, nullptr,
			SDL_MapRGBA(surface->format,
				color.bits.r, color.bits.g, color.bits.b, 0xff));

		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

		opacityChanged = true;
	}

	if (opacityChanged) {
		double opacity = model.GetOpacity();
		if ((opacityVisible = opacity > 0.0)) {
			SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL,
				static_cast<MR_UInt8>(opacity * 255.0));
		}
		opacityChanged = false;
	}
}

void SdlScreenFadeView::Render()
{
	if (opacityVisible) {
		SDL_BlitSurface(surface, nullptr, SDL_GetVideoSurface(), nullptr);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
