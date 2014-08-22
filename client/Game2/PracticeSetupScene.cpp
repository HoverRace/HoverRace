
// PracticeSetupScene.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/MediaRes.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Display/Texture.h"
#include "../../engine/Display/Wallpaper.h"
#include "../../engine/Util/Config.h"

#include "GameSelectScene.h"
#include "SessionLoadingScene.h"

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
	SUPER(display, director, "PRACTICE", "Practice Setup"),
	display(display), director(director), rulebookLibrary(rulebookLibrary)
{
	using namespace Display;

	bgTex = std::make_shared<MediaRes<Texture>>("ui/bg/practice.png");
	SetBackground(new Wallpaper(bgTex, Wallpaper::Fill::ZOOM, 1.0, 0xff333333));
}

PracticeSetupScene::~PracticeSetupScene()
{
}

void PracticeSetupScene::OnScenePushed()
{
	// Immediately show the rulebook selector.
	auto selScene = std::make_shared<GameSelectScene>(display, director,
		rulebookLibrary, false);
	selScene->GetOkSignal().connect([&](std::shared_ptr<Rules> rules,
		std::shared_ptr<Display::Res<Display::Texture>> mapRes)
	{
		director.RequestNewPracticeSession(rules,
			std::make_shared<SessionLoadingScene>(display, director, mapRes));
	});
	selScene->GetCancelSignal().connect([&]() {
		director.RequestMainMenu();
	});
	director.RequestPushScene(selScene);
}

}  // namespace Client
}  // namespace HoverRace
