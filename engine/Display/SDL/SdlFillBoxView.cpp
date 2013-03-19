
// SdlFillBoxView.cpp
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

#include "../FillBox.h"

#include "SdlFillBoxView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlFillBoxView::SdlFillBoxView(SdlDisplay &disp, FillBox &model) :
	SUPER(disp, model),
	surface()
{
}

SdlFillBoxView::~SdlFillBoxView()
{
	if (surface) {
		SDL_FreeSurface(surface);
	}
}

void SdlFillBoxView::OnModelUpdate(int prop)
{
	switch (prop) {
		case FillBox::Props::COLOR:
		case FillBox::Props::SIZE:
			if (surface) {
				SDL_FreeSurface(surface);
				surface = nullptr;
			}
			break;
	}
}

void SdlFillBoxView::PrepareRender()
{
	if (!surface) {
		const Vec2 &size = model.GetSize();
		surface = disp.CreateHardwareSurface(
			static_cast<int>(size.x), static_cast<int>(size.y));

		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

		const Color color = model.GetColor();
		SDL_FillRect(surface, nullptr,
			SDL_MapRGBA(surface->format,
				color.bits.r, color.bits.g, color.bits.b, 0xff));

		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

		// The alpha component is the same across the entire surface,
		// so we can use per-surface alpha.
		if (color.bits.a != 0xff) {
			SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, color.bits.a);
		}
	}
}

void SdlFillBoxView::Render()
{
	disp.DrawUiSurface(surface, model.GetPos());
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
