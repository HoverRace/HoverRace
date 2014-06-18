
// AudioSettingsScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Log.h"

#include "GameDirector.h"

#include "AudioSettingsScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

AudioSettingsScene::AudioSettingsScene(Display::Display &display,
                                       GameDirector &director) :
	SUPER(display, director, "Settings > Audio", "Audio Settings")
{
	typedef Display::UiViewModel::Alignment Alignment;
	const auto &s = display.styles;

	auto root = GetContentRoot();

	settingsGrid = root->AddChild(new Display::FlexGrid(display));
	settingsGrid->SetPos(640, 60);
	settingsGrid->SetAlignment(Alignment::N);

	size_t col = 0;
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::E);
		cell.SetFill(true);
	}
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::W);
		cell.SetFill(true);
	}

	size_t row = 0;
	col = 0;
	settingsGrid->AddGridCell(row, col++,
		new Display::Label("Sound Effects", s.bodyFont, s.bodyFg));
	settingsGrid->AddGridCell(row, col++,
		new Display::Label("( disabled )", s.bodyFont, s.bodyFg));
}

AudioSettingsScene::~AudioSettingsScene()
{
}

}  // namespace Client
}  // namespace HoverRace
