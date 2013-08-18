
// SdlSymbolIconView.cpp
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

#include "../../Util/Config.h"
#include "../../Util/Str.h"
#include "../SymbolIcon.h"
#include "SdlSurfaceText.h"

#include "SdlSymbolIconView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlSymbolIconView::SdlSymbolIconView(SdlDisplay &disp, SymbolIcon &model) :
	SUPER(disp, model),
	texture(), colorChanged(true), width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&SdlSymbolIconView::OnUiScaleChanged, this));
}

SdlSymbolIconView::~SdlSymbolIconView()
{
	if (texture) {
		SDL_DestroyTexture(texture);
	}
}

void SdlSymbolIconView::OnModelUpdate(int prop)
{
	switch (prop) {
		case FillBox::Props::COLOR:
			colorChanged = true;
			break;

		case FillBox::Props::SIZE:
		case SymbolIcon::Props::SYMBOL:
			if (texture) {
				SDL_DestroyTexture(texture);
				texture = nullptr;
			}
			break;
	}
}

void SdlSymbolIconView::OnUiScaleChanged()
{
	if (texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

Vec3 SdlSymbolIconView::Measure()
{
	return model.GetSize().Promote();
}

void SdlSymbolIconView::PrepareRender()
{
	if (!texture) {
		UpdateTexture();
	} else if (colorChanged) {
		UpdateTextureColor();
	}
}

void SdlSymbolIconView::Render()
{
	int w, h;
	SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
	disp.DrawUiTexture(texture,
		model.GetAlignedPos(unscaledWidth, unscaledHeight),
		model.GetLayoutFlags());
}

void SdlSymbolIconView::UpdateTexture()
{
	if (texture) SDL_DestroyTexture(texture);

	double scale = 1.0;

	Config *cfg = Config::GetInstance();

	UiFont font(cfg->GetDefaultSymbolFontName(), model.GetSize().y);
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = disp.GetUiScale());
	}

	wchar_t wtext[2] = { model.GetSymbol(), 0 };
#	ifdef _WIN32
		std::wstring text = wtext;
#	else
		std::string text = (const char*)Str::WU(wtext);
#	endif

	// Render the text onto a fresh new surface.
	SdlSurfaceText textRenderer(disp, font);
	SDL_Surface *tempSurface = textRenderer.RenderToNewSurface(text);
	width = textRenderer.GetWidth();
	height = textRenderer.GetHeight();

	// Convert the surface to the display format.
	texture = SDL_CreateTextureFromSurface(disp.GetRenderer(), tempSurface);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(tempSurface);

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;

	UpdateTextureColor();
}

void SdlSymbolIconView::UpdateTextureColor()
{
	const Color cm = model.GetColor();
	SDL_SetTextureColorMod(texture, cm.bits.r, cm.bits.g, cm.bits.b);
	SDL_SetTextureAlphaMod(texture, cm.bits.a);

	colorChanged = false;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
