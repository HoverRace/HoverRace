
// SdlDisplay.cpp
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
#include <SDL2/SDL_image.h>

#include "../../Util/Config.h"
#include "../../Util/Log.h"
#include "../../Util/Str.h"
#include "../../Exception.h"
#include "../ActiveText.h"
#include "../BaseContainer.h"
#include "../Button.h"
#include "../ClickRegion.h"
#include "../FillBox.h"
#include "../KeycapIcon.h"
#include "../Label.h"
#include "../Picture.h"
#include "../Res.h"
#include "../RuleLine.h"
#include "../ScreenFade.h"
#include "../Slider.h"
#include "../SymbolIcon.h"
#include "../UiFont.h"
#include "../Wallpaper.h"
#include "SdlActiveTextView.h"
#include "SdlBaseContainerView.h"
#include "SdlButtonView.h"
#include "SdlClickRegionView.h"
#include "SdlFillBoxView.h"
#include "SdlKeycapIconView.h"
#include "SdlLabelView.h"
#include "SdlPictureView.h"
#include "SdlRuleLineView.h"
#include "SdlScreenFadeView.h"
#include "SdlSliderView.h"
#include "SdlSymbolIconView.h"
#include "SdlTexture.h"
#include "SdlTypeCase.h"
#include "SdlLegacyDisplay.h"
#include "SdlWallpaperView.h"

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

/**
 * Defines aliases for font names.
 */
class FontAliasMap
{
public:
	FontAliasMap()
	{
		auto cfg = Config::GetInstance();
		map.emplace("", cfg->GetDefaultFontName());
		map.emplace("monospace", cfg->GetDefaultMonospaceFontName());
	}

public:
	/**
	 * Maps a possibly-aliased font name to it's real font name.
	 * @param name The name.
	 * @return The real font name.  If there is no alias, this is the name.
	 */
	const std::string &Lookup(const std::string &name)
	{
		const auto &iter = map.find(name);
		return iter == map.cend() ? name : iter->second;
	}

private:
	std::unordered_map<std::string, std::string> map;
};

/**
 * Wraps an istream in a SDL_RWops struct.
 */
class InputStreamRwOps
{
public:
	InputStreamRwOps(std::istream *is) :
		ops(SDL_AllocRW())
	{
		ops->size = &InputStreamRwOps::RSize;
		ops->seek = &InputStreamRwOps::RSeek;
		ops->read = &InputStreamRwOps::RRead;
		ops->write = &InputStreamRwOps::RWrite;
		ops->close = &InputStreamRwOps::RClose;
		ops->type = SDL_RWOPS_UNKNOWN;
		ops->hidden.unknown.data1 = this;
		ops->hidden.unknown.data2 = is;
	}

	~InputStreamRwOps()
	{
		if (ops) {
			SDL_FreeRW(ops);
		}
	}

private:
	static InputStreamRwOps *Self(SDL_RWops *ctx)
	{
		return static_cast<InputStreamRwOps*>(
			ctx->hidden.unknown.data1);
	}

	static std::istream *Stream(SDL_RWops *ctx)
	{
		return static_cast<std::istream*>(
			ctx->hidden.unknown.data2);
	}

	static MR_Int64 RSize(SDL_RWops *ctx)
	{
		auto is = Stream(ctx);
		auto pos = is->tellg();

		is->seekg(0, std::ios::end);
		if (!is) return -1;

		MR_Int64 retv = is->tellg();
		is->seekg(pos, std::ios::beg);
		return is ? retv : -1;
	}

	static MR_Int64 RSeek(SDL_RWops *ctx, MR_Int64 offset, int whence)
	{
		auto is = Stream(ctx);
		switch (whence) {
			case RW_SEEK_SET:
				is->seekg(offset, std::ios::beg);
				break;
			case RW_SEEK_CUR:
				is->seekg(offset, std::ios::cur);
				break;
			case RW_SEEK_END:
				is->seekg(offset, std::ios::end);
				break;
			default: {
				std::ostringstream oss;
				oss << "Unknown SDL_RWops seek whence: " << whence;
				throw UnimplementedExn(oss.str());
			}
		}
		return is ? static_cast<MR_Int64>(is->tellg()) : -1;
	}

