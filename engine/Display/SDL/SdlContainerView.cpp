
// SdlContainerView.cpp
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

#include "../Container.h"

#include "SdlContainerView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlContainerView::PrepareRender()
{
	auto &children = model.GetChildren();
	if (children.empty()) return;

	std::for_each(children.begin(), children.end(),
		std::mem_fn(&ViewModel::PrepareRender));
}

void SdlContainerView::Render()
{
	auto &children = model.GetChildren();
	if (children.empty()) return;

	bool clip = model.IsClip();
	SDL_Rect oldClip;
	SDL_Renderer *renderer = display.GetRenderer();

	if (clip) {
		// Translate our bounds into screen-space.
		Vec2 pos = display.LayoutUiPosition(
			model.GetAlignedPos(model.GetSize().x, model.GetSize().y));

		const Vec2 &size = model.GetSize();
		double w = size.x;
		double h = size.y;
		if (!model.IsLayoutUnscaled()) {
			double uiScale = display.GetUiScale();
			w *= uiScale;
			h *= uiScale;
		}

		SDL_Rect clipRect = {
			static_cast<int>(pos.x), static_cast<int>(pos.y),
			static_cast<int>(w), static_cast<int>(h) };

		SDL_RenderGetClipRect(renderer, &oldClip);
		SDL_RenderSetClipRect(renderer, &clipRect);
	}

	Vec2 oldOrigin = display.AddUiOrigin(model.GetPos());
	std::for_each(children.begin(), children.end(),
		std::mem_fn(&ViewModel::Render));
	display.SetUiOrigin(oldOrigin);

	if (clip) {
		SDL_RenderSetClipRect(renderer, &oldClip);
	}
}

Vec3 SdlContainerView::Measure()
{
	return model.GetSize().Promote();
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
