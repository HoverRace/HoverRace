
// UiScene.cpp
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/UiViewModel.h"

#include "UiScene.h"

namespace HoverRace {
namespace Client {

void UiScene::AttachController(Control::InputEventController &controller)
{
	// Standard digital directions.
	static Control::Nav NAV_U(Control::Nav::UP);
	static Control::Nav NAV_D(Control::Nav::DOWN);
	static Control::Nav NAV_L(Control::Nav::LEFT);
	static Control::Nav NAV_R(Control::Nav::RIGHT);

	controller.AddMenuMaps();

	auto &ui = controller.actions.ui;

	okConn = ui.menuOk->Connect(std::bind(&UiScene::OnAction, this));

	upConn = ui.menuUp->Connect(std::bind(&UiScene::OnNav, this, NAV_U));
	downConn = ui.menuDown->Connect(std::bind(&UiScene::OnNav, this, NAV_D));
	leftConn = ui.menuLeft->Connect(std::bind(&UiScene::OnNav, this, NAV_L));
	rightConn = ui.menuRight->Connect(std::bind(&UiScene::OnNav, this, NAV_R));
}

void UiScene::DetachController(Control::InputEventController&)
{
	rightConn.disconnect();
	leftConn.disconnect();
	downConn.disconnect();
	upConn.disconnect();

	okConn.disconnect();
}

void UiScene::OnAction()
{
	if (focusRoot) {
		focusRoot->OnAction();
	}
}

void UiScene::OnNav(Control::Nav &nav)
{
	if (focusRoot) {
		focusRoot->OnNavigate(nav);
	}
}

/**
 * Set the widget used as the root for all focus management.
 *
 * Widgets which are not under this widget will not be able to receive input
 * focus.
 *
 * @param root The root widget.
 */
void UiScene::SetFocusRoot(std::shared_ptr<Display::UiViewModel> root)
{
	if (focusRoot) {
		focusReqConn.disconnect();
		focusRelConn.disconnect();
	}

	focusRoot = std::move(root);

	if (focusRoot) {
		// By assigning a focus root, we act as if we are a one-element
		// container that is always focused.

		focusReqConn = focusRoot->GetFocusRequestedSignal().connect(
			[&](Display::UiViewModel &widget) { widget.TryFocus(); });

		focusRelConn = focusRoot->GetFocusRelinquishedSignal().
			connect([&](Display::UiViewModel &widget, const Control::Nav&) {
				widget.DropFocus();
				widget.TryFocus();
			});
	}
}

}  // namespace Client
}  // namespace HoverRace
