
// ViewModel.h
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

#include "Display.h"
#include "View.h"

#include "ViewAttacher.h"

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
 * Base class for renderable components.
 * View models only track the state of the renderable.  The rendering is
 * handled by the View, which is attached by the API-specific Display when
 * the the view model is added to the scene graph.
 * @author Michael Imamura
 */
class MR_DllDeclare ViewModel
{
	public:
		ViewModel() : needsLayout(true) { }
		virtual ~ViewModel() { }

	public:
		virtual void AttachView(Display &disp) = 0;
		void SetView(std::unique_ptr<View> &&view) { this->view = std::move(view); }
		View *GetView() const { return view.get(); }

	protected:
		template<class T>
		void AttachViewDynamic(Display &disp, T *self)
		{
			dynamic_cast<ViewAttacher<T>&>(disp).AttachView(*self);
		}

	protected:
		/**
		 * Indicate that the current layout is out-of-date and needs to be adjusted.
		 * Subclasses should call this when a property changes that affects the
		 * layout (such as the size).
		 */
		void RequestLayout() { needsLayout = true; }

		/**
		 * Adjust the size and position of any child elements.
		 *
		 * Subclasses with child elements should override this function.
		 *
		 * This is called automatically during the PrepareRender() phase if
		 * RequestLayout() has been called.  It is also called the first time
		 * PrepareRender() is invoked.  After this function is called, it
		 * will not be called again until another call to RequestLayout().
		 */
		virtual void Layout() { }

	public:
		/**
		 * Calculate the size of the component.
		 * For UI components, the return value is affected by layout flags.
		 * @return The size (may be zero if the size cannot be determined yet).
		 * @warning Depending on the view, this may be an expensive operation
		 *          since the view may have to call View::PrepareRender to apply
		 *          model changes. As such, it is recommended to call Measure in
		 *          the PrepareRender phase itself.
		 */
		virtual Vec3 Measure() const { return view ? view->Measure() : Vec3(0, 0, 0); }

		void PrepareRender()
		{
			if (needsLayout) {
				Layout();
				needsLayout = false;
			}
			if (view) view->PrepareRender();
		}

		void Render() { if (view) view->Render(); }

	protected:
		/**
		 * Indicate that a model property has changed.
		 * If a view is attached, it will be notified.
		 * @param prop The model-specific ID of the property that changed.
		 */
		virtual void FireModelUpdate(int prop) { if (view) view->OnModelUpdate(prop); }

	private:
		bool needsLayout;
		std::unique_ptr<View> view;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
