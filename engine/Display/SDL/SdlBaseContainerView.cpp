
// SdlBaseContainerView.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../../Util/Log.h"
#include "../BaseContainer.h"
#include "SdlTexture.h"

#include "SdlBaseContainerView.h"

namespace HoverRace {
namespace Display {
namespace SDL {

SdlBaseContainerView::SdlBaseContainerView(SdlDisplay &disp,
	BaseContainer &model) :
	SUPER(disp, model),
	screenPos(0, 0), screenSize(0, 0),
	rttChanged(true), rttSizeChanged(true)
{
	displayConfigChangedConn =
		disp.GetDisplayConfigChangedSignal().connect([&](int, int) {
			rttChanged = true;
			rttSizeChanged = true;
		});
}

SdlBaseContainerView::~SdlBaseContainerView()
{
}

void SdlBaseContainerView::OnModelUpdate(int prop)
{
	switch (prop) {
		case BaseContainer::Props::CLIP:
		case BaseContainer::Props::OPACITY:
			rttChanged = true;
			break;
	}
}

void SdlBaseContainerView::PrepareRender()
{
	if (!model.IsVisible() || model.IsEmpty()) return;

	// Determine if we need to render to a texture first then draw later.
	if (rttChanged) {
		bool rttNeeded = (model.GetOpacity() < 1.0);

		if (rttNeeded) {
			//TODO: Use fixed size of container if clipping is on.
			int width = display.GetScreenWidth();
			int height = display.GetScreenHeight();

			if (rttTarget) {
				//TODO: Check if texture size really needs to change.
			}

			if (rttSizeChanged || !rttTarget) {
				SDL_Texture *texture = SDL_CreateTexture(display.GetRenderer(),
					SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_TARGET,
					width, height);
				if (!texture) {
					std::ostringstream oss;
					oss << "Unable to create " << width << 'x' << height <<
						" target texture for container: " << SDL_GetError();
					throw Exception(oss.str());
				}

				rttTarget.reset(new SdlTexture(display, texture));
			}
		}
		else {
			if (rttTarget) rttTarget.release();
		}

		rttChanged = false;
		rttSizeChanged = false;
	}

	model.ForEachVisibleChild(std::mem_fn(&ViewModel::PrepareRender));
}

void SdlBaseContainerView::Render()
{
	if (!model.IsVisible() || model.IsEmpty()) return;

	SDL_Renderer *renderer = display.GetRenderer();

	//TODO: Currently, we assume that the render target is the whole screen.
	SDL_Texture *oldRenderTarget = nullptr;
	if (rttTarget) {
		oldRenderTarget = SDL_GetRenderTarget(renderer);
		SDL_SetRenderTarget(renderer, rttTarget->Get());
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
	}

	bool clip = model.IsClip();
	SDL_Rect oldClip = { 0, 0, 0, 0 };

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

		screenPos = pos;
		screenSize.x = w;
		screenSize.y = h;

		SDL_RenderGetClipRect(renderer, &oldClip);
		SDL_Rect ourClip = {
			static_cast<int>(pos.x), static_cast<int>(pos.y),
			static_cast<int>(w), static_cast<int>(h) };
		SDL_Rect clipRect = { 0, 0, 0, 0 };

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

		SDL_RenderSetClipRect(renderer, &clipRect);
	}

	const Vec2 &size = model.GetSize();
	Vec2 origin = model.GetAlignedPos(size.x, size.y);
	origin += model.GetChildOffset();

	Vec2 oldOrigin = display.AddUiOrigin(origin);
	auto oldFlags = display.AddUiLayoutFlags(model.GetLayoutFlags());
	model.ForEachVisibleChild(std::mem_fn(&ViewModel::Render));
	display.SetUiLayoutFlags(oldFlags);
	display.SetUiOrigin(oldOrigin);

	if (clip) {
		SDL_RenderSetClipRect(renderer, &oldClip);
	}

	if (rttTarget) {
		SDL_SetRenderTarget(renderer, oldRenderTarget);

		// Draw the render target with the selected opacity.
		SDL_Texture *tex = rttTarget->Get();
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(tex,
			static_cast<MR_UInt8>(255.0 * model.GetOpacity()));
		SDL_SetTextureColorMod(tex, 0xff, 0xff, 0xff);

		SDL_RenderCopy(renderer, tex, nullptr, nullptr);
	}
}

Vec3 SdlBaseContainerView::Measure()
{
	return model.GetSize().Promote();
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
