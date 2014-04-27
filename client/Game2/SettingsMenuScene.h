
// SettingsMenuScene.h
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

#pragma once

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
		class FlexGrid;
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Settings main menu.
 * @author Michael Imamura
 */
class SettingsMenuScene : public DialogScene
{
	typedef DialogScene SUPER;
	public:
		SettingsMenuScene(Display::Display &display, GameDirector &director);
		virtual ~SettingsMenuScene();

	public:
		virtual void OnPhaseTransition(double progress) override;
		virtual void PrepareRender() override;
		virtual void Render() override;

	private:
		std::unique_ptr<Display::ScreenFade> fader;
		std::shared_ptr<Display::FlexGrid> menuGrid;
};

}  // namespace Client
}  // namespace HoverRace
