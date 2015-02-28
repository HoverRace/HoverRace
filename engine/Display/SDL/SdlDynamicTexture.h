
// SdlDynamicTexture.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../../Exception.h"

#include "SdlTexture.h"

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
 * Wrapper for an SDL texture that can be updated.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlDynamicTexture : public SdlTexture
{
	using SUPER = SdlTexture;

public:
	SdlDynamicTexture() = delete;
	SdlDynamicTexture(SdlDisplay &display, SDL_Texture *texture);
	SdlDynamicTexture(SdlDisplay &display, int width, int height);
	SdlDynamicTexture(const SdlDynamicTexture&) = delete;

	virtual ~SdlDynamicTexture()
	{
		if (surface) SDL_FreeSurface(surface);
	}

	SdlDynamicTexture &operator=(const SdlDynamicTexture&) = delete;

public:
	/**
	 * Check if this texture has changes that haven't been updated yet.
	 *
	 * Call Update() to write the commit the changes to the texture.
	 *
	 * @return @c true if dirty, @c false if not.
	 */
	bool IsDirty() const { return dirty; }

public:
	/**
	 * Updates the texture from the backing surface.
	 * @return @c true if an update was performed (dirty flag was set),
	 *         @c false if no update was needed.
	 */
	bool Update()
	{
		if (dirty) {
			SDL_UpdateTexture(texture, nullptr,
				surface->pixels, surface->pitch);
			dirty = false;
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Updates the texture with new pixel data.
	 * @param fn The update function.  Will be passed the drawing surface.
	 *           Returns @c true to update the texture immediately,
	 *           or @c false to defer the update until a later call to Update().
	 */
	template<class Fn>
	void Update(Fn fn)
	{
		dirty = true;

		bool retv;
		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
		try {
			retv = static_cast<bool>(fn(surface));
		}
		catch (...) {
			if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
			throw;
		}
		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
		if (retv) Update();
	}

protected:
	SDL_Surface *surface;
	bool dirty;
};

/**
 * Thrown when the dynamic texture could not be created.
 * @author Michael Imamura
 */
class DynamicTextureExn : public Exception
{
	typedef Exception SUPER;

public:
	DynamicTextureExn() : SUPER() { }
	DynamicTextureExn(const std::string &msg) : SUPER(msg) { }
	DynamicTextureExn(const char *msg) : SUPER(msg) { }
	DynamicTextureExn(const std::string &msg, const std::string &sdlError) :
		SUPER(msg + sdlError) { }
	virtual ~DynamicTextureExn() noexcept { }
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
