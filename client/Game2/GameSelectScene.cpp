
// GameSelectScene.cpp
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
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "Rulebook.h"
#include "RulebookLibrary.h"
#include "Rules.h"
#include "TrackSelectScene.h"

#include "GameSelectScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	const double BTN_WIDTH = 240;
	const double BTN_HEIGHT = 520;
	const double BTN_GAP = 20;
}

GameSelectScene::GameSelectScene(Display::Display &display,
                                 GameDirector &director,
                                 RulebookLibrary &rulebookLibrary,
                                 bool multiplayer) :
	SUPER(display, director, "", "Rulebook Select"),
	display(display), director(director)
{
	SetPhaseTransitionDuration(1000);
	SetStateTransitionDuration(1000);

	SetBackground(nullptr);

	auto root = GetContentRoot();

	// Build the list of valid rulebooks.
	std::vector<std::shared_ptr<const Rulebook>> rulebooks;
	for (auto iter = rulebookLibrary.cbegin();
		iter != rulebookLibrary.cend(); ++iter)
	{
		auto rulebook = *iter;
		if (!multiplayer || rulebook->GetMaxPlayers() > 1) {
			rulebooks.push_back(rulebook);
		}
	}

	double panelWidth = (rulebooks.size() * (BTN_WIDTH + BTN_GAP)) - BTN_GAP;
	rulebookPanel = root->AddChild(new Display::Container(display,
		Vec2(panelWidth, BTN_HEIGHT)));
	rulebookPanel->SetPos(DialogScene::MARGIN_WIDTH, 0);

	const Vec2 btnSize(BTN_WIDTH, BTN_HEIGHT);
	double x = 0;
	for (auto rulebook : rulebooks) {
		auto btn = rulebookPanel->AddChild(
			new Display::Button(display, btnSize, rulebook->GetTitle()));
		btn->SetPos(x, 0);
		btn->GetClickedSignal().connect(std::bind(
			&GameSelectScene::OnRulebookSelected, this, rulebook));

		x += BTN_WIDTH + BTN_GAP;
	}
}

GameSelectScene::~GameSelectScene()
{
}

void GameSelectScene::OnRulebookSelected(std::shared_ptr<const Rulebook> rulebook)
{
	auto scene = std::make_shared<TrackSelectScene>(display, director, rulebook);
	scene->GetOkSignal().connect([&](std::shared_ptr<Rules> rules) {
		director.RequestPopScene();
		okSignal(rules);
	});
	scene->GetCancelSignal().connect([&]() {
		director.RequestPopScene();
		cancelSignal();
	});
	director.RequestPushScene(scene);
}

void GameSelectScene::OnOk()
{
}

void GameSelectScene::OnCancel()
{
	cancelSignal();
}

void GameSelectScene::OnPhaseTransition(double progress)
{
	double f = pow((1.0 - progress), 4);

	rulebookPanel->SetPos(f * 1280.0 + MARGIN_WIDTH, 0);

	SUPER::OnPhaseTransition(progress);
}

void GameSelectScene::OnStateTransition(double progress)
{
	if (GetPhase() != Phase::RUNNING) return;

	double f = 1 - pow(progress, 4);

	rulebookPanel->SetPos(DialogScene::MARGIN_WIDTH, f * -(BTN_HEIGHT + 1));

	SUPER::OnStateTransition(progress);
}

}  // namespace Client
}  // namespace HoverRace
