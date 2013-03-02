
// UiFont.h
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

#pragma once

#include "../Util/SelFmt.h"

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
namespace Display {

/**
 * Describes a font used for the UI.
 * @author Michael Imamura
 */
struct MR_DllDeclare UiFont
{
	UiFont(const std::string &name="Arial", double size=20.0, int style=0) :
		name(name), size(size), style(style) { }

	enum Style {
		BOLD = 0x01,
		ITALIC = 0x02,
	};

#	ifdef WITH_SDL_PANGO
		std::ostream &WritePango(std::ostream &os) const;
#	endif

	std::string name;
	double size;
	int style;
};

MR_DllDeclare inline bool operator==(const UiFont &a, const UiFont &b)
{
	return
		a.size == b.size &&
		a.style == b.style &&
		a.name == b.name;
}

MR_DllDeclare inline bool operator!=(const UiFont &a, const UiFont &b)
{
	return !operator==(a, b);
}

MR_DllDeclare inline std::ostream &operator<<(std::ostream &os,
                                              const UiFont &fs)
{
	switch (Util::GetSelFmt(os)) {
		case Util::SEL_FMT_PANGO:
#			ifdef WITH_SDL_PANGO
				return fs.WritePango(os);
#			endif

		default:
			os << fs.name;
			if (fs.style & UiFont::BOLD) os << " Bold";
			if (fs.style & UiFont::ITALIC) os << " Italic";
			os << ' ' << fs.size;
	}

	return os;
}

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
