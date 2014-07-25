
// ScreenFade.h
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

#pragma once

#include "Color.h"

#include "Background.h"

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
 * Paints a solid color over the whole screen with adjustable opacity.
 * @author Michael Imamura
 */
class MR_DllDeclare ScreenFade : public Background
{
	typedef Background SUPER;

	public:
		struct Props
		{
			enum {
				COLOR = SUPER::Props::NEXT_,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		ScreenFade(const Color color, double opacity);
		virtual ~ScreenFade();

	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

	public:
		const Color GetColor() const { return color; }
		void SetColor(const Color color);

	private:
		Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
