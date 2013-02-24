
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

#include "../../Util/Config.h"
#include "../../Exception.h"
#include "../Label.h"
#include "SdlLabelView.h"
#include "SdlLegacyDisplay.h"

#include "SdlDisplay.h"

using namespace HoverRace::Util;
using HoverRace::VideoServices::VideoBuffer;

namespace HoverRace {
namespace Display {
namespace SDL {

SdlDisplay::SdlDisplay() :
	SUPER(),
	legacyDisplay(new SdlLegacyDisplay())
{
	ApplyVideoMode();

	legacyDisplay->OnWindowResChange();
	legacyDisplay->CreatePalette();
}

SdlDisplay::~SdlDisplay()
{
}

void SdlDisplay::AttachView(Label &model)
{
	model.SetView(std::unique_ptr<View>(new SdlLabelView(*this, model)));
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
		legacyDisplay->OnWindowResChange();
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

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
