
// MultipartText.h
// Header for text widget with static and dynamic formatted portions.
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

#include <boost/format.hpp>

#include "../Util/MR_Types.h"
#include "FontSpec.h"

namespace HoverRace {
namespace VideoServices {

class NumericGlyphs;

/**
 * Like StaticText, but can contain static (constant string) parts
 * as well as dynamic (formatted number) parts rendered via a glyph set.
 * @author Michael Imamura
 */
class MR_DllDeclare MultipartText
{
	public:
		MultipartText(const FontSpec &font,
			const NumericGlyphs *glyphs,
			MR_UInt8 color=43);
		~MultipartText();

		const FontSpec &GetFont() const;
		const NumericGlyphs *GetGlyphs() const;
		const MR_UInt8 GetColor() const;

		void Blt(int x, int y, Viewport2D *vp, ...) const;

	private:
		class Part;
		class StringPart;
		class FormatPart;
		void AddPart(Part *part);
		void AddStringPart(const char *s);

	private:
		FontSpec font;
		const NumericGlyphs *glyphs;
		MR_UInt8 color;
		typedef std::vector<Part*> parts_t;
		parts_t parts;
		StringPart *lastStringPart;

	public:
		friend MR_DllDeclare MultipartText &operator<<(MultipartText &self, const char *s);
		friend MR_DllDeclare MultipartText &operator<<(MultipartText &self, const std::string &s);
		friend MR_DllDeclare MultipartText &operator<<(MultipartText &self, const boost::format &fmt);
};

}  // namespace VideoServices
}  // namespace HoverRace
