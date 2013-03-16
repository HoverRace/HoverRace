
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
		SDL_Surface *tempSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			static_cast<int>(size.x), static_cast<int>(size.y), 32,
			(MR_UInt32)(255 << (8 * 3)),
			(MR_UInt32)(255 << (8 * 2)),
			(MR_UInt32)(255 << (8 * 1)),
			255);

		const Color color = model.GetColor();
		MR_UInt32 fillColor =
			((MR_UInt32)(color.bits.r) << 24) +
			((MR_UInt32)(color.bits.g) << 16) +
			((MR_UInt32)(color.bits.b) << 8) +
			color.bits.a;
		MR_UInt8 *buf = static_cast<MR_UInt8*>(tempSurface->pixels);
		for (int y = 0; y < tempSurface->h; y++) {
			MR_UInt32 *row = reinterpret_cast<MR_UInt32*>(buf);
			for (int x = 0; x < tempSurface->w; x++) {
				*row++ = fillColor;
			}
			buf += tempSurface->pitch;
		}

		surface = SDL_DisplayFormatAlpha(tempSurface);
		SDL_FreeSurface(tempSurface);
	}
}

void SdlFillBoxView::Render()
{
	disp.DrawUiSurface(surface, model.GetPos());
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
