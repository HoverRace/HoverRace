
// Container.h
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
 * An invisible container for other UI widgets.
 * The child 
 * @author Michael Imamura
 */
class MR_DllDeclare Container : public UiViewModel
{
	typedef UiViewModel SUPER;

	public:
		struct Props
		{
			enum {
				SIZE = SUPER::Props::NEXT_,
				CLIP,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		Container(Display &display, uiLayoutFlags_t layoutFlags=0);
		Container(Display &display, const Vec2 &size, bool clip=true,
			uiLayoutFlags_t layoutFlags=0);
		virtual ~Container() { }

	public:
		virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }

	public:
		template<typename T>
		typename std::enable_if<std::is_base_of<UiViewModel, T>::value, std::shared_ptr<T>>::type
		AddChild(T *child)
		{
			std::shared_ptr<T> sharedChild(child);
			children.emplace_back(sharedChild);
			child->AttachView(display);
			return sharedChild;
		}

	public:
		/**
		 * Retrieve the size of the container.
		 * @note If clipping is turned off, the size 
		 * @return The size, where @c x is the width and @c y is the height.
		 */
		const Vec2 &GetSize() const { return size; }
		void SetSize(const Vec2 &size);
		/// Convenience function for SetSize(const Vec2&).
		void SetSize(double w, double h) { SetSize(Vec2(w, h)); }

		/**
		 * Check if child elements are clipped to the container bounds.
		 * @return @c true if clipping is enabled, @false if elements are
		 *         allowed to render outside of the bounds.
		 */
		bool IsClip() const { return clip; }
		void SetClip(bool clip);

		const std::vector<UiViewModelPtr> &GetChildren() const { return children; }

	public:
		virtual Vec3 Measure() const { return size.Promote(); }

	private:
		Display &display;
		Vec2 size;
		bool clip;
		std::vector<UiViewModelPtr> children;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
