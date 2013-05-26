
// View.h
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

/**
 * Base class for views.
 * Views handle the actual API-specific rendering of view models.
 * @author Michael Imamura
 */
class MR_DllDeclare View
{
	public:
		virtual ~View() { }

	public:
		/**
		 * Called by the view model to indicate that a property has changed.
		 * The view can use this to decide if view-specific resources need to be
		 * reloaded.
		 * @param prop The model-specific ID of the property that changed.
		 */
		virtual void OnModelUpdate(int prop) { }

	public:
		virtual Vec3 Measure() = 0;
		virtual void PrepareRender() = 0;
		virtual void Render() = 0;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
