
// DemoGameScene.h
//
// Copyright (c) 2014-2016 Michael Imamura.
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
 * Demo mode, used for the main menu background.
 * @author Michael Imamura
 */
class DemoGameScene : public GameScene
{
	using SUPER = GameScene;

public:
	DemoGameScene(Display::Display &display, GameDirector &director,
		Script::Core &scripting,
		std::shared_ptr<Util::Loader> loader);
	virtual ~DemoGameScene();

public:
	void AttachController(Control::InputEventController&,
		ConnList&) override { }
	void DetachController(Control::InputEventController&,
		ConnList&) override { }

protected:
	void OnFinishedLoading() override;
};

}  // namespace Client
}  // namespace HoverRace
