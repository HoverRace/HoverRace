
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

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Label.h"

#include "SettingsScene.h"

namespace HoverRace {
namespace Client {

/// Default size for settings sliders.
Vec2 SettingsScene::SLIDER_SIZE = Vec2(300, 20);


SettingsScene::SettingsScene(Display::Display &display, GameDirector &director,
	const std::string &parentTitle, const std::string &title,
	const std::string &name) :
	SUPER(display, director, JoinTitles(parentTitle, title), name),
	curRow(0), needsLoadFromConfig(true)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	auto statusRoot = GetStatusRoot();

	ActivateExtraAction(_("Reset to defaults"));

	auto root = GetContentRoot();

	settingsGrid = root->AddChild(new FlexGrid(display));
	settingsGrid->SetPos(640, 60);
	settingsGrid->SetMargin(20, 10);
	settingsGrid->SetAlignment(Alignment::N);

	size_t col = 0;
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::E);
	}
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::W);
	}
}

void SettingsScene::AddSettingLabel(size_t row, const std::string &label)
{
	const auto &s = display.styles;

	settingsGrid->AddGridCell(row, 0,
		new Display::Label(label, s.bodyFont, s.bodyFg));
}

void SettingsScene::OnExtra()
{
	ResetToDefaults();
	LoadFromConfig();
}

void SettingsScene::PrepareRender()
{
	if (needsLoadFromConfig) {
		LoadFromConfig();
		needsLoadFromConfig = false;
	}

	SUPER::PrepareRender();
}

}  // namespace Client
}  // namespace HoverRace
