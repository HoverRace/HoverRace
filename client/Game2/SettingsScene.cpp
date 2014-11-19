
// SettingsScene.cpp
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

#include "../StdAfx.h"

#include "../../engine/Display/Label.h"

#include "SettingsScene.h"

namespace HoverRace {
namespace Client {

SettingsScene::SettingsScene(Display::Display &display, GameDirector &director,
	const std::string &title, const std::string &name) :
	SUPER(display, director, std::string(_("Settings")) + " // " + title, name),
	curRow(0)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;
	const auto &s = display.styles;

	auto root = GetContentRoot();

	settingsGrid = root->AddChild(new FlexGrid(display));
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
}

void SettingsScene::AddSettingLabel(size_t row, const std::string &label)
{
	const auto &s = display.styles;

	settingsGrid->AddGridCell(row, 0,
		new Display::Label(label, s.bodyFont, s.bodyFg));
}

}  // namespace Client
}  // namespace HoverRace