	static size_t RRead(SDL_RWops *ctx, void *ptr,
		size_t size, size_t maxnum)
	{
		auto is = Stream(ctx);
		auto buf = static_cast<char*>(ptr);
		size_t num = 0;
		for (size_t i = 0; i < maxnum; i++) {
			is->read(buf, static_cast<std::streamsize>(size));
			if (is) {
				buf += size;
				num++;
			}
			else {
				break;
			}
		}
		return num;
	}

	static size_t RWrite(SDL_RWops*, const void*, size_t, size_t)
	{
		return 0;
	}

	static int RClose(SDL_RWops *ctx)
	{
		auto self = Self(ctx);
		SDL_FreeRW(self->ops);
		self->ops = nullptr;
		return 0;
	}

public:
	SDL_RWops *ops;
};

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

/**
 * Searches for the closest supported display mode.
 * @param displayIdx Zero-based index of the display.
 * @param w The width.
 * @param h The height.
 * @param refreshRate The refresh rate.
 * @return The found display mode, if available.
 */
boost::optional<SDL_DisplayMode> FindDisplayMode(int displayIdx,
	int w, int h, int refreshRate)
{
	SDL_DisplayMode req;
	req.format = 0;
	req.w = w;
	req.h = h;
	req.refresh_rate = refreshRate;
	req.driverdata = nullptr;

	SDL_DisplayMode closest;
	if (SDL_GetClosestDisplayMode(displayIdx, &req, &closest)) {
		return { closest };
	}
	else {
		return {};
	}
}

/**
 * Searches for the best fullscreen mode.
 * @param displayIdx Zero-based index of the display.
 * @return The found best fullscreen mode, if available.
 */
boost::optional<SDL_DisplayMode> FindBestFullscreenMode(int displayIdx)
{
	const auto &vidCfg = Config::GetInstance()->video;

	if (auto mode = FindDisplayMode(displayIdx,
		vidCfg.xResFullscreen, vidCfg.yResFullscreen,
		vidCfg.fullscreenRefreshRate))
	{
		return mode;
	}

	HR_LOG(warning) << "No fullscreen mode for " << vidCfg.xResFullscreen <<
		'x' << vidCfg.yResFullscreen << " (" << vidCfg.fullscreenRefreshRate <<
		" Hz on display " << displayIdx << "; trying default.";

	if (auto mode = FindDisplayMode(displayIdx, 1280, 720, 0)) {
		return mode;
	}

	HR_LOG(warning) << "No fullscreen mode for 1280x720; picking the first "
		"fullscreen mode we can find.";

	int numRes = SDL_GetNumDisplayModes(displayIdx);
	if (numRes < 1) {
		HR_LOG(error) << "No fullscreen resolutions found; "
			"disabling fullscreen mode.";
		return {};
	}

	for (int i = 0; i < numRes; i++) {
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(displayIdx, i, &mode) < 0) {
			HR_LOG(warning) << "Failed to retrieve resolution info "
				"for monitor " << displayIdx << " and mode " << i;
			continue;
		}

		return { mode };
	}

	HR_LOG(error) << "No available display modes for monitor " << displayIdx <<
		"; disabling fullscreen mode";
	return {};
}

/**
 * Activates fullscreen mode.
 * @param window The window (may not be @c nullptr).
 * @param mode The fullscreen display mode.
 * @return @c true if it succeeds, @c false if it fails.
 */
bool ActivateFullscreen(SDL_Window *window, SDL_DisplayMode &mode)
{
	assert(window);

	if (SDL_SetWindowDisplayMode(window, &mode) < 0) {
		HR_LOG(error) << "Unable to set fullscreen mode: " <<
			SDL_GetError();
		return false;
	}

	if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) < 0) {
		HR_LOG(error) << "Unable to enable fullscreen mode: " <<
			SDL_GetError();
		return false;
	}

	return true;
}

}  // namespace

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

	legacyDisplay = new SdlLegacyDisplay(*this);
	SUPER::OnDisplayConfigChanged();
	legacyDisplay->CreatePalette();
}

SdlDisplay::~SdlDisplay()
{
	delete legacyDisplay;

	for (auto &entry : loadedFonts) {
		TTF_CloseFont(entry.second);
	}

	if (renderer) SDL_DestroyRenderer(renderer);
	if (window) SDL_DestroyWindow(window);
}

void SdlDisplay::AttachView(ActiveText &model)
{
	model.SetView(std::unique_ptr<View>(new SdlActiveTextView(*this, model)));
}

