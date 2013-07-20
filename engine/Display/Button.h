
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

#include "UiViewModel.h"

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
 * Base class for clickable areas.
 * @author Michael Imamura
 */
class MR_DllDeclare Button : public UiViewModel
{
	typedef UiViewModel SUPER;

	public:
		struct Props
		{
			enum {
				/// Fired when a fixed size is set or auto-sizing is enabled.
				SIZE = SUPER::Props::NEXT_,
				PRESSED,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		Button(Display &display, uiLayoutFlags_t layoutFlags=0);
		Button(Display &display, const Vec2 &size, uiLayoutFlags_t layoutFlags=0);
		virtual ~Button();

	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

	public:
		virtual void OnMouseMoved(const Vec2 &pos);
		virtual void OnMousePressed(const Control::Mouse::Click &click);
		virtual void OnMouseReleased(const Control::Mouse::Click &click);

	public:
		typedef boost::signals2::signal<void(Button&)> clickedSignal_t;
		clickedSignal_t &GetClickedSignal() { return clickedSignal; }
	protected:
		void FireClickedSignal();

	public:
		const Vec2 &GetSize() const;
		void SetSize(const Vec2 &size);
		/// Convenience function for SetSize(const Vec2&).
		void SetSize(double w, double h) { SetSize(Vec2(w, h)); }

		/**
		 * Check if automatic sizing is enabled.
		 * @return @c true if the size is determined by the contents of the
		 *         button, @c false if using a fixed size.
		 */
		bool IsAutoSize() const { return autoSize; }
		void SetAutoSize();

	protected:
		bool IsPressed() const { return pressed; }
		void SetPressed(bool pressed);

	public:
		virtual Vec3 Measure() const { return size.Promote(); }

	protected:
		bool TestHit(const Vec2 &pos) const;

	private:
		Display &display;
		mutable Vec2 size;
		bool autoSize;
		mutable bool needsSizing;
		bool pressed;
		clickedSignal_t clickedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
