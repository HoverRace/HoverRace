
// MainMenuScene.h
//
// Copyright (c) 2013-2016 Michael Imamura.
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

#include "GameDirector.h"

#include "FormScene.h"

namespace HoverRace {
	namespace Display {
		class Button;
		class Container;
		class Display;
	}
}

namespace HoverRace {
namespace Client {

/**
 * The title scene.
 * @author Michael Imamura
 */
class MainMenuScene : public FormScene
{
	using SUPER = FormScene;

public:
	MainMenuScene(Display::Display &display, GameDirector &director,
		RulebookLibrary &rulebookLibrary);
	virtual ~MainMenuScene();

private:
	std::shared_ptr<Display::Button> AddButton(const std::string &text,
		bool enabled = true, bool focused = false);

private:
	void OnPracticeClicked();
	void OnMultiplayerClicked();
	void OnSettingsClicked();
	void OnMutedClicked();

public:
	// Scene
	void OnStateChanged(State oldState) override;
	void OnStateTransition(double interval) override;
	void Layout() override;
	void PrepareRender() override;
	void Render() override;

private:
	Display::Display &display;
	GameDirector &director;
	RulebookLibrary &rulebookLibrary;

	std::unique_ptr<Display::FillBox> letterUpBox;
	std::unique_ptr<Display::FillBox> letterDownBox;
	std::shared_ptr<Display::Container> titleContainer;
	std::shared_ptr<Display::Container> menuContainer;
	std::vector<std::shared_ptr<Display::Button>> menuButtons;
	boost::signals2::scoped_connection displayConfigChangedConn;
};

}  // namespace Client
}  // namespace HoverRace
