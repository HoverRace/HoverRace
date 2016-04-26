
// SdlSymbolIconView.cpp
//
// Copyright (c) 2013-2016 Michael Imamura.
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

#include <SDL2/SDL.h>

#include <utf8/utf8.h>

#include "../../Util/Config.h"
#include "../../Util/Str.h"
#include "../SymbolIcon.h"
#include "../TypeCase.h"
#include "SdlSurfaceText.h"

#include "SdlSymbolIconView.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlSymbolIconView::SdlSymbolIconView(SdlDisplay &disp, SymbolIcon &model) :
	SUPER(disp, model),
	width(0), height(0)
{
	uiScaleChangedConnection = disp.GetUiScaleChangedSignal().connect(
		std::bind(&decltype(typeLine)::reset, &typeLine, nullptr));
}

SdlSymbolIconView::~SdlSymbolIconView()
{
}

void SdlSymbolIconView::OnModelUpdate(int prop)
{
	switch (prop) {
		case Box::Props::SIZE:
		case SymbolIcon::Props::SYMBOL:
			typeLine.reset();
			break;
	}
}

Vec3 SdlSymbolIconView::Measure()
{
	return model.GetSize().Promote();
}

void SdlSymbolIconView::PrepareRender()
{
	if (!typeLine) {
		UpdateTexture();
	}
}

void SdlSymbolIconView::Render()
{
	if (typeLine) {
		Vec2 aligned = display.LayoutUiPosition(
			model.GetAlignedPos(unscaledWidth, unscaledHeight));
		typeLine->Render(
			model.GetColor(),
			static_cast<int>(aligned.x),
			static_cast<int>(aligned.y));
	}
}

void SdlSymbolIconView::UpdateTexture()
{
	double scale = 1.0;

	Config *cfg = Config::GetInstance();

	// Scale the symbol font to fit the bounds.
	double iconFontSize = model.GetSize().y * 1.20;

	UiFont font(cfg->GetDefaultSymbolFontName(), iconFontSize);
	if (!model.IsLayoutUnscaled()) {
		font.size *= (scale = display.GetUiScale());
	}

	int wtext[] = { model.GetSymbol() };
	std::string text;
	utf8::utf32to8(wtext, wtext + 1, std::back_inserter(text));

	std::unique_ptr<TypeLine> newLine(new TypeLine());
	auto typeCase = display.GetTypeCase(font);
	typeCase->Prepare(text, newLine.get());
	typeLine.swap(newLine);

	auto &glyphRect = typeLine->glyphs[0].first->srcRect;
	width = glyphRect.w;
	height = glyphRect.h;

	// We pre-scale the texture (by adjusting the font size) so that would
	// normally throw off the size adjustments later, so we need to keep track
	// of what the size would be (approximately) if we hadn't pre-scaled the
	// texture.
	unscaledWidth = width / scale;
	unscaledHeight = height / scale;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
