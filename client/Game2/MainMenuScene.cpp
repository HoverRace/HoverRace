
// MainMenuScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Util/Config.h"

#include "PracticeSetupScene.h"
#include "Rulebook.h"

#include "MainMenuScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

MainMenuScene::MainMenuScene(Display::Display &display, GameDirector &director,
                             RulebookLibrary &rulebookLibrary) :
	SUPER(display, "Main Menu"),
	display(display), director(director), rulebookLibrary(rulebookLibrary)
{
	typedef Display::UiViewModel::Alignment Alignment;

	SetPhaseTransitionDuration(500);

	auto root = GetRoot();

	//TODO: Letterbox mask.

	double sliderHeight = 120;

	titleContainer = root->AddChild(new Display::Container(display,
		Vec2(1280, sliderHeight)));

	titleContainer->AddChild(new Display::FillBox(titleContainer->GetSize(), 0xff000000));

	Config *cfg = Config::GetInstance();
	auto titleLbl = titleContainer->AddChild(new Display::Label(
		"[ logo goes here :) ]",
		Display::UiFont(cfg->GetDefaultFontName(), 40, Display::UiFont::BOLD | Display::UiFont::ITALIC),
		0xffffffff));
	titleLbl->SetPos(40, sliderHeight);
	titleLbl->SetAlignment(Alignment::SW);

	menuContainer = root->AddChild(new Display::Container(display,
		Vec2(1280, sliderHeight)));

	menuContainer->AddChild(new Display::FillBox(menuContainer->GetSize(), 0xff000000));

	AddButton(_("Practice"))->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnPracticeClicked, this));
	AddButton(_("Multiplayer"), false)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnMultiplayerClicked, this));
	AddButton(_("Settings"), false)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnSettingsClicked, this));
	AddButton(_("Credits"), false)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnSettingsClicked, this));

	AddButton(_("Quit"))->GetClickedSignal().connect([&](Display::ClickRegion&) {
		director.RequestShutdown();
	});
}

MainMenuScene::~MainMenuScene()
{
}

std::shared_ptr<Display::Button> MainMenuScene::AddButton(const std::string &text, bool enabled)
{
	menuButtons.emplace_back(menuContainer->AddChild(new Display::Button(display, text)));
	std::shared_ptr<Display::Button> &btn = menuButtons.back();
	btn->SetEnabled(enabled);
	return btn;
}

void MainMenuScene::OnPracticeClicked()
{
	director.RequestReplaceScene(
		std::make_shared<PracticeSetupScene>(display, director, rulebookLibrary));
}

void MainMenuScene::OnMultiplayerClicked()
{
	//TODO: Push MultiplayerModeSelectScene
}

void MainMenuScene::OnSettingsClicked()
{
	//TODO: Push SettingsScene
}

void MainMenuScene::OnPhaseTransition(double interval)
{
	double titleHeight = titleContainer->GetSize().y;
	//titleContainer->SetOpacity(interval);
	titleContainer->SetPos(0, titleHeight * interval - titleHeight);
	//menuContainer->SetOpacity(interval);
	menuContainer->SetPos(0, 720 - (menuContainer->GetSize().y * interval));
}

void MainMenuScene::Layout()
{
	// Set equal spacing between each of the buttons.

	const double sidePadding = 40;

	double totalWidth = 0;
	for (std::shared_ptr<Display::Button> &btn : menuButtons) {
		totalWidth += btn->Measure().x;
	}

	double spacing = (1280.0 - totalWidth - (sidePadding * 2)) / (menuButtons.size() + 1);
	
	double x = sidePadding + spacing;
	for (std::shared_ptr<Display::Button> &btn : menuButtons) {
		btn->SetPos(x, 0);
		x += btn->Measure().x + spacing;
	}
}

}  // namespace Client
}  // namespace HoverRace
