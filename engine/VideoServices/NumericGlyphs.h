
// NumericGlyphs.h
// Header for collection of number-related StaticText glyphs.
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

#pragma once

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace VideoServices {

struct Font;
class StaticText;

/**
 * A collection of number-related glyphs as StaticText instances.
 * @author Michael Imamura.
 */
class MR_DllDeclare NumericGlyphs
{
	public:
		NumericGlyphs(const HoverRace::VideoServices::Font &font,
			MR_UInt8 color=43);
		virtual ~NumericGlyphs();

		void SetFont(const HoverRace::VideoServices::Font &font);
		void SetColor(MR_UInt8 color);

		const HoverRace::VideoServices::StaticText *GetGlyph(char c);

	private:
		HoverRace::VideoServices::StaticText *glyphs[32];
};

}  // namespace VideoServices
}  // namespace HoverRace
