
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

#include "../../StdAfx.h"

#include <SDL2/SDL.h>

#include "../../Exception.h"

#include "SdlLegacyDisplay.h"

namespace HoverRace {
namespace Display {
namespace SDL {

SdlLegacyDisplay::~SdlLegacyDisplay()
{
	if (nativeSurface) {
		SDL_FreeSurface(nativeSurface);
	}
	if (texture) {
		SDL_DestroyTexture(texture);
	}
}

void SdlLegacyDisplay::OnWindowResChange()
{
	SUPER::OnWindowResChange();

	if (nativeSurface) {
		SDL_FreeSurface(nativeSurface);
	}
	if (texture) {
		SDL_DestroyTexture(texture);
	}

	// The pixel format is selected via SDL_CreateRGBSurface, then we make sure we
	// use the same pixel format for SDL_CreateTexture, so the later
	// SDL_ConvertPixels is fast (in theory).

	nativeSurface = SDL_CreateRGBSurface(0, GetWidth(), GetHeight(), 32,
		0, 0, 0, 0);
	if (!nativeSurface) {
		throw Exception(SDL_GetError());
	}

	texture = SDL_CreateTexture(sdlDisplay.GetRenderer(),
		nativeSurface->format->format, SDL_TEXTUREACCESS_STREAMING,
		GetWidth(), GetHeight());
	if (!texture) {
		throw Exception(SDL_GetError());
	}
}

void SdlLegacyDisplay::Flip()
{
	// Convert the legacy surface to the bit depth of the screen surface,
	// then blit.

	SDL_Surface *legacySurface = GetLegacySurface();
	if (legacySurface && texture) {
		SDL_Renderer *renderer = sdlDisplay.GetRenderer();

		// We can't use SDL_ConvertPixels to convert from an indexed format, so
		// we use SDL_BlitSurface to do that, using the nativeSurface as a
		// temporary buffer.
		SDL_BlitSurface(legacySurface, nullptr, nativeSurface, nullptr);

		if (SDL_MUSTLOCK(nativeSurface)) {
			if (SDL_LockSurface(nativeSurface) < 0) {
				throw Exception(SDL_GetError());
			}
		}

		void *pixels;
		int pitch;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) < 0) {
			throw Exception(SDL_GetError());
		}

		MR_UInt32 destFmt;
		SDL_QueryTexture(texture, &destFmt, nullptr, nullptr, nullptr);

		// Copy the (already converted) pixels from nativeSurface to the
		// streaming texture.
		// Since the nativeSurface and destination texture use the same pixel
		// format, this is effectively a memcpy with a some sanity checks.
		if (SDL_ConvertPixels(nativeSurface->w, nativeSurface->h,
			nativeSurface->format->format, nativeSurface->pixels, nativeSurface->pitch,
			destFmt, pixels, pitch) < 0)
		{
			throw Exception(SDL_GetError());
		}

		SDL_UnlockTexture(texture);

		if (SDL_MUSTLOCK(nativeSurface)) {
			SDL_UnlockSurface(nativeSurface);
		}

		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
