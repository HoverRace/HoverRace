
// SdlTexture.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#pragma once

#include <SDL2/SDL.h>

#include "../Texture.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		namespace SDL {
			class SdlDisplay;
		}
	}
}

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * Wrapper for SDL_Texture.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlTexture : public Texture
{
	typedef Texture SUPER;
	public:
		SdlTexture(SdlDisplay &display, SDL_Texture *texture) :
			SUPER(), texture(texture), display(display) { }
		SdlTexture(const SdlTexture&) = delete;

		virtual ~SdlTexture()
		{
			if (texture) SDL_DestroyTexture(texture);
		}

		SdlTexture &operator=(const SdlTexture&) = delete;

	public:
		SDL_Texture *Get() const { return texture; }

	protected:
		SDL_Texture *texture;
		SdlDisplay &display;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
