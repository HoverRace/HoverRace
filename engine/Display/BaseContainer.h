
// BaseContainer.h
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
 * An invisible container for other UI widgets.
 * @author Michael Imamura
 */
class MR_DllDeclare BaseContainer : public UiViewModel
{
	using SUPER = UiViewModel;

public:
	struct Props
	{
		enum
		{
			SIZE = SUPER::Props::NEXT_,
			CLIP,
			OPACITY,
			VISIBLE,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	BaseContainer(Display &display, uiLayoutFlags_t layoutFlags = 0);
	BaseContainer(Display &display, const Vec2 &size, bool clip = true,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~BaseContainer() { }

public:
	void AttachView(Display &disp) override { AttachViewDynamic(disp, this); }

protected:
	class Child
	{
	public:
		Child(BaseContainer &bc, std::shared_ptr<UiViewModel> child) :
			child(std::move(child))
		{
			// Route focus request events back to the container.
			focusRequestedConn.reset(
				new boost::signals2::scoped_connection(
					this->child->GetFocusRequestedSignal().connect(
						std::bind(&BaseContainer::OnChildRequestedFocus, &bc,
							std::placeholders::_1))));
			focusRelinquishedConn.reset(
				new boost::signals2::scoped_connection(
					this->child->GetFocusRelinquishedSignal().connect(
						std::bind(&BaseContainer::OnChildRelinquishedFocus, &bc,
							std::placeholders::_1, std::placeholders::_2))));
		}
		Child(const Child&) = delete;
		Child(Child&&) = default;

		Child &operator=(const Child&) = delete;
		Child &operator=(Child&&) = default;

	public:
		std::shared_ptr<UiViewModel> child;
	private:
		// scoped_connection is not movable, so we wrap in a unique_ptr.
		std::unique_ptr<boost::signals2::scoped_connection> focusRequestedConn;
		std::unique_ptr<boost::signals2::scoped_connection> focusRelinquishedConn;
	};

	using children_t = std::vector<Child>;

	// These are marked as protected so that subclasses can restrict how and
	// what types of widgets can be added / removed from the container.

	/**
	 * Create and append a new child widget to the end of the list.
	 * @tparam T The type of the child widget.
	 * @tparam Args The types of the arguments.
	 * @param args The arguments to pass to the child widget's constructor.
	 * @return A shared pointer to the newly-constructed widget.
	 */
	template<class T, class... Args>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>>::type
	NewChild(Args&&... args)
	{
		auto sharedChild = std::make_shared<T>(std::forward<Args>(args)...);
		children.emplace_back(*this, sharedChild);
		sharedChild->AttachView(display);
		return sharedChild;
	}

	/**
	 * Remove a child element.
	 * @param child The child element; must be a shared_ptr to a subclass
	 *              of UiViewModel.
	 * @return The same child element that was passed in.
	 */
	template<typename T>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value, std::shared_ptr<T>>::type
	RemoveChild(const std::shared_ptr<T> &child)
	{
		for (auto iter = children.begin(); iter != children.end(); ++iter) {
			if (iter->child == child) {
				children.erase(iter);
				break;
			}
		}
		return child;
	}

	/**
	 * Move a widget to a different position in the list.
	 * @param child The child.
	 * @param idx The destination index.  If beyond the end, then the child will
	 *            be moved to the end of the list.
	 * @return The same child element that was passed in.
	 */
	template<class T>
	typename std::enable_if<std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<T>
		>::type
	ReorderChild(const std::shared_ptr<T> &child, size_t idx)
	{
		auto sz = children.size();
		if (sz == 0) return child;

		if (idx >= sz) {
			idx = sz - 1;
		}

		for (auto iter = children.begin(); iter != children.end(); ++iter) {
			if (iter->child == child) {
				auto dest = children.begin();
				std::advance(dest, idx);
				if (dest < iter) {
					std::rotate(dest, iter, iter + 1);
				}
				else if (dest > iter) {
					std::rotate(iter, iter + 1, dest + 1);
				}
				break;
			}
		}

		return child;
	}

	/**
	 * Remove all child elements.
	 */
	virtual void Clear()
	{
		children.clear();
	}

private:
	template<typename P, bool(UiViewModel::*F)(P)>
	bool PropagateMouseEvent(P param)
	{
		if (!visible || children.empty()) return false;

		// We iterate over the child widgets in reverse since the widgets
		// are rendered back to front, so we let the ones in front get
		// first chance to handle the events.

		Vec2 oldOrigin(0, 0);
		for (auto iter = children.rbegin();
			iter != children.rend(); ++iter)
		{
			auto &child = iter->child;
			oldOrigin = display.AddUiOrigin(child->GetPos());
			bool retv = (child.get()->*F)(param);
			display.SetUiOrigin(oldOrigin);
			if (retv) return true;
		}

		return false;
	}

public:
	bool OnMouseMoved(const Vec2 &pos) override
	{
		return PropagateMouseEvent<const Vec2&, &UiViewModel::OnMouseMoved>(pos);
	}
	bool OnMousePressed(const Control::Mouse::Click &click) override
	{
		return PropagateMouseEvent<const Control::Mouse::Click&, &UiViewModel::OnMousePressed>(click);
	}
	bool OnMouseReleased(const Control::Mouse::Click &click) override
	{
		return PropagateMouseEvent<const Control::Mouse::Click&, &UiViewModel::OnMouseReleased>(click);
	}

protected:
	/**
	 * Called when a child widget requests input focus.
	 * @param child The child widget.
	 */
	virtual void OnChildRequestedFocus(UiViewModel &child) { HR_UNUSED(child); }

	/**
	 * Called when a child widget passes input focus to another widget.
	 * @param child The child widget.
	 * @param nav The navigation direction.
	 */
	virtual void OnChildRelinquishedFocus(UiViewModel &child,
		const Control::Nav &nav) { HR_UNUSED(child); HR_UNUSED(nav); }

public:
	void ShrinkWrap();

	/**
	 * Retrieve the size of the container.
	 * @note If clipping is turned off, the size is undefined.
	 * @return The size, where @c x is the width and @c y is the height.
	 */
	const Vec2 &GetSize() const { return size; }
	void SetSize(const Vec2 &size);
	/// Convenience function for SetSize(const Vec2&).
	void SetSize(double w, double h) { SetSize(Vec2(w, h)); }

	/**
	 * Check if child elements are clipped to the container bounds.
	 * @return @c true if clipping is enabled, @c false if elements are
	 *         allowed to render outside of the bounds.
	 */
	bool IsClip() const { return clip; }
	void SetClip(bool clip);

	/**
	 * Retrieve the opacity.
	 * @return The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
	 */
	double GetOpacity() const { return opacity; }
	void SetOpacity(double opacity);

	/**
	 * Check if the children of this container are shown.
	 * @return @c true if visible, @c false if not.
	 */
	bool IsVisible() const { return visible; }
	void SetVisible(bool visible);

	/**
	 * Check if this container is devoid of child widgets.
	 * @return @c true if empty, @c false if not.
	 */
	bool IsEmpty() const { return children.empty(); }

	/**
	 * Iterate over each child widget.
	 * @param fn The function to execute for each widget.
	 */
	template<class Fn>
	void ForEachChild(Fn fn) const
	{
		for (auto &child : children) {
			fn(child.child);
		}
	}

protected:
	children_t &GetChildren() { return children; }

public:
	Vec3 Measure() override { return size.Promote(); }

protected:
	Display &display;
private:
	Vec2 size;
	bool clip;
	double opacity;
	bool visible;
	children_t children;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
