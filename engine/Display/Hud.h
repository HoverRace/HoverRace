
// Hud.h
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

#include "Container.h"

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
		class Display;
	}
}

namespace HoverRace {
namespace Display {

/**
 * The container for the heads-up display.
 * @author Michael Imamura
 */
class MR_DllDeclare Hud : public Container
{
	typedef Container SUPER;

	public:
		struct Props
		{
			enum {
				VISIBLE = SUPER::Props::NEXT_,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		Hud(Display &display, const Vec2 &size, bool clip=true,
			uiLayoutFlags_t layoutFlags=0);
		virtual ~Hud() { }

/*
	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }
*/

	public:
		/**
		 * Check if the HUD is currently visible;
		 * @return @c true if the HUD is visible, @c false otherwise.
		 */
		bool IsVisible() const { return visible; }
		void SetVisible(bool visible);

	private:
		Display &display;
		bool visible;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
