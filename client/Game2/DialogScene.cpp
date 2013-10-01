
// FormScene.cpp
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

#include "../../engine/Display/Container.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "DialogScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

const double DialogScene::MARGIN_WIDTH = 40;

/**
 * Constructor.
 * @param display The display.
 * @param title The title to display.
 * @param name The name of the scene.  See Scene::GetName.
 */
DialogScene::DialogScene(Display::Display &display,
                         const std::string &title,
                         const std::string &name) :
	SUPER(display, name),
	title(title)
{
	typedef Display::UiViewModel::Alignment Alignment;

	Config *cfg = Config::GetInstance();
	const std::string &fontName = cfg->GetDefaultFontName();

	auto root = GetRoot();

	contentRoot = root->AddChild(new Display::Container(
		display, Display::Vec2(1280, 720 - 200)));
	contentRoot->SetPos(0, 100);

	statusRoot = root->AddChild(new Display::Container(
		display, Display::Vec2(1280 - (MARGIN_WIDTH * 2), 40), false));
	statusRoot->SetPos(MARGIN_WIDTH, 720 - 80);

	auto titleLbl = root->AddChild(new Display::Label(title,
		Display::UiFont(fontName, 40, Display::UiFont::BOLD),
		Display::COLOR_WHITE));
	titleLbl->SetPos(MARGIN_WIDTH, 80);
	titleLbl->SetAlignment(Alignment::SW);
}

DialogScene::~DialogScene()
{
}

}  // namespace Client
}  // namespace HoverRace
