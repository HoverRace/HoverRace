
// SymbolIcon.cpp
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

#include "../StdAfx.h"

#include "../Util/Log.h"

#include "SymbolIcon.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param size The size of the icon, where @c x is the width
 *             and @c y is the height.
 * @param symbol The symbol ID.
 * @param color The color of the icon (including alpha).
 * @param layoutFlags Optional layout flags.
 */
SymbolIcon::SymbolIcon(const Vec2 &size, int symbol, Color color,
	uiLayoutFlags_t layoutFlags) :
	SUPER(size, color, layoutFlags),
	symbol(symbol)
{
}

/**
 * Constructor.
 * @param w The width of the icon.
 * @param h The height of the icon.
 * @param symbol The symbol ID.
 * @param color The color of the icon (including alpha).
 * @param layoutFlags Optional layout flags.
 */
SymbolIcon::SymbolIcon(double w, double h, int symbol, Color color,
	uiLayoutFlags_t layoutFlags) :
	SUPER(w, h, color, layoutFlags),
	symbol(symbol)
{
}

void SymbolIcon::SetSymbol(int symbol)
{
	if (this->symbol != symbol) {

		// The symbol ID is implemented as a wchar_t, which may be 16 bits wide
		// (e.g. on Win32) so we make sure the symbol is within the safe range.
		if (symbol <= 0 || symbol > 65535) {
			Log::Warn("Invalid symbol (expected to be 1..65535): %d", symbol);
			symbol = 32;
		}

		this->symbol = symbol;
		FireModelUpdate(Props::SYMBOL);
	}
}

}  // namespace Display
}  // namespace HoverRace
