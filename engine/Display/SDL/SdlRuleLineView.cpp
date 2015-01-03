
// SdlRuleLineView.cpp
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

#include <SDL2/SDL.h>

#include "../RuleLine.h"

#include "SdlRuleLineView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlRuleLineView::Render()
{
	const Color color = model.GetColor();
	if (color.bits.a > 0) {
		SDL_Renderer *renderer = display.GetRenderer();
		SDL_SetRenderDrawColor(renderer,
			color.bits.r, color.bits.g, color.bits.b, color.bits.a);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		Vec3 size = model.Measure();
		Vec2 pos = display.LayoutUiPosition(
			model.GetAlignedPos(size.x, size.y));

		double w = size.x;
		double h = size.y;
		if (!model.IsLayoutUnscaled()) {
			double uiScale = display.GetUiScale();
			w *= uiScale;
			h *= uiScale;
		}

		int iw = static_cast<int>(w);
		int ih = static_cast<int>(h);

		// The defining characteristic of RuleLines are that regardless of the
		// scaling, the line is always at least one pixel thick.
		if (model.GetDirection() == RuleLine::Direction::H) {
			if (ih == 0) ih = 1;
		}
		else {
			if (iw == 0) iw = 1;
		}

		SDL_Rect rect = {
			static_cast<int>(pos.x), static_cast<int>(pos.y), iw, ih };
		SDL_RenderFillRect(renderer, &rect);
	}
}

Vec3 SdlRuleLineView::Measure()
{
	return model.Measure();
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
