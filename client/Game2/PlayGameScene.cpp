
// PlayGameScene.cpp
//
// Copyright (c) 2014 Michael Imamura.
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
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Player/Player.h"

#include "ClientSession.h"
#include "PauseMenuScene.h"

#include "PlayGameScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

PlayGameScene::PlayGameScene(Display::Display &display, GameDirector &director,
                             Script::Core *scripting,
                             std::shared_ptr<Rules> rules,
                             std::shared_ptr<Loader> loader) :
	SUPER("Game", display, director, scripting, rules, loader)
{
}

PlayGameScene::~PlayGameScene()
{
}

void PlayGameScene::AttachController(Control::InputEventController &controller)
{
	if (auto player = session->GetPlayer(0)) {
		if (auto mc = player->GetMainCharacter()) {
			controller.AddPlayerMaps(1, &mc);
			controller.AddCameraMaps();
		}
	}

	//TODO: Use separate action for pausing than ui.menuCancel.
	controller.AddMenuMaps();

	cameraZoomInConn = controller.actions.camera.zoomIn->Connect(
		std::bind(&PlayGameScene::OnCameraZoom, this, 1));
	cameraZoomOutConn = controller.actions.camera.zoomOut->Connect(
		std::bind(&PlayGameScene::OnCameraZoom, this, -1));
	cameraPanUpConn = controller.actions.camera.panUp->Connect(
		std::bind(&PlayGameScene::OnCameraPan, this, 1));
	cameraPanDownConn = controller.actions.camera.panDown->Connect(
		std::bind(&PlayGameScene::OnCameraPan, this, -1));
	cameraResetConn = controller.actions.camera.reset->Connect(
		std::bind(&PlayGameScene::OnCameraReset, this));

	pauseConn = controller.actions.ui.menuCancel->Connect(
		std::bind(&PlayGameScene::OnPause, this));
}

void PlayGameScene::DetachController(Control::InputEventController&)
{
	// Shut off the engine when the controller is detached (e.g. when showing a
	// dialog) otherwise we'll just keep accelerating into the wall.
	if (auto player = session->GetPlayer(0)) {
		if (auto mc = player->GetMainCharacter()) {
			mc->SetEngineState(false);
			mc->SetBrakeState(false);
			mc->SetTurnLeftState(false);
			mc->SetTurnRightState(false);
			mc->SetLookBackState(false);
		}
	}

	pauseConn.disconnect();

	cameraResetConn.disconnect();
	cameraPanDownConn.disconnect();
	cameraPanUpConn.disconnect();
	cameraZoomOutConn.disconnect();
	cameraZoomInConn.disconnect();
}

void PlayGameScene::OnCameraZoom(int increment)
{
	for (auto &viewport : viewports) {
		viewport.observer->Zoom(increment);
	}
}

void PlayGameScene::OnCameraPan(int increment)
{
	for (auto &viewport : viewports) {
		viewport.observer->Scroll(increment);
	}
}

void PlayGameScene::OnCameraReset()
{
	for (auto &viewport : viewports) {
		viewport.observer->Home();
	}
}

/// Fired when the pause button (e.g. ESC) is pressed.
void PlayGameScene::OnPause()
{
	director.RequestPushScene(std::make_shared<PauseMenuScene>(display, director));
}

}  // namespace HoverScript
}  // namespace Client
