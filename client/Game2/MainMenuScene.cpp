
// MainMenuScene.cpp
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/SymbolIcon.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "MessageScene.h"
#include "PracticeSetupScene.h"
#include "Rulebook.h"
#include "SettingsMenuScene.h"

#include "MainMenuScene.h"

using namespace HoverRace::Util;
using namespace HoverRace::VideoServices;

namespace HoverRace {
namespace Client {

MainMenuScene::MainMenuScene(Display::Display &display, GameDirector &director,
	RulebookLibrary &rulebookLibrary) :
	SUPER(display, "Main Menu"),
	display(display), director(director), rulebookLibrary(rulebookLibrary)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SetPhaseTransitionDuration(500);
	SetStateTransitionDuration(500);

	auto root = GetRoot();

	double sliderHeight = 120;

	// Letterbox mask.
	// These will be sized and positioned in Layout().
	letterUpBox.reset(new FillBox(0, 0, 0xff000000,
		UiLayoutFlags::FLOATING | UiLayoutFlags::UNSCALED));
	letterUpBox->AttachView(display);
	letterDownBox.reset(new FillBox(0, 0, 0xff000000,
		UiLayoutFlags::FLOATING | UiLayoutFlags::UNSCALED));
	letterDownBox->AttachView(display);

	titleContainer = root->NewChild<Container>(display,
		Vec2(1280, sliderHeight));

	auto titleLbl = titleContainer->NewChild<Label>(
		"HoverRace",
		UiFont(40, UiFont::BOLD | UiFont::ITALIC),
		0xffffffff);
	titleLbl->SetPos(40, sliderHeight);
	titleLbl->SetAlignment(Alignment::SW);
	titleLbl->SetFixedScale(true);

	menuContainer = root->NewChild<Container>(display,
		Vec2(1280, sliderHeight));
	menuContainer->SetPos(0, 720 - sliderHeight + 2);

	AddButton(_("Practice"), true, true)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnPracticeClicked, this));
	AddButton(_("Multiplayer"), false)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnMultiplayerClicked, this));
	AddButton(_("Settings"))->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnSettingsClicked, this));
	AddButton(_("Credits"), false)->GetClickedSignal().connect(
		std::bind(&MainMenuScene::OnSettingsClicked, this));

	AddButton(_("Quit"))->GetClickedSignal().connect([&](ClickRegion&) {
		director.RequestShutdown();
	});

	if (SoundServer::IsDisabled()) {
		auto icon = std::make_shared<SymbolIcon>(1, 1, 0xf026, COLOR_WHITE);
		icon->AttachView(display);

		auto mutedBtn = titleContainer->NewChild<Button>(
			display, _("Silent Mode"));
		mutedBtn->SetIcon(icon);
		mutedBtn->SetAlignment(Alignment::NE);
		mutedBtn->SetPos(1280, 0);
		mutedBtn->GetClickedSignal().connect(
			std::bind(&MainMenuScene::OnMutedClicked, this));
	}

	displayConfigChangedConn =
		display.GetDisplayConfigChangedSignal().connect(
			std::bind(&MainMenuScene::RequestLayout, this));
}

MainMenuScene::~MainMenuScene()
{
}

std::shared_ptr<Display::Button> MainMenuScene::AddButton(
	const std::string &text, bool enabled, bool focused)
{
	using namespace Display;

	menuButtons.emplace_back(menuContainer->NewChild<Button>(display, text));
	std::shared_ptr<Button> &btn = menuButtons.back();
	btn->SetEnabled(enabled);
	if (focused) {
		btn->RequestFocus();
	}

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
	director.RequestPushScene(std::make_shared<SettingsMenuScene>(display, director));
}

void MainMenuScene::OnMutedClicked()
{
	std::string msg;

	if (Config::GetInstance()->runtime.silent) {
		msg = _("Sound has been disabled because the --silent command-line "
			"parameter was set.");
	}
	else {
		msg = boost::str(boost::format(
			_("There was a problem starting the sound system.\n\n"
			"The error was:\n%s")) %
			SoundServer::GetInitError());
	}

	director.RequestPushScene(std::make_shared<MessageScene>(display, director,
		_("Silent Mode"), msg));
}

void MainMenuScene::OnStateChanged(State oldState)
{
	// When we're finally visible (and all the initial dialog boxes are out
	// of the way), display to the user the status.
	if (GetState() == State::FOREGROUND) {
		director.RequestStatusPresentation();
	}
	SUPER::OnStateChanged(oldState);
}

void MainMenuScene::OnStateTransition(double interval)
{
	double f = pow((1.0 - interval), 4);

	auto color = Display::COLOR_BLACK;
	color.bits.a = 255 - static_cast<MR_UInt8>(f * 255.0);

	double sliderHeight = letterUpBox->GetSize().y;
	letterUpBox->SetTranslation(0, f * -sliderHeight);
	letterUpBox->SetColor(color);
	letterDownBox->SetTranslation(0, f * sliderHeight);
	letterDownBox->SetColor(color);

	double titleHeight = titleContainer->GetSize().y;
	titleContainer->SetTranslation(0, f * -titleHeight);
	menuContainer->SetTranslation(0, f * titleHeight);
}

void MainMenuScene::Layout()
{
	const auto &vidCfg = Config::GetInstance()->video;
	double w = vidCfg.xRes;
	double h = vidCfg.yRes;

	// Position the letterbox mask, since it extends beyond the UI area.

	auto uiScale = display.GetUiScale();
	auto &uiOffset = display.GetUiOffset();

	double sliderHeight = uiOffset.y + (120.0 * uiScale);

	letterUpBox->SetPos(0, 0);
	letterUpBox->SetSize(w, sliderHeight);

	letterDownBox->SetPos(0, h - sliderHeight + 1);
	letterDownBox->SetSize(w, sliderHeight);

	// Set equal spacing between each of the buttons.

	const double sidePadding = 40;

	double totalWidth = 0;
	for (std::shared_ptr<Display::Button> &btn : menuButtons) {
		totalWidth += btn->Measure().x;
	}

	double spacing =
		(1280.0 - totalWidth - (sidePadding * 2)) /
		static_cast<double>(menuButtons.size() + 1);

	double x = sidePadding + spacing;
	for (std::shared_ptr<Display::Button> &btn : menuButtons) {
		btn->SetPos(x, 0);
		x += btn->Measure().x + spacing;
	}
}

void MainMenuScene::PrepareRender()
{
	letterUpBox->PrepareRender();
	letterDownBox->PrepareRender();

	SUPER::PrepareRender();
}

void MainMenuScene::Render()
{
	letterUpBox->Render();
	letterDownBox->Render();

	SUPER::Render();
}

}  // namespace Client
}  // namespace HoverRace
