
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
	controller.AddMenuMaps();

	auto &menuOkAction = controller.actions.ui.menuOk;
	okConn = menuOkAction->Connect(std::bind(&UiScene::OnAction, this));
}

void UiScene::DetachController(Control::InputEventController&)
{
	okConn.disconnect();
}

void UiScene::OnAction()
{
	if (focusRoot) {
		focusRoot->OnAction();
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
	}

	focusRoot = std::move(root);

	if (focusRoot) {
		focusReqConn = focusRoot->GetFocusRequestedSignal().connect(
			[&](Display::UiViewModel &widget) { widget.TryFocus(); });
	}
}

}  // namespace Client
}  // namespace HoverRace
