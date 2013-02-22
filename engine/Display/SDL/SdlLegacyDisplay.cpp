
// SdlLegacyDisplay.cpp
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

#include "../../Exception.h"

#include "SdlLegacyDisplay.h"

namespace HoverRace {
namespace Display {
namespace SDL {

void SdlLegacyDisplay::Flip()
{
	// Convert the legacy surface to the bit depth of the screen surface,
	// then blit.  This isn't particularly fast, but the speed isn't
	// the point of the legacy display :)

	SDL_Surface *screenSurface = SDL_GetVideoSurface();
	SDL_Surface *legacySurface = GetLegacySurface();
	if (legacySurface != NULL) {
		SDL_Surface *tempSurface = SDL_DisplayFormat(legacySurface);
		SDL_BlitSurface(tempSurface, NULL, screenSurface, NULL);
		SDL_FreeSurface(tempSurface);
	}

	SDL_Flip(screenSurface);
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
