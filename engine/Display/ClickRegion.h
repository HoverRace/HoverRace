
// ClickRegion.h
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

#include "UiViewModel.h"

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
 * Base class for clickable areas.
 * @author Michael Imamura
 */
class MR_DllDeclare ClickRegion : public UiViewModel
{
	using SUPER = UiViewModel;

public:
	struct Props
	{
		enum {
			/// Fired when a fixed size is set or auto-sizing is enabled.
			SIZE = SUPER::Props::NEXT_,
			ENABLED,
			PRESSED,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	ClickRegion(Display &display, uiLayoutFlags_t layoutFlags = 0);
	ClickRegion(Display &display, const Vec2 &size,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~ClickRegion();

public:
	void AttachView(Display &disp) override { AttachViewDynamic(disp, this); }

public:
	bool OnMouseMoved(const Vec2 &pos) override;
	bool OnMousePressed(const Control::Mouse::Click &click) override;
	bool OnMouseReleased(const Control::Mouse::Click &click) override;
	bool OnAction() override;

protected:
	/**
	 * Handle when the mouse is dragged over the widget.
	 * @param relPos The relative position.
	 */
	virtual void OnMouseDrag(const Vec2 &relPos) { HR_UNUSED(relPos); }

public:
	typedef boost::signals2::signal<void(ClickRegion&)> clickedSignal_t;
	clickedSignal_t &GetClickedSignal() { return clickedSignal; }
protected:
	virtual void FireClickedSignal();

public:
	bool TryFocus() override;

	const Vec2 &GetSize();
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

	/**
	 * Check if the widget is enabled.
	 * Disabled widgets are visible but not clickable or focusable.
	 * @return @c true if enabled, @c false if disabled.
	 */
	bool IsEnabled() const { return enabled; }
	void SetEnabled(bool enabled);

protected:
	bool IsPressed() const { return pressed; }
	void SetPressed(bool pressed);

public:
	virtual Vec3 Measure() { return size.Promote(); }

protected:
	void RequestSizing();
	bool TestHit(const Vec2 &pos);
	Vec2 ScreenPosToRel(const Vec2 &pos);

protected:
	Display &display;
private:
	Vec2 size;
	bool autoSize;
	bool needsSizing;
	bool enabled;
	bool pressed;
	clickedSignal_t clickedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
