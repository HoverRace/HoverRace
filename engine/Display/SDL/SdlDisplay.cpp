
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

#include <SDL2/SDL.h>
#ifdef WITH_SDL_PANGO
#	include <SDL_Pango.h>
#endif

#include "../../Util/Config.h"
#include "../../Util/Log.h"
#include "../../Util/Str.h"
#include "../../Exception.h"
#include "../Button.h"
#include "../ClickRegion.h"
#include "../Container.h"
#include "../FillBox.h"
#include "../Label.h"
#include "../ScreenFade.h"
#include "../SymbolIcon.h"
#include "../UiFont.h"
#include "SdlButtonView.h"
#include "SdlClickRegionView.h"
#include "SdlContainerView.h"
#include "SdlFillBoxView.h"
#include "SdlLabelView.h"
#include "SdlScreenFadeView.h"
#include "SdlSymbolIconView.h"
#include "SdlLegacyDisplay.h"

#include "SdlDisplay.h"

using namespace HoverRace::Util;
using HoverRace::VideoServices::VideoBuffer;

static std::ostream &operator<<(std::ostream &oss, const SDL_RendererInfo &info) {
	oss << info.name << " (";
	if (info.flags & SDL_RENDERER_SOFTWARE) oss << ":SW";
	if (info.flags & SDL_RENDERER_ACCELERATED) oss << ":Accel";
	if (info.flags & SDL_RENDERER_PRESENTVSYNC) oss << ":VSync";
	if (info.flags & SDL_RENDERER_TARGETTEXTURE) oss << ":RTT";
	oss << ')';

	if (info.max_texture_width != 0 || info.max_texture_height != 0) {
		oss << " Texture: " <<
			info.max_texture_width << "x" << info.max_texture_height;
	}

	return oss;
}

namespace HoverRace {
namespace Display {
namespace SDL {

namespace {
	struct RendererInfo {
		RendererInfo(int idx) : idx(idx), score(0) {
			SDL_GetRenderDriverInfo(idx, &info);

			// Blacklisting the Direct3D driver since we prefer an OpenGL one.
			// This also fixes issue #201 where SDL_SetTextureAlphaMod seems to stop
			// working after a screen resize.
			blacklisted = (strncmp(info.name, "direct3d", 8) == 0);

			bool noAccel = Config::GetInstance()->runtime.noAccel;
			if (noAccel && (info.flags & SDL_RENDERER_ACCELERATED)) blacklisted = true;

			score += (blacklisted ? 1000 : 9000);

			if (info.flags & SDL_RENDERER_ACCELERATED) score += 900;
			else if (info.flags & SDL_RENDERER_SOFTWARE) score += 100;
			else score += 500;

			if (info.flags & SDL_RENDERER_TARGETTEXTURE) score += 50;

			if (info.flags & SDL_RENDERER_PRESENTVSYNC) score += 5;
		}

		SDL_RendererInfo info;
		int idx;
		int score;
		bool blacklisted;
	};

