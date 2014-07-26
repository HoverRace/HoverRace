
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

#include "../../Util/Log.h"

#include "../Container.h"

#include "SdlContainerView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlContainerView::PrepareRender()
{
	if (!model.IsVisible()) return;

	auto &children = model.GetChildren();
	if (children.empty()) return;

	std::for_each(children.begin(), children.end(),
		std::mem_fn(&ViewModel::PrepareRender));
}

void SdlContainerView::Render()
{
	if (!model.IsVisible()) return;

	auto &children = model.GetChildren();
	if (children.empty()) return;

	bool clip = model.IsClip();
	SDL_Rect oldClip = { 0 };
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

		SDL_RenderGetClipRect(renderer, &oldClip);
		SDL_Rect ourClip = {
			static_cast<int>(pos.x), static_cast<int>(pos.y),
			static_cast<int>(w), static_cast<int>(h) };
		SDL_Rect clipRect = { 0 };

		// If there's an existing clip area, then set the clip rect to be
		// the intersection of the two areas.
		if (oldClip.w == 0 || oldClip.h == 0) {
			clipRect = ourClip;
		}
		else {
			if (!SDL_IntersectRect(&oldClip, &ourClip, &clipRect)) {
				// If there's no intersection, set a tiny off-screen rect so
				// the existing clip area check still works.
				clipRect.x = -1;
				clipRect.y = -1;
				clipRect.w = 1;
				clipRect.h = 1;
			}
		}

		/*
		Util::Log::Info("Clip: <%d, %d> (%d x %d)",
			clipRect.x, clipRect.y,
			clipRect.w, clipRect.h);
		*/

		SDL_RenderSetClipRect(renderer, &clipRect);
	}

	const Vec2 &size = model.GetSize();
	Vec2 oldOrigin = display.AddUiOrigin(model.GetAlignedPos(size.x, size.y));
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
