
// TrackSelectScene.cpp
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
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/Picture.h"
#include "../../engine/Display/RuleLine.h"
#include "../../engine/Model/TrackEntry.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "Rulebook.h"
#include "Rules.h"

#include "TrackSelectScene.h"

using HoverRace::Model::TrackEntry;
using HoverRace::Model::TrackEntryPtr;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

class TrackSelButton : public Display::Button
{
	using SUPER = Display::Button;

public:
	TrackSelButton(Display::Display &display, TrackEntryPtr entry) :
		SUPER(display, entry->name), entry(entry) { }
	virtual ~TrackSelButton() { }

public:
	TrackEntry *GetTrackEntry() const { return entry.get(); }

private:
	TrackEntryPtr entry;
};

}  // namespace

TrackSelectScene::TrackSelectScene(Display::Display &display,
	GameDirector &director, std::shared_ptr<const Rulebook> rulebook) :
	SUPER(display, director, "", "Track Select"),
	trackSelected(false),
	rules(std::make_shared<Rules>(rulebook)),
	trackList(), selectedTrack()
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SetPhaseTransitionDuration(1000);

	SetStoppingTransitionEnabled(true);
	SetBackground(nullptr);

	trackList.Reload(Config::GetInstance()->GetTrackBundle());

	const auto &s = display.styles;

	auto root = GetContentRoot();

	subtitleGrid = root->AddChild(new FlexGrid(display));
	subtitleGrid->SetMargin(10, 0);
	subtitleGrid->SetPadding(0, 0);
	subtitleGrid->SetFixedHeight(40);
	subtitleGrid->SetPos(DialogScene::MARGIN_WIDTH, 0);

	rulebookLbl = subtitleGrid->AddGridCell(0, 0, new Label(
		rulebook->GetTitle() + " //",
		UiFont(30), 0xffd0d0d0))->GetContents();
	rulebookLbl->SetAlignment(Alignment::SW);
	rulebookLbl->SetFixedScale(true);

	rulebookDescLbl = subtitleGrid->AddGridCell(0, 1, new Label(
		rulebook->GetDescription(),
		UiFont(25), 0xff6d6d6d))->GetContents();
	rulebookDescLbl->SetAlignment(Alignment::SW);
	rulebookDescLbl->SetFixedScale(true);

	subtitleRule = root->AddChild(new RuleLine(
		RuleLine::Direction::H,
		1280 - (DialogScene::MARGIN_WIDTH * 2), 1, 0xffffffff));
	subtitleRule->SetPos(DialogScene::MARGIN_WIDTH, 60);

	trackPanel = root->AddChild(new Container(display));
	trackPanel->SetPos(DialogScene::MARGIN_WIDTH, 100);

	//TODO: A better list UI (categories, sorting, etc.).
	trackGrid = trackPanel->AddChild(new FlexGrid(display));
	trackGrid->SetFixedWidth(360);

	auto &cell = trackGrid->GetColumnDefault(0);
	cell.SetFill(true);

	size_t row = 0;
	for (TrackEntryPtr ent : trackList) {
		auto trackCell = trackGrid->AddGridCell(row++, 0,
			new TrackSelButton(display, ent));
		trackCell->GetContents()->GetClickedSignal().connect(std::bind(
			&TrackSelectScene::OnTrackSelected, this, ent));
	}

	selTrackPanel = trackPanel->AddChild(new Container(display));
	selTrackPanel->SetPos(380, 0);
	selTrackPanel->SetVisible(false);

	trackPic = selTrackPanel->AddChild(new Picture(
		std::shared_ptr<Res<Texture>>(), 260, 260));
	trackPic->SetPos(0, 20);

	trackMetaGrid = selTrackPanel->AddChild(new FlexGrid(display));
	trackMetaGrid->SetPos(280, 20);
	trackMetaGrid->SetMargin(0, 0);
	trackMetaGrid->SetPadding(0, 0);

	trackNameLbl = trackMetaGrid->AddGridCell(0, 0,
		new Label(520, "Name", s.bodyHeadFont, s.bodyHeadFg))->GetContents();

	trackDescLbl = trackMetaGrid->AddGridCell(1, 0,
		new Label(520, "Desc", s.bodyAsideFont, s.bodyAsideFg))->GetContents();

	readyBtn = trackPanel->AddChild(new Button(display, _("Ready")));
	readyBtn->SetPos(1280 - (MARGIN_WIDTH * 2), 360);
	readyBtn->SetAlignment(Alignment::NE);
	readyBtn->SetEnabled(false);
	readyBtn->GetClickedSignal().connect(std::bind(
		&TrackSelectScene::OnReady, this));
}

TrackSelectScene::~TrackSelectScene()
{
}

void TrackSelectScene::OnTrackSelected(Model::TrackEntryPtr entry)
{
	Log::Info("Selected track: %s", entry->name.c_str());

	auto trackBundle = Config::GetInstance()->GetTrackBundle();

	rules->SetTrackEntry(entry);
	mapTexture = trackBundle->LoadMap(entry);

	selectedTrack = entry;
	selTrackPanel->SetVisible(true);
	trackPic->SetTexture(mapTexture);
	trackNameLbl->SetText(entry->name);
	trackDescLbl->SetText(entry->description);

	readyBtn->SetEnabled(true);
}

void TrackSelectScene::OnReady()
{
	trackSelected = true;
	okSignal(rules, mapTexture);
}

void TrackSelectScene::OnPhaseTransition(double progress)
{
	double f;
	double slideOffset = 720.0;
	if (GetPhase() == Phase::STOPPING) {
		// Slide up if a track was selected, slide down otherwise.
		if (trackSelected) {
			slideOffset = 0;
			f = 1.0 - pow(progress, 4);
		}
		else {
			f = pow(progress, 4);
		}
	}
	else {
		// Slide up.
		f = 1 - pow(1.0 - progress, 4);
	}

	subtitleGrid->SetTranslation(0, slideOffset + f * -720.0);
	subtitleRule->SetTranslation(0, slideOffset + f * -720.0);
	trackPanel->SetTranslation(0, slideOffset + f * -720.0);

	SUPER::OnPhaseTransition(progress);
}

}  // namespace Client
}  // namespace HoverRace
