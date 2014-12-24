
// SymbolIcon.h
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

#include "FillBox.h"

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
		class Display;
	}
}

namespace HoverRace {
namespace Display {

/**
 * One of the predefined icons from the symbol font.
 * @author Michael Imamura
 */
class MR_DllDeclare SymbolIcon : public FillBox
{
	using SUPER = FillBox;

public:
	struct Props
	{
		enum
		{
			SYMBOL = SUPER::Props::NEXT_,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	SymbolIcon(const Vec2 &size, int symbol,
		const Color color = COLOR_WHITE, uiLayoutFlags_t layoutFlags = 0);
	SymbolIcon(double w, double h, int symbol,
		const Color color = COLOR_WHITE, uiLayoutFlags_t layoutFlags = 0);
	virtual ~SymbolIcon() { }

public:
	virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

public:
	int GetSymbol() const { return symbol; }
	void SetSymbol(int symbol);

private:
	int symbol;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
