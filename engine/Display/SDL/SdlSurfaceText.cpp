
// SdlSurfaceText.cpp
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

#include "../../StdAfx.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../../Util/SelFmt.h"
#include "../../Util/Str.h"
#include "../../Exception.h"
#include "../Label.h"

#include "SdlSurfaceText.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {
namespace SDL {

/**
 * Constructor.
 * @param display The current display.
 */
SdlSurfaceText::SdlSurfaceText(SdlDisplay &display) :
	display(display),
	font(), color(COLOR_WHITE), wrapWidth(-1),
	width(0), height(0)
{
}

/**
 * Constructor with initial font and color.
 * @param display The current display.
 * @param font The font to render with.
 * @param color The text foreground color.
 */
SdlSurfaceText::SdlSurfaceText(SdlDisplay &display, const UiFont &font,
                               const Color color) :
	display(display),
	font(font), color(color), wrapWidth(-1),
	width(0), height(0)
{
}

/**
 * Calculate the line-height of text using the current font.
 *
 * This is useful when needing to treat an empty string as if it were being
 * rendered, since normally an empty string would be a zero-by-zero size.
 *
 * This will set the height to the line-height and the width to 1.
 *
 * @return The line-height (also available via GetHeight).
 */
int SdlSurfaceText::MeasureLineHeight()
{
	TTF_Font *ttfFont = display.LoadTtfFont(font);

	width = 1;
	height = TTF_FontHeight(ttfFont);

	return height;
}

/**
 * Create a new SDL surface and render text onto it.
 * The new SDL surface will be sized to fit the rendered text.
 * @param s The text to render (may not be blank).
 * @return The newly-created surface.
 */
SDL_Surface *SdlSurfaceText::RenderToNewSurface(const std::string &s)
{
	SDL_Surface *tempSurface = nullptr;

	bool fixedWidth = wrapWidth > 0;

	TTF_Font *ttfFont = display.LoadTtfFont(font);

	//TODO: Handle newlines ourselves.
	SDL_Color color = { 0xff, 0xff, 0xff, 0xff };

	tempSurface = TTF_RenderUTF8_Blended_Wrapped(ttfFont,
		s.c_str(), color,
		fixedWidth ? static_cast<unsigned>(wrapWidth) : 4096);
	width = tempSurface->w;
	height = tempSurface->h;

	return tempSurface;
}

/**
 * Render text onto an existing surface.
 * @param dest The destination surface (may not be @c nullptr).
 * @param x The horizontal offset.
 * @param y The vertical offset.
 * @param s The text to render (may not be blank).
 * @return The same surface.
 */
SDL_Surface *SdlSurfaceText::RenderToSurface(SDL_Surface *dest,
                                             int x, int y,
                                             const std::string &s)
{
	SDL_Rect destRect = { x, y, 0, 0 };

	SDL_Surface *src = RenderToNewSurface(s);
	SDL_BlitSurface(src, nullptr, dest, &destRect);
	SDL_FreeSurface(src);

	return dest;
}

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace
