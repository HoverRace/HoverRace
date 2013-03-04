
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

#ifdef _WIN32
static RGBQUAD RGB_BLACK = { 0, 0, 0, 0 };
static RGBQUAD RGB_WHITE = { 0xff, 0xff, 0xff, 0 };
#endif

SdlScreenFadeView::SdlScreenFadeView(SdlDisplay &disp, ScreenFade &model) :
	SUPER(disp, model),
	surface(), opacityChanged(true)
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
		SDL_Surface *tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			disp.GetScreenWidth(), disp.GetScreenHeight(), 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

		const Color color = model.GetColor();
		MR_UInt32 fillColor =
			((MR_UInt32)(color.bits.r) << 24) +
			((MR_UInt32)(color.bits.g) << 16) +
			((MR_UInt32)(color.bits.b) << 8) +
			0xff;
		MR_UInt8 *buf = static_cast<MR_UInt8*>(tempSurface->pixels);
		for (int y = 0; y < tempSurface->h; y++) {
			MR_UInt32 *row = reinterpret_cast<MR_UInt32*>(buf);
			for (int x = 0; x < tempSurface->w; x++) {
				*row++ = fillColor;
			}
			buf += tempSurface->pitch;
		}

		surface = SDL_DisplayFormat(tempSurface);
		SDL_FreeSurface(tempSurface);

		opacityChanged = true;
	}

	if (opacityChanged) {
		SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL,
			static_cast<MR_UInt8>(model.GetOpacity() * 255.0));
		opacityChanged = false;
	}
}

void SdlScreenFadeView::Render()
{
	SDL_BlitSurface(surface, nullptr, SDL_GetVideoSurface(), nullptr);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
