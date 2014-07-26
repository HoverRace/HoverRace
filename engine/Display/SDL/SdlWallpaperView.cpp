
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

namespace {

void CalcFill(SDL_Texture *tex, int destW, int destH, SDL_Rect &dest)
{
	int w, h;
	if (SDL_QueryTexture(tex, nullptr, nullptr, &w, &h) < 0) {
		throw Exception(SDL_GetError());
	}

	double wd = (double)w;
	double hd = (double)h;
	double destWd = (double)destW;
	double destHd = (double)destH;

	double srcRatio = wd / hd;
	double destRatio = destWd / destHd;
	double zoom = (srcRatio > destRatio) ? (destHd / hd) : (destWd / wd);
	int zw = (int)(wd * zoom);
	int zh = (int)(hd * zoom);

	dest.x = (destW - zw) / 2;
	dest.y = (destH - zh) / 2;
	dest.w = zw;
	dest.h = zh;
}

}  // namespace

SdlWallpaperView::SdlWallpaperView(SdlDisplay &disp, Wallpaper &model) :
	SUPER(disp, model), fillChanged(true), opacityChanged(true),
	computedAlpha(0), destRectPtr(nullptr)
{
	displayConfigChangedConn =
		disp.GetDisplayConfigChangedSignal().connect([&](int, int) {
			fillChanged = true;
		});
}

void SdlWallpaperView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Background::Props::OPACITY:
		case Wallpaper::Props::COLOR:
			opacityChanged = true;
			break;
		case Wallpaper::Props::FILL:
			fillChanged = true;
			break;
	}
}

Vec3 SdlWallpaperView::Measure()
{
	return Vec3(display.GetScreenWidth(), display.GetScreenHeight(), 0);
}

void SdlWallpaperView::PrepareRender()
{
	if (!texture) {
		texture = display.LoadRes(model.GetTexture());
	}
	if (fillChanged) {
		switch (model.GetFill()) {
			case Wallpaper::Fill::ZOOM:
				CalcFill(texture->Get(), display.GetScreenWidth(),
					display.GetScreenHeight(), destRect);
				destRectPtr = &destRect;
				break;

			case Wallpaper::Fill::STRETCH:
				destRectPtr = nullptr;
				break;

			default: {
				std::ostringstream oss;
				oss << "SdlWallpaperView::PrepareRender: fill=" <<
					static_cast<int>(model.GetFill());
				throw UnimplementedExn(oss.str());
			}
		}
	}
	if (opacityChanged) {
		double alpha = static_cast<double>(model.GetColor().bits.a);
		double opacity = model.GetOpacity();
		computedAlpha = static_cast<MR_UInt8>(opacity * alpha);
	}
}

void SdlWallpaperView::Render()
{
	if (computedAlpha > 0) {
		Color color = model.GetColor();

		SDL_Renderer *renderer = display.GetRenderer();
		SDL_Texture *tex = texture->Get();
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(tex, computedAlpha);
		SDL_SetTextureColorMod(tex, color.bits.r, color.bits.g, color.bits.b);
		SDL_RenderCopy(renderer, tex, nullptr, destRectPtr);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
