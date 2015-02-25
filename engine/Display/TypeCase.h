
// TypeCase.h
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

#include <SDL2/SDL.h>

#include "UiFont.h"

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

class TypeCase;

/**
 * A single glyph in the backing texture.
 * @author Michael Imamura
 */
class GlyphEntry
{
public:
	GlyphEntry() : page(0) { srcRect.h = -1; }

public:
	bool IsInitialized() const { return srcRect.h < 0; }

public:
	unsigned int page;  ///< Index of the texture that contains the character.
	SDL_Rect srcRect;  ///< The bounds of the glyph in the texture.
};

/**
 * A line of text, prepared by a TypeCase.
 *
 * This is intended to be a reusable buffer; i.e., it can be cleared and a
 * new prepared line of text loaded in place.
 *
 * @author Michael Imamura
 */
class TypeLine
{
public:
	TypeLine() : typeCase(nullptr) { }

public:
	TypeCase *typeCase;  // Owning TypeCase, for sanity checking.
	std::vector<GlyphEntry*> glyphs;
};

/**
 * Character-by-character text renderer.
 * @author Michael Imamura
 */
class MR_DllDeclare TypeCase
{
public:
	/**
	 * Constructor.
	 * @param font The font.
	 * @param width The width of the backing texture.
	 * @param height The height of the backing texture.
	 */
	TypeCase(const UiFont &font, int width, int height) :
		font(font), width(width), height(height) { }
	TypeCase(const TypeCase&) = delete;

	virtual ~TypeCase() { }

	TypeCase &operator=(const TypeCase&) = delete;

public:
	/**
	 * Prepare a string to be rendered.
	 *
	 * If @p rects is omitted, then this can be used to preemptively add
	 * characters to the type case before they are used later.
	 *
	 * @param s The text string that will be rendered.
	 * @param[out] rects Optional destination buffer for the prepared text.
	 */
	virtual void Prepare(const std::string &s, TypeLine *rects = nullptr) = 0;

	/**
	 * Render a string.
	 *
	 * Only characters which have been previously added via AddChars() will
	 * be rendered.
	 *
	 * @param s The string to render.
	 * @param x The screen X coordinate of the upper-left corner.
	 * @param y The screen Y coordinate of the upper-left corner.
	 */
	virtual void Render(const TypeLine &s, int x, int y) = 0;

protected:
	const UiFont font;
	const int width;
	const int height;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