	bool operator<(const RendererInfo &a, const RendererInfo &b) {
		if (a.score == b.score) {
			return strcmp(a.info.name, b.info.name) < 0;
		}
		else {
			return b.score < a.score;
		}
	}
}

/**
 * Constructor.
 * This will create a new window with the specified title.
 * Initial size and position will be pulled from the Config.
 * @param windowTitle The window title.
 */
SdlDisplay::SdlDisplay(const std::string &windowTitle) :
	SUPER(), windowTitle(windowTitle), window(nullptr), renderer(nullptr),
	legacyDisplay(nullptr)
{
	ApplyVideoMode();

	Config *cfg = Config::GetInstance();

#	ifdef _WIN32
		//TODO: Get the filename from the config.
		LoadPrivateFont("fontawesome/fontawesome-webfont.ttf");
#	endif

	legacyDisplay = new SdlLegacyDisplay(*this);
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
	delete legacyDisplay;

#	ifdef WITH_SDL_PANGO
		if (pangoContext) SDLPango_FreeContext(pangoContext);
#	elif defined(WITH_SDL_TTF)
		BOOST_FOREACH(auto &entry, loadedFonts) {
			TTF_CloseFont(entry.second);
		}
#	endif
	if (renderer) SDL_DestroyRenderer(renderer);
	if (window) SDL_DestroyWindow(window);
}

void SdlDisplay::AttachView(Button &model)
{
	model.SetView(std::unique_ptr<View>(new SdlButtonView(*this, model)));
}

void SdlDisplay::AttachView(ClickRegion &model)
{
	model.SetView(std::unique_ptr<View>(new SdlClickRegionView(*this, model)));
}

void SdlDisplay::AttachView(Container &model)
{
	model.SetView(std::unique_ptr<View>(new SdlContainerView(*this, model)));
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

void SdlDisplay::AttachView(SymbolIcon &model)
{
	model.SetView(std::unique_ptr<View>(new SdlSymbolIconView(*this, model)));
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
		//TODO: As of the 2013-06-01 (rev 7250) SDL2 snapshot, this is no longer
		// required.  Leaving it in for now since some people may try to build
		// with an older snapshot.
		// It should be removed when SDL2 is finally released.
		SDL_RenderSetViewport(renderer, nullptr);

		width = vidCfg.xRes;
		height = vidCfg.yRes;

#		ifdef WITH_SDL_TTF
			// Clear the loaded font cache, since resizing will change the
			// sizes of the UI-scaled text.
			BOOST_FOREACH(auto &entry, loadedFonts) {
				TTF_CloseFont(entry.second);
			}
			loadedFonts.clear();
#		endif

		SUPER::OnDisplayConfigChanged();
	}
}

void SdlDisplay::Flip()
{
	SDL_RenderPresent(renderer);
}

/**
 * Apply the current video configuration to the display.
 */
void SdlDisplay::ApplyVideoMode()
{
	const Config *cfg = Config::GetInstance();
	const Config::cfg_video_t &vidCfg = cfg->video;

	// First try to enable OpenGL support, otherwise go on without it.
	if (cfg->runtime.noAccel ||
		!(window = SDL_CreateWindow(windowTitle.c_str(),
		vidCfg.xPos, vidCfg.yPos, vidCfg.xRes, vidCfg.yRes,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL)))
	{
		if (!(window = SDL_CreateWindow(windowTitle.c_str(),
			vidCfg.xPos, vidCfg.yPos, vidCfg.xRes, vidCfg.yRes,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)))
		{
			throw Exception(SDL_GetError());
		}
	}

	// Try to find a usable renderer.
	// We prefer an accelerated one, but we'll allow fallback to software.

	std::set<RendererInfo> renderers;
	int numDrivers = SDL_GetNumRenderDrivers();
	for (int i = 0; i < numDrivers; i++) {
		renderers.insert(RendererInfo(i));
	}

	int i = 0;
	BOOST_FOREACH(const RendererInfo &info, renderers) {
		Log::Info("renderer[%d]: %s", i++, boost::lexical_cast<std::string>(info.info).c_str());
	}

	// Find a working renderer.
	for (auto iter = renderers.begin(); iter != renderers.end(); ++iter) {
		if (renderer = SDL_CreateRenderer(window, iter->idx, 0)) {
			SDL_RendererInfo info;
			SDL_GetRendererInfo(renderer, &info);
			Log::Info("Selected renderer: %s", boost::lexical_cast<std::string>(info).c_str());
			break;
		}
	}

	if (!renderer) {
		throw Exception("Unable to find a compatible renderer");
	}

	// We keep track of the current state of the window so
	// OnDisplayConfigChanged() can determine if anything special
	// needs to be done.
	width = vidCfg.xRes;
	height = vidCfg.yRes;
}

#ifdef WITH_SDL_TTF
/**
 * Load the TTF font for a given name and size.
 * The font is cached for future retrievals.
 * @param font The font specification.
 * @return The loaded SDL_ttf font (never @c nullptr).
 * @throws HoverRace::Exception The font could not be loaded.
 */
TTF_Font *SdlDisplay::LoadTtfFont(const UiFont &font)
{
	// Scale the font size to match the DPI we use in SDL_Pango.
	// SDL_ttf always assumes a DPI of 75.
	int size = font.size * 60 / 75;

	std::string fullFontName = font.name;
	if (font.style & UiFont::Style::BOLD) fullFontName += "Bold";
	if (font.style & UiFont::Style::ITALIC) fullFontName += "Oblique";
	fullFontName += ".ttf";
	loadedFontKey key(fullFontName, size);

	auto iter = loadedFonts.find(key);
	if (iter == loadedFonts.end()) {
		Config *cfg = Config::GetInstance();
		OS::path_t fontPath = cfg->GetMediaPath();
		fontPath /= Str::UP("fonts");
		fontPath /= Str::UP(fullFontName);

		TTF_Font *retv = TTF_OpenFont((const char*)Str::PU(fontPath), size);
		if (!retv) {
			throw Exception(TTF_GetError());
		}

		loadedFonts.insert(loadedFonts_t::value_type(key, retv));

		return retv;
	}
	else {
		return iter->second;
	}
}
#endif

#ifdef _WIN32
/**
 * Load a private (visible only to this process) font.
 * @param filename The filename of the font, relative to the media dir.
 */
void SdlDisplay::LoadPrivateFont(const std::string &filename)
{
	Config *cfg = Config::GetInstance();
	OS::path_t fontPath = cfg->GetMediaPath();
	fontPath /= Str::UP("fonts");
	fontPath /= Str::UP(filename);

	int fontsAdded = AddFontResourceExW((const wchar_t*)Str::PW(fontPath),
		FR_PRIVATE | FR_NOT_ENUM, 0);
	Log::Info("Loaded %d private font(s) from file: %s", fontsAdded, filename.c_str());
	if (fontsAdded < 1) {
		throw Exception("Failed to load private font: " + filename);
	}
}
#endif

/**
 * Blit an SDL texture to the backbuffer with the current layout state.
 * @param texture The texture to blit (may be @c NULL).
 * @param relPos The UI-space position, relative to the current UI origin.
 * @param layoutFlags Optional layout flags, from the view model.
 */
void SdlDisplay::DrawUiTexture(SDL_Texture *texture, const Vec2 &relPos,
                               uiLayoutFlags_t layoutFlags)
{
	if (texture) {
		Vec2 adjustedPos = LayoutUiPosition(relPos);

		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
		SDL_Rect destRect = { (int)adjustedPos.x, (int)adjustedPos.y, w, h };
		SDL_RenderCopy(renderer, texture, nullptr, &destRect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