void SdlDisplay::AttachView(BaseContainer &model)
{
	model.SetView(std::unique_ptr<View>(new SdlBaseContainerView(*this, model)));
}

void SdlDisplay::AttachView(Button &model)
{
	model.SetView(std::unique_ptr<View>(new SdlButtonView(*this, model)));
}

void SdlDisplay::AttachView(ClickRegion &model)
{
	model.SetView(std::unique_ptr<View>(new SdlClickRegionView(*this, model)));
}

void SdlDisplay::AttachView(KeycapIcon &model)
{
	model.SetView(std::unique_ptr<View>(new SdlKeycapIconView(*this, model)));
}

void SdlDisplay::AttachView(FillBox &model)
{
	model.SetView(std::unique_ptr<View>(new SdlFillBoxView(*this, model)));
}

void SdlDisplay::AttachView(Label &model)
{
	model.SetView(std::unique_ptr<View>(new SdlLabelView(*this, model)));
}

void SdlDisplay::AttachView(Picture &model)
{
	model.SetView(std::unique_ptr<View>(new SdlPictureView(*this, model)));
}

void SdlDisplay::AttachView(RuleLine &model)
{
	model.SetView(std::unique_ptr<View>(new SdlRuleLineView(*this, model)));
}

void SdlDisplay::AttachView(ScreenFade &model)
{
	model.SetView(std::unique_ptr<View>(new SdlScreenFadeView(*this, model)));
}

void SdlDisplay::AttachView(Slider &model)
{
	model.SetView(std::unique_ptr<View>(new SdlSliderView(*this, model)));
}

void SdlDisplay::AttachView(SymbolIcon &model)
{
	model.SetView(std::unique_ptr<View>(new SdlSymbolIconView(*this, model)));
}

void SdlDisplay::AttachView(Wallpaper &model)
{
	model.SetView(std::unique_ptr<View>(new SdlWallpaperView(*this, model)));
}

std::shared_ptr<TypeCase> SdlDisplay::MakeTypeCase(const UiFont &font)
{
	// Decide texture size based on font size.
	int sz;
	if (font.size < 15) {
		sz = 128;
	}
	else if (font.size < 25) {
		sz = 256;
	}
	else {
		sz = 512;
	}

	return std::make_shared<SdlTypeCase>(*this, font, sz, sz);
}

/**
 * Loads a texture resource.
 * @param res The resource (may be @c nullptr).
 * @return The loaded resource.
 * @throw ResLoadExn
 */
std::shared_ptr<SdlTexture> SdlDisplay::LoadRes(std::shared_ptr<Res<Texture>> res)
{
	SDL_Surface *surface = nullptr;

	if (!res) {
		// Return the default texture.
		// Generally, this shouldn't happen.
		// We generate a new instance of the default texture in case something
		// modifies the texture with SDL_UpdateTexture().
		surface = SDL_CreateRGBSurface(0, 2, 2, 32, 0, 0, 0, 0);
		if (!surface) {
			throw ResLoadExn(
				std::string("Failed to create default texture surface: ") +
				SDL_GetError());
		}

#		ifdef _DEBUG
			//TODO: Fill in the surface with a checkerboard pattern.
#		endif
	}
	else if (res->IsGenerated()) {
		auto imageData = res->GetImageData();
		if (!imageData) {
			throw ResLoadExn(res->GetId() + ": "
				"Generated image has no image data");
		}

		switch (imageData->depth) {
			case 8:
			case 16:
			case 32:
				break;
			default: {
				std::ostringstream oss;
				oss << res->GetId() << ": Unsupported color depth: " <<
					imageData->depth;
				throw ResLoadExn(oss.str());
			}
		}

		surface = SDL_CreateRGBSurfaceFrom(imageData->pixels.get(),
			imageData->width, imageData->height,
			imageData->depth, imageData->pitch,
			imageData->rMask, imageData->gMask,
			imageData->bMask, imageData->aMask);
		if (!surface) {
			throw ResLoadExn(res->GetId() + ": " + SDL_GetError());
		}

		// Assign palette if 8-bit.
		if (imageData->depth == 8) {
			if (SDL_SetPaletteColors(surface->format->palette,
				GetLegacyDisplay().GetPalette(), 0, 256) < 0)
			{
				throw ResLoadExn(res->GetId() + ": " + SDL_GetError());
			}
		}
	}
	else {
		auto is = res->Open();
		InputStreamRwOps ops(is.get());

		surface = IMG_Load_RW(ops.ops, 1);
		if (!surface) {
			throw ResLoadExn(res->GetId() + ": " + IMG_GetError());
		}
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) {
		throw ResLoadExn(res->GetId() + ": " + SDL_GetError());
	}

	return std::make_shared<SdlTexture>(*this, texture);
}

