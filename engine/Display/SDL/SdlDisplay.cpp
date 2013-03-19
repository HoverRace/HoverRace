
// SdlDisplay.cpp
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
#ifdef WITH_SDL_PANGO
#	include <SDL_Pango.h>
#endif

#include "../../Util/Config.h"
#include "../../Exception.h"
#include "../FillBox.h"
#include "../Label.h"
#include "../ScreenFade.h"
#include "SdlFillBoxView.h"
#include "SdlLabelView.h"
#include "SdlScreenFadeView.h"
#include "SdlLegacyDisplay.h"

#include "SdlDisplay.h"

using namespace HoverRace::Util;
using HoverRace::VideoServices::VideoBuffer;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlDisplay::SdlDisplay() :
	SUPER()
{
	ApplyVideoMode();

	legacyDisplay.reset(new SdlLegacyDisplay(*this));
	SUPER::OnDisplayConfigChanged();
	legacyDisplay->CreatePalette();

#	ifdef WITH_SDL_PANGO
		pangoContext = SDLPango_CreateContext();
		SDLPango_SetDpi(pangoContext, 60, 60);  // Match Windows native rendering sizing.
		SDLPango_SetDefaultColor(pangoContext, MATRIX_TRANSPARENT_BACK_WHITE_LETTER);
#	endif
}

SdlDisplay::~SdlDisplay()
{
#	ifdef WITH_SDL_PANGO
		SDLPango_FreeContext(pangoContext);
#	endif
}

void SdlDisplay::AttachView(FillBox &model)
{
	model.SetView(std::unique_ptr<View>(new SdlFillBoxView(*this, model)));
}

void SdlDisplay::AttachView(Label &model)
{
	model.SetView(std::unique_ptr<View>(new SdlLabelView(*this, model)));
}

void SdlDisplay::AttachView(ScreenFade &model)
{
	model.SetView(std::unique_ptr<View>(new SdlScreenFadeView(*this, model)));
}

void SdlDisplay::OnDesktopModeChanged(int width, int height)
{
	legacyDisplay->OnDesktopModeChange(width, height);
}

void SdlDisplay::OnDisplayConfigChanged()
{
	Config::cfg_video_t &vidCfg = Config::GetInstance()->video;

	bool resChanged = (vidCfg.xRes != width || vidCfg.yRes != height);

	//TODO: Determine what settings actually changed and decide if resources
	// need to be reloaded.

	if (resChanged) {
		ApplyVideoMode();
		SUPER::OnDisplayConfigChanged();
	}
}

void SdlDisplay::Flip()
{
	SDL_Flip(SDL_GetVideoSurface());
}

/**
 * Apply the current video configuration to the display.
 */
void SdlDisplay::ApplyVideoMode()
{
	Config::cfg_video_t &vidCfg = Config::GetInstance()->video;

	if (SDL_SetVideoMode(vidCfg.xRes, vidCfg.yRes, 0,
		SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE) == NULL)
	{
		throw Exception(SDL_GetError());
	}

	// We keep track of the current state of the window so
	// OnDisplayConfigChanged() can determine if anything special
	// needs to be done.
	width = vidCfg.xRes;
	height = vidCfg.yRes;
}

/**
 * Create a screen-compatible hardware surface.
 * This is useful for avoiding the creation of a temporary surface, with the
 * caveat that the resulting surface will not have an alpha channel.
 * @param w The desired surface width.
 * @param h The desired surface height.
 * @return The surface (never @c nullptr).
 */
SDL_Surface *SdlDisplay::CreateHardwareSurface(int w, int h)
{
	SDL_Surface *vidSurface = SDL_GetVideoSurface();
	SDL_PixelFormat *pfmt = vidSurface->format;

	SDL_Surface *retv = SDL_CreateRGBSurface(
		vidSurface->flags & SDL_HWSURFACE,
		w, h,
		pfmt->BitsPerPixel,
		pfmt->Rmask, pfmt->Gmask, pfmt->Bmask, pfmt->Amask);

	if (!retv) throw Exception(SDL_GetError());

	return retv;
}

/**
 * Blit an SDL surface to the backbuffer.
 * @param surface The surface to blit (may be @c NULL).
 * @param relPos The UI-space position, relative to the current UI origin.
 */
void SdlDisplay::DrawUiSurface(SDL_Surface *surface, const Vec2 &relPos)
{
	if (surface) {
		Vec2 adjustedPos = relPos;
		adjustedPos += GetUiOrigin();

		//TODO: Convert UI coords to screen coords.

		SDL_Rect destRect = { (MR_Int16)adjustedPos.x, (MR_Int16)adjustedPos.y, 0, 0 };
		SDL_BlitSurface(surface, nullptr, SDL_GetVideoSurface(), &destRect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
