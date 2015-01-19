
// UiScene.h
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

#include "Scene.h"

namespace HoverRace {
	namespace Display {
		class UiViewModel;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Base class for UI-centric scenes.
 * @author Michael Imamura
 */
class UiScene : public Scene
{
	using SUPER = Scene;

public:
	UiScene(const std::string &name = "") :
		SUPER(name), needsLayout(true) { }
	virtual ~UiScene() { }

public:
	bool IsMouseCursorEnabled() const override { return true; }

public:
	void AttachController(Control::InputEventController &controller) override;
	void DetachController(Control::InputEventController &controller) override;

private:
	void OnAction();

protected:
	void SetFocusRoot(std::shared_ptr<Display::UiViewModel> root);

	/**
	 * Indicate that the current layout is out-of-date and needs to be adjusted.
	 * Subclasses should call this when a property changes that affects the
	 * layout (such as the size).
	 */
	void RequestLayout() { needsLayout = true; }

	/**
	 * Adjust the size and position of any child elements.
	 *
	 * Subclasses with child elements that are relative to each other should
	 * override this function.
	 *
	 * This is called automatically during the PrepareRender() phase if
	 * RequestLayout() has been called.  It is also called the first time
	 * PrepareRender() is invoked.  After this function is called, it
	 * will not be called again until another call to RequestLayout().
	 */
	virtual void Layout() { }

public:
	void PrepareRender() override
	{
		if (needsLayout) {
			Layout();
			needsLayout = false;
		}
	}

	void Render() override { }

private:
	bool needsLayout;
	std::shared_ptr<Display::UiViewModel> focusRoot;
	boost::signals2::scoped_connection focusReqConn;
	boost::signals2::scoped_connection okConn;
};

}  // namespace Client
}  // namespace HoverRace