void SdlDisplay::OnDesktopModeChanged(int width, int height)
{
	legacyDisplay->OnDesktopModeChange(width, height);
}

void SdlDisplay::OnDisplayConfigChanged()
{
	const auto &vidCfg = Config::GetInstance()->video;

	bool resChanged = (vidCfg.xRes != width || vidCfg.yRes != height);

	//TODO: Determine what settings actually changed and decide if resources
	// need to be reloaded.

	if (resChanged) {
		width = vidCfg.xRes;
		height = vidCfg.yRes;

		// Clear the loaded font cache, since resizing will change the
		// sizes of the UI-scaled text.
		for (auto &entry : loadedFonts) {
			TTF_CloseFont(entry.second);
		}
		loadedFonts.clear();

		SUPER::OnDisplayConfigChanged();
	}
}

void SdlDisplay::Flip()
{
	SDL_RenderPresent(renderer);
}

void SdlDisplay::Screenshot()
{
	const auto cfg = Config::GetInstance();

	OS::path_t path = cfg->GenerateScreenshotPath(".bmp");

	SDL_Surface *surface =
		SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	if (!surface) {
		throw Exception(std::string("Unable to create screenshot surface: ") +
			SDL_GetError());
	}

	if (SDL_RenderReadPixels(renderer, nullptr,
		SDL_PIXELFORMAT_ARGB8888,
		surface->pixels, surface->pitch) < 0)
	{
		SDL_FreeSurface(surface);
		throw Exception(std::string("Failed reading screen for screenshot: ") +
			SDL_GetError());
	}

	if (SDL_SaveBMP(surface, Str::PU(path)) < 0) {
		SDL_FreeSurface(surface);
		throw Exception(std::string("Failed to save screenshot: ") +
			SDL_GetError());
	}

	HR_LOG(info) << "Saved screenshot: " << path;

	SDL_FreeSurface(surface);
}

/**
 * Apply the current video configuration to the display.
 */
