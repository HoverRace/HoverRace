
// VideoBuffer.cpp
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
//

#include "../StdAfx.h"

#include <SDL2/SDL.h>

#include "../Display/Display.h"
#include "../Exception.h"
#include "../Util/Config.h"
#include "../Util/OS.h"

#include "VideoBuffer.h"

using HoverRace::Util::Config;
using HoverRace::Util::OS;

namespace HoverRace {
namespace VideoServices {

/**
 * Constructor.
 * @param display The display this is connected to.
 */
VideoBuffer::VideoBuffer(Display::Display &display) :
	desktopWidth(0), desktopHeight(0), width(0), height(0), pitch(0),
	fullscreen(false),
	legacySurface(nullptr), vbuf(nullptr), zbuf(nullptr),
	bgPalette()
{
	// Be notified of window resizes so we can update the internal surface.
	display.GetDisplayConfigChangedSignal().connect(
		std::bind(&VideoBuffer::OnWindowResChange, this));
}

VideoBuffer::~VideoBuffer()
{
	delete[] zbuf;
	if (legacySurface) {
		SDL_FreeSurface(legacySurface);
		delete[] vbuf;
	}
}

/**
 * Notify the video buffer that the desktop resolution has changed.
 * This should be called immediately after the video buffer instance is created
 * and every time the desktop resolution changes.
 * @param width The desktop resolution width.
 * @param height The desktop resolution height.
 */
void VideoBuffer::OnDesktopModeChange(int width, int height)
{
	desktopWidth = width;
	desktopHeight = height;
}

/**
 * Notify the video buffer that the window size has changed.
 */
void VideoBuffer::OnWindowResChange()
{
	const auto &vidCfg = Config::GetInstance()->video;
	width = vidCfg.xRes;
	height = vidCfg.yRes;
	pitch = width;

	int remainder = width % 4;
	if (remainder > 0) pitch += (4 - remainder);

	// The size of the legacy surface matches the screen surface,
	// just has a different bit depth.
	// We're allocating our own buffer to work around an issue where on X11
	// SDL doesn't seem to allocate a large enough buffer itself (??).
	if (legacySurface != NULL) {
		SDL_FreeSurface(legacySurface);
		delete[] vbuf;
	}
	vbuf = new MR_UInt8[height * pitch];
	legacySurface = SDL_CreateRGBSurfaceFrom(vbuf, width, height, 8, pitch, 0, 0, 0, 0);
	AssignPalette();

	delete[] zbuf;
	zbuf = new MR_UInt16[width * height];
}

void VideoBuffer::CreatePalette()
{
	const auto &vidcfg = Config::GetInstance()->video;
	double gamma = vidcfg.gamma;
	double contrast = vidcfg.contrast;
	double brightness = vidcfg.brightness;

	if (gamma < 0.2) gamma = 0.2;
	else if (gamma > 4.0) gamma = 4.0;

	if (contrast > 1.0) contrast = 1.0;
	else if (contrast < 0.3) contrast = 0.3;

	if (brightness > 1.0) brightness = 1.0;
	else if (brightness < 0.3) brightness = 0.3;

	const double invGamma = 1.0 / gamma;
	const double intensity = contrast * brightness;
	const double baseIntensity = brightness - intensity;

	ColorPalette::paletteEntry_t newPalette[256] = { { 0, 0, 0, 0 } };

	ColorPalette::paletteEntry_t *basicPalette =
		ColorPalette::GetColors(invGamma, intensity, baseIntensity);
	memcpy(newPalette + MR_RESERVED_COLORS_BEGINNING, basicPalette,
		MR_BASIC_COLORS * sizeof(ColorPalette::paletteEntry_t));
	delete[] basicPalette;

	if (bgPalette) {
		ColorPalette::paletteEntry_t *pal = newPalette + MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS;
		for (int i = 0; i < MR_BACK_COLORS; i++) {
			*pal++ = ColorPalette::ConvertColor(
				bgPalette[i * 3], bgPalette[i * 3 + 1], bgPalette[i * 3 + 2],
				invGamma, intensity, baseIntensity);
		}
	}

	memcpy(palette, newPalette, 256 * sizeof(ColorPalette::paletteEntry_t));
	AssignPalette();

	// Notify listeners that the palette changed.
	paletteChangedSignal();
}

void VideoBuffer::SetBackgroundPalette(std::unique_ptr<MR_UInt8[]> &palette)
{
	bgPalette = std::move(palette);
	CreatePalette();
}

void VideoBuffer::AssignPalette()
{
	if (legacySurface != NULL) {
		SDL_SetPaletteColors(legacySurface->format->palette, palette, 0, 256);
	}
}

VideoBuffer::pixelMeter_t VideoBuffer::GetPixelMeter() const
{
	// Assume square pixels.
	return pixelMeter_t(1, 1);
}

void VideoBuffer::LockLegacySurface()
{
	if (SDL_MUSTLOCK(legacySurface)) {
		if (SDL_LockSurface(legacySurface) < 0) {
			throw Exception("Unable to lock legacy surface");
		}
	}
}

void VideoBuffer::UnlockLegacySurface()
{
	if (SDL_MUSTLOCK(legacySurface)) {
		SDL_UnlockSurface(legacySurface);
	}

	Flip();
}

void VideoBuffer::Clear(MR_UInt8 color)
{
	memset(legacySurface->pixels, color, legacySurface->pitch * legacySurface->h);
}

}  // namespace VideoServices
}  // namespace HoverRace
