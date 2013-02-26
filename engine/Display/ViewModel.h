
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
		virtual void AttachView(Display *disp) = 0;
		void SetView(std::unique_ptr<View> &&view) { this->view = std::move(view); }
		View *GetView() const { return view.get(); }

	protected:
		template<class T>
		void AttachViewDynamic(Display *disp, T *self)
		{
			dynamic_cast<ViewAttacher<T>*>(disp)->AttachView(*self);
		}

	public:
		void PrepareRender() { if (view) view->PrepareRender(); }
		void Render() { if (view) view->Render(); }

	protected:
		/**
		 * Indicate that a model property has changed.
		 * If a view is attached, it will be notified.
		 * @param prop The model-specific ID of the property that changed.
		 */
		void FireModelUpdate(int prop) { if (view) view->OnModelUpdate(prop); }

	private:
		std::unique_ptr<View> view;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
