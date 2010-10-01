
// NumericGlyphs.cpp
// Collection of number-related StaticText glyphs.
//
// Copyright (c) 2009 Michael Imamura.
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

#include "FontSpec.h"
#include "StaticText.h"

#include "NumericGlyphs.h"

namespace HoverRace {
namespace VideoServices {

/**
 * Constructor.
 * @param font The font.
 * @param color The color index.
 */
NumericGlyphs::NumericGlyphs(const FontSpec &font, MR_UInt8 color)
{
	for (int i = 0; i < 32; ++i) {
		glyphs[i] = NULL;
	}

	const char *s = " ',-.0123456789";
	char bf[2] = { 0, 0 };
	while (*s) {
		bf[0] = *s;
		glyphs[*s - 32] = new StaticText(bf, font, color);
		++s;
	}
}

NumericGlyphs::~NumericGlyphs()
{
	for (int i = 0; i < 32; ++i) {
		delete glyphs[i];
	}
}

void NumericGlyphs::SetFont(const HoverRace::VideoServices::FontSpec &font)
{
	for (int i = 0; i < 32; ++i) {
		glyphs[i]->SetFont(font);
	}
}

void NumericGlyphs::SetColor(MR_UInt8 color)
{
	for (int i = 0; i < 32; ++i) {
		glyphs[i]->SetColor(color);
	}
}

/**
 * Retrieve the glyph for a character.
 * @param c The ASCII character.
 * @return The StaticText instance for the character or NULL if not available.
 */
const StaticText *NumericGlyphs::GetGlyph(char c) const
{
	return (c < 32 || c >= 64) ? NULL : glyphs[c - 32];
}

}  // namespace VideoServices
}  // namespace HoverRace
