
// View.h
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

/**
 * Base class for views.
 * Views handle the actual API-specific rendering of view models.
 * @author Michael Imamura
 */
class MR_DllDeclare View
{
public:
	virtual ~View() { }

	// Disable assignment by default; a view should only be bound to the model
	// once at construction time.
	View &operator=(const View&) = delete;

public:
	/**
	 * Called by the view model to indicate that a property has changed.
	 * The view can use this to decide if view-specific resources need to be
	 * reloaded.
	 * @param prop The model-specific ID of the property that changed.
	 */
	virtual void OnModelUpdate(int prop) { HR_UNUSED(prop); }

public:
	/**
	 * Retrieve the screen-space position, if available.
	 * The availability of this information varies from view to view.
	 * @return The screen position, or (0, 0) if unavailable..
	 */
	virtual Vec2 GetScreenPos() const { return { 0, 0 }; }

	/**
	 * Retrieve the screen-space size, if available.
	 * The availability of this information varies from view to view.
	 * @return The screen size, or (0, 0) if unavailable.
	 */
	virtual Vec2 GetScreenSize() const { return { 0, 0 }; }

	virtual Vec3 Measure() = 0;
	virtual void PrepareRender() = 0;
	virtual void Render() = 0;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
