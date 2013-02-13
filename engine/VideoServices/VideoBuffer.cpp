// VideoBuffer.cpp
// Rendering system.
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
//

#include "StdAfx.h"

#include "VideoBuffer.h"

#include "../Exception.h"
#include "../Util/Config.h"
#include "../Util/OS.h"

using HoverRace::Util::Config;
using HoverRace::Util::OS;

namespace HoverRace {
namespace VideoServices {

VideoBuffer::VideoBuffer() :
	desktopWidth(0), desktopHeight(0), width(0), height(0), pitch(0),
	fullscreen(false),
	zbuf(NULL), vbuf(NULL),
	bgPalette()
{
}

VideoBuffer::~VideoBuffer()
{
	delete[] zbuf;
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
	SDL_Surface *surface = SDL_GetVideoSurface();
	width = surface->w;
	height = surface->h;
	pitch = surface->pitch;

	delete[] zbuf;
	zbuf = new MR_UInt16[width * height];
}

void VideoBuffer::CreatePalette()
{
	Config::cfg_video_t &vidcfg = Config::GetInstance()->video;
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

	ColorPalette::paletteEntry_t newPalette[256] = { 0 };

	ColorPalette::paletteEntry_t *basicPalette =
		ColorPalette::GetColors(invGamma, intensity, baseIntensity);
	memcpy(newPalette + MR_RESERVED_COLORS_BEGINNING, basicPalette,
		MR_BASIC_COLORS * sizeof(ColorPalette::paletteEntry_t));
	delete[] basicPalette;

	if (bgPalette != NULL) {
		ColorPalette::paletteEntry_t *pal = newPalette + MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS;
		for (int i = 0; i < MR_BACK_COLORS; i++) {
			*pal++ = ColorPalette::ConvertColor(
				bgPalette[i * 3], bgPalette[i * 3 + 1], bgPalette[i * 3 + 2],
				invGamma, intensity, baseIntensity);
		}
	}

	memcpy(palette, newPalette, 256 * sizeof(ColorPalette::paletteEntry_t));
}

void VideoBuffer::SetBackgroundPalette(std::unique_ptr<MR_UInt8[]> &palette)
{
	bgPalette = std::move(palette);

	CreatePalette();
}

void VideoBuffer::AssignPalette()
{
	SDL_Surface *surface = SDL_GetVideoSurface();
	if (surface != NULL) {
		SDL_SetPalette(surface, SDL_PHYSPAL, palette, 0, 256);
	}
}

VideoBuffer::pixelMeter_t VideoBuffer::GetPixelMeter() const
{
	if (fullscreen) {
		return pixelMeter_t(width * 3, height * 4);
	}
	else {
		return pixelMeter_t(desktopWidth * 3, desktopHeight * 4);
	}
}

void VideoBuffer::LockScreenSurface()
{
	SDL_Surface *surface = SDL_GetVideoSurface();

	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0) {
			throw Exception("Unable to lock surface");
		}
	}

	vbuf = static_cast<MR_UInt8*>(surface->pixels);
}

void VideoBuffer::UnlockScreenSurface()
{
	SDL_Surface *surface = SDL_GetVideoSurface();

	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}

	Flip();
}

void VideoBuffer::Flip()
{
	SDL_Flip(SDL_GetVideoSurface());
}

void VideoBuffer::Clear(MR_UInt8 color)
{
	SDL_Surface *surface = SDL_GetVideoSurface();
	memset(surface->pixels, color, surface->pitch * surface->w);
}

}  // namespace VideoServices
}  // namespace HoverRace
