
// UiViewModel.h
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Util/MR_Types.h"
#include "../Vec.h"
#include "UiLayoutFlags.h"
#include "ViewModel.h"

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
	namespace Control {
		namespace Mouse {
			struct Click;
		}
		class Nav;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Base class for UI (2D) components.
 * @author Michael Imamura
 */
class MR_DllDeclare UiViewModel : public ViewModel
{
	using SUPER = ViewModel;

public:
	struct Props
	{
		enum
		{
			POS,
			ALIGNMENT,
			FOCUSED,
			NEXT_,  ///< First index for subclasses.
		};
	};

	/**
	 * Imagine the component pinned to the container with a thumbtack.
	 * The thumbtack's position is at GetPos(), and the alignment determines
	 * which corner the component hangs from.
	 */
	enum class Alignment
	{
		NW,  ///< Northwest corner (default).
		N,  ///< North (horizontally-centered).
		NE,  ///< Northeast corner.
		E,  ///< East (vertically-centered).
		SE,  ///< Southeast corner.
		S,  ///< South (horizontally-centered).
		SW,  ///< Southwest corner.
		W,  ///< West (vertically-centered).
		CENTER,  ///< Horizontally @e and vertically centered.
	};

public:
	UiViewModel(uiLayoutFlags_t layoutFlags = 0) :
		SUPER(), pos(0, 0), translation(0, 0), alignment(Alignment::NW),
		layoutFlags(layoutFlags), id(0), focused(false) { }
	virtual ~UiViewModel() { }

public:
	virtual bool OnMouseMoved(const Vec2&) { return false; }
	virtual bool OnMousePressed(const Control::Mouse::Click&) { return false; }
	virtual bool OnMouseReleased(const Control::Mouse::Click&) { return false; }
	virtual bool OnAction() { return false; }
	virtual bool OnNavigate(const Control::Nav&) { return false; }

public:
	/**
	 * Get the position of the component.
	 * @return The position, relative to the parent (if any).
	 * @see GetAlignment()
	 */
	const Vec2 &GetPos() const { return pos; }
	void SetPos(const Vec2 &pos);
	/// Convenience function for SetPos(const Vec2&).
	void SetPos(double x, double y) { SetPos(Vec2(x, y)); }

	/**
	 * Retrieves the position translation.
	 * @return The translation.
	 */
	const Vec2 &GetTranslation() const { return translation; }
	void SetTranslation(const Vec2 &translation);
	/// Convenience function for SetTranslation(const Vec2&).
	void SetTranslation(double x, double y) { SetTranslation(Vec2(x, y)); }

	/**
	 * Retrieve the alignment of the component.
	 * @return The alignment.
	 * @see UiViewModel::Alignment
	 */
	Alignment GetAlignment() const { return alignment; }
	void SetAlignment(Alignment alignment);

	Vec2 GetAlignedPos(const Vec2 &pos, double w, double h) const;

	/**
	 * Retrieve the position adjusted by the current alignment.
	 * @param w The width of the component.
	 * @param h The height of the component.
	 * @return The adjusted position.
	 */
	Vec2 GetAlignedPos(double w, double h) const
	{
		return GetAlignedPos(pos + translation, w, h);
	}

	/**
	 * Retrieve the layout flags.
	 * @see UiViewModel::LayoutFlags
	 */
	uiLayoutFlags_t GetLayoutFlags() const { return layoutFlags; }

	// Convenience functions for querying layout flags.
	uiLayoutFlags_t IsLayoutUnscaled() const { return layoutFlags & UiLayoutFlags::UNSCALED; }
	uiLayoutFlags_t IsLayoutFloating() const { return layoutFlags & UiLayoutFlags::FLOATING; }

	/**
	 * Retrieve the non-unique identifier for this widget.
	 * @return The identifier.
	 */
	MR_UInt32 GetId() const { return id; }

	/**
	 * Set the non-unique identifier for this widget.
	 * This is used mainly for debugging.  Default ID is zero.
	 * @param id The identifier.
	 */
	void SetId(MR_UInt32 id) { this->id = id; }

	/**
	 * Check if this widget currently has input focus.
	 * @return @c true if focused, @c false if not.
	 */
	bool IsFocused() const { return focused; }

	/**
	 * Attempts to give this widget focus.
	 *
	 * Subclasses should override this with logic to determine if the widget
	 * is focusable.
	 *
	 * This is called by the parent container if focus has been requested.
	 *
	 * @return @c true if it succeeds (widget is now focused),
	 *         @c false if it fails (widget did not take focus).
	 */
	virtual bool TryFocus() { return false; }

	/**
	 * Force this widget to give up focus, if it is currently focused.
	 *
	 * This is called by the parent container if focus is shifting away to
	 * another widget.
	 */
	virtual void DropFocus() { SetFocused(false); }

protected:
	void SetFocused(bool focused);

public:
	using focusRequestedSignal_t =
		boost::signals2::signal<void(UiViewModel&)>;
	focusRequestedSignal_t &GetFocusRequestedSignal() { return focusRequestedSignal; }

	void RequestFocus();

private:
	Vec2 pos;
	Vec2 translation;
	Alignment alignment;
	uiLayoutFlags_t layoutFlags;
	MR_UInt32 id;
	bool focused;
	focusRequestedSignal_t focusRequestedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
