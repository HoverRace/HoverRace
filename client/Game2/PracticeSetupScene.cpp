
// PracticeSetupScene.cpp
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
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Config.h"

#include "GameSelectScene.h"

#include "PracticeSetupScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param display The current display.
 * @param director The current game director.
 * @param rulebookLibrary The library of registered rulebooks.
 */
PracticeSetupScene::PracticeSetupScene(Display::Display &display,
                                       GameDirector &director,
                                       RulebookLibrary &rulebookLibrary) :
	SUPER(display, "PRACTICE", "Practice Setup"),
	display(display), director(director), rulebookLibrary(rulebookLibrary)
{
	fader.reset(new Display::ScreenFade(Display::COLOR_BLACK, 1.0));
	fader->AttachView(display);
}

PracticeSetupScene::~PracticeSetupScene()
{
}

void PracticeSetupScene::OnScenePushed()
{
	// Immediately show the rulebook selector.
	auto selScene = std::make_shared<GameSelectScene>(display, director,
		rulebookLibrary, false);
	selScene->GetOkSignal().connect([&](std::shared_ptr<Rules> rules) {
		director.RequestNewPracticeSession(rules);
	});
	selScene->GetCancelSignal().connect([&]() {
		director.RequestMainMenu();
	});
	director.RequestPushScene(selScene);
}

void PracticeSetupScene::PrepareRender()
{
	fader->PrepareRender();

	SUPER::PrepareRender();
}

void PracticeSetupScene::Render()
{
	fader->Render();

	SUPER::Render();
}

}  // namespace Client
}  // namespace HoverRace