void SdlDisplay::ApplyVideoMode()
{
	auto *cfg = Config::GetInstance();
	auto &vidCfg = cfg->video;

	SDL_Rect winRect = { vidCfg.xPos, vidCfg.yPos, vidCfg.xRes, vidCfg.yRes };

	MR_UInt32 flags = SDL_WINDOW_RESIZABLE;
	boost::optional<SDL_DisplayMode> fullscreenMode;
	if (vidCfg.fullscreen) {
		// We don't set SDL_WINDOW_FULLSCREEN since we want to specify the
		// display and mode for fullscreen.
		//TODO: Support SDL_WINDOW_FULLSCREEN_DESKTOP.

		int idx = vidCfg.fullscreenMonitorIndex;
		if (idx >= SDL_GetNumVideoDisplays()) {
			HR_LOG(warning) << "Configured monitor (" << idx << ") is not "
				"available; using monitor 0.";
			vidCfg.fullscreenMonitorIndex = idx = 0;
		}

		fullscreenMode = FindBestFullscreenMode(idx);
		if (!fullscreenMode) {
			vidCfg.fullscreen = false;
		}
		else {
			SDL_Rect bounds;
			if (SDL_GetDisplayBounds(idx, &bounds) == 0) {
				winRect = bounds;
			}
			else {
				HR_LOG(warning) << "Could not get bounds for monitor " << idx <<
					"; disabling fullscreen.";
				vidCfg.fullscreen = false;
			}
		}
	}

	// First try to enable OpenGL support, otherwise go on without it.
	if (cfg->runtime.noAccel ||
		(window = SDL_CreateWindow(windowTitle.c_str(),
			winRect.x, winRect.y, winRect.w, winRect.h,
			flags | SDL_WINDOW_OPENGL)) == nullptr)
	{
		if ((window = SDL_CreateWindow(windowTitle.c_str(),
			winRect.x, winRect.y, winRect.w, winRect.h,
			flags)) == nullptr)
		{
			throw Exception(SDL_GetError());
		}
	}

	// Activate fullscreen mode if requested.
	if (vidCfg.fullscreen) {
		if (!ActivateFullscreen(window, *fullscreenMode)) {
			vidCfg.fullscreen = false;
			// Restore configured position and size.
			SDL_SetWindowPosition(window, vidCfg.xPos, vidCfg.yPos);
			SDL_SetWindowSize(window, vidCfg.xRes, vidCfg.yRes);
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
	for (const RendererInfo &info : renderers) {
		HR_LOG(info) << "renderer[" << i << "]: " << info.info;
		i++;
	}

	// Find a working renderer.
	for (auto iter = renderers.begin(); iter != renderers.end(); ++iter) {
		if ((renderer = SDL_CreateRenderer(window, iter->idx, 0)) != nullptr) {
			SDL_RendererInfo info;
			SDL_GetRendererInfo(renderer, &info);
			HR_LOG(info) << "Selected renderer: " << info;

			// Track whether we're in SW mode, so we can make appropriate
			// quality decisions later.
			bool noAccel = !(info.flags & SDL_RENDERER_ACCELERATED);
			if (cfg->runtime.noAccel != noAccel) {
				cfg->runtime.noAccel = noAccel;
				styles.Reload();
			}

			break;
		}
	}

	if (!renderer) {
		throw Exception("Unable to find a compatible renderer");
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")) {
		// If this fails, then SDL will just use the default ("nearest").
		HR_LOG(info) << "Failed to set \"linear\" render scale quality.";
	}

	// We keep track of the current state of the window so
	// OnDisplayConfigChanged() can determine if anything special
	// needs to be done.
	width = vidCfg.xRes;
	height = vidCfg.yRes;
}

/**
 * Load the TTF font for a given name and size.
 * The font is cached for future retrievals.
 * @param font The font specification.
 * @param uiScale Apply the user-selected scaling.
 *                False is useful if scaling will be applied later.
 * @return The loaded SDL_ttf font (never @c nullptr).
 * @throws HoverRace::Exception The font could not be loaded.
 */
TTF_Font *SdlDisplay::LoadTtfFont(const UiFont &font, bool uiScale)
{
	static FontAliasMap aliasMap;

	const Config *cfg = Config::GetInstance();

	// Scale the font size to match the DPI we used in SDL_Pango.
	// SDL_ttf always assumes a DPI of 75.
	double dsize = font.size * 60.0;
	if (uiScale) dsize *= cfg->video.textScale;
	int size = static_cast<int>(dsize / 75.0);

	std::string fullFontName = aliasMap.Lookup(font.name);
	if (font.isBold()) fullFontName += "Bold";
	if (font.isItalic()) fullFontName += "Oblique";
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

/**
 * Blit an SDL texture to the backbuffer with the current layout state.
 * @param texture The texture to blit (may be @c NULL).
 * @param relPos The UI-space position, relative to the current UI origin.
 * @param layoutFlags Optional layout flags, from the view model.
 */
void SdlDisplay::DrawUiTexture(SDL_Texture *texture, const Vec2 &relPos,
	uiLayoutFlags_t layoutFlags)
{
	HR_UNUSED(layoutFlags);

	if (texture) {
		Vec2 adjustedPos = LayoutUiPosition(relPos);

		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
		SDL_Rect destRect = { (int)adjustedPos.x, (int)adjustedPos.y, w, h };
		SDL_RenderCopy(renderer, texture, nullptr, &destRect);
	}
}

/**
 * Blit a scaled SDL texture to the backbuffer with the current layout state.
 * @param texture The texture to blit (may be @c NULL).
 * @param relPos The UI-space position, relative to the current UI origin.
 * @param scale The scaling to apply to the texture.
 * @param layoutFlags Optional layout flags, from the view model.
 */
void SdlDisplay::DrawUiTexture(SDL_Texture *texture, const Vec2 &relPos,
	const Vec2 &scale, uiLayoutFlags_t layoutFlags)
{
	HR_UNUSED(layoutFlags);

	if (texture) {
		Vec2 adjustedPos = LayoutUiPosition(relPos);

		int w, h;
		SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

		w = static_cast<int>(static_cast<double>(w) * scale.x);
		h = static_cast<int>(static_cast<double>(h) * scale.y);

		SDL_Rect destRect = { (int)adjustedPos.x, (int)adjustedPos.y, w, h };
		SDL_RenderCopy(renderer, texture, nullptr, &destRect);
	}
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
