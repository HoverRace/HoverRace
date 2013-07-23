
// Button.h
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

#include "ClickRegion.h"

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
		class FillBox;
		class Label;
	}
}

namespace HoverRace {
namespace Display {

/**
 * A standard button, with a text label.
 * @author Michael Imamura
 */
class MR_DllDeclare Button : public ClickRegion
{
	typedef ClickRegion SUPER;

	public:
		struct Props
		{
			enum {
				TEXT = SUPER::Props::NEXT_,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		Button(Display &display, const std::string &text,
			uiLayoutFlags_t layoutFlags=0);
		Button(Display &display, const Vec2 &size, const std::string &text,
			uiLayoutFlags_t layoutFlags=0);
		virtual ~Button();

	private:
		void Init(const std::string &text);

	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

	public:
		FillBox *GetBackgroundChild() const { return background.get(); }
		Label *GetLabelChild() const { return label.get(); }

	protected:
		virtual void Layout();

	public:
		virtual Vec3 Measure() const;

	private:
		Display &display;
		std::unique_ptr<FillBox> background;
		std::unique_ptr<Label> label;
		double paddingTop, paddingRight, paddingBottom, paddingLeft;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
