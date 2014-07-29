
// SdlDisplay.h
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
#include <SDL2/SDL_ttf.h>

#include "../Display.h"
#include "../Res.h"
#include "../UiViewModel.h"

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
			class SdlTexture;
		}
		class Label;
		class Texture;
		struct UiFont;
	}
}

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * SDL-based software rendering.
 * @author Michael Imamura
 */
class MR_DllDeclare SdlDisplay : public Display
{
	typedef Display SUPER;
	public:
		SdlDisplay(const std::string &windowTitle="");
		virtual ~SdlDisplay();

	public:
		// ViewAttacher
		virtual void AttachView(Button &model);
		virtual void AttachView(ClickRegion &model);
		virtual void AttachView(Container &model);
		virtual void AttachView(FillBox &model);
		virtual void AttachView(Label &model);
		virtual void AttachView(Picture &model);
		virtual void AttachView(RuleLine &model);
		virtual void AttachView(ScreenFade &model);
		virtual void AttachView(SymbolIcon &model);
		virtual void AttachView(Wallpaper &model);

	public:
		std::shared_ptr<SdlTexture> LoadRes(std::shared_ptr<Res<Texture>> res);

	public:
		// Display
		virtual VideoServices::VideoBuffer &GetLegacyDisplay() const { return *legacyDisplay; }
		virtual void OnDesktopModeChanged(int width, int height);
		virtual void OnDisplayConfigChanged();
		virtual void Flip();
		void Screenshot() override;

	private:
		void ApplyVideoMode();

	public:
		int GetScreenWidth() const { return width; }
		int GetScreenHeight() const { return height; }

		SDL_Window *GetWindow() const { return window; }
		SDL_Renderer *GetRenderer() const { return renderer; }

	public:
		// Text-renderer-specific utilities.
		TTF_Font *LoadTtfFont(const UiFont &font);

	public:
		// SDL-specific utilities.
		void DrawUiTexture(SDL_Texture *texture, const Vec2 &relPos,
			uiLayoutFlags_t layoutFlags=0);

	private:
		std::string windowTitle;
		SDL_Window *window;
		SDL_Renderer *renderer;
		int width, height;
		VideoServices::VideoBuffer *legacyDisplay;

		typedef std::pair<std::string, int> loadedFontKey;
		typedef std::map<loadedFontKey, TTF_Font*> loadedFonts_t;
		loadedFonts_t loadedFonts;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
