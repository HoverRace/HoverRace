
// PlayGameScene.h
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include "GameScene.h"

namespace HoverRace {
namespace Client {

/**
 * The actual, genuine gameplay scene.  Accept no imitations.
 *
 * This scene manages the lifetime of a single session, from start to finish.
 * Pushing this scene onto the stage starts a new game session, and likewise
 * popping this scene from the stage aborts the game session.
 *
 * @author Michael Imamura
 */
class PlayGameScene : public GameScene
{
	typedef GameScene SUPER;

public:
	PlayGameScene(Display::Display &display, GameDirector &director,
		Script::Core *scripting, std::shared_ptr<Rules> rules,
		std::shared_ptr<Util::Loader> loader);
	virtual ~PlayGameScene();

public:
	void AttachController(Control::InputEventController &controller,
		ConnList &conns) override;
	void DetachController(Control::InputEventController &controller,
		ConnList &conns) override;

private:
	void OnCameraZoom(int increment);
	void OnCameraPan(int increment);
	void OnCameraReset();
	void OnPause();
};

}  // namespace Client
}  // namespace HoverRace
