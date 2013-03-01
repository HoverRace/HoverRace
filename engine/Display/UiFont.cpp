
// UiFont.cpp
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

#ifdef WITH_SDL_PANGO
#	include <boost/format.hpp>
#	include <glib.h>
#endif

#include "../Util/OS.h"

#include "UiFont.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

#ifdef WITH_SDL_PANGO
// Render this font specification in Pango style.
std::ostream &UiFont::WritePango(std::ostream &os) const
{
	// Using the "C" locale here just in case, for some reason, we need
	// to use a font with a size in the thousands.  Pango doesn't
	// interpret a size of "4.000,00" properly.  :)
	static boost::format szFmt("%0.2f", OS::stdLocale);

	// Escape the font name since we're writing to Pango markup.
	char *escapedName = g_markup_escape_text(name.c_str(), -1);
	os << escapedName;
	g_free(escapedName);

	if (style & UiFont::BOLD) os << " Bold";
	if (style & UiFont::ITALIC) os << " Italic";

	os << ' ' << szFmt % size;

	return os;
}
#endif

}  // namespace Display
}  // namespace HoverRace
