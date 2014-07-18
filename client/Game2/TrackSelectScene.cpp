
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

#include "StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/RuleLine.h"
#include "../../engine/Model/TrackEntry.h"
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
		typedef Display::Button SUPER;
		public:
			TrackSelButton(Display::Display &display, TrackEntryPtr entry) :
				SUPER(display, entry->name), entry(entry) { }
			virtual ~TrackSelButton() { }

		public:
			TrackEntry *GetTrackEntry() const { return entry.get(); }

		private:
			TrackEntryPtr entry;
	};
}

TrackSelectScene::TrackSelectScene(Display::Display &display,
                                   GameDirector &director,
                                   std::shared_ptr<const Rulebook> rulebook) :
	SUPER(display, director, "", "Track Select"),
	rules(std::make_shared<Rules>(rulebook)),
	trackList()
{
	typedef Display::UiViewModel::Alignment Alignment;

	SetPhaseTransitionDuration(1000);

	trackList.Reload(Config::GetInstance()->GetTrackBundle());

	Config *cfg = Config::GetInstance();
	const std::string &fontName = cfg->GetDefaultFontName();

	auto root = GetContentRoot();

	subtitleGrid = root->AddChild(new Display::FlexGrid(display));
	subtitleGrid->SetMargin(10, 0);
	subtitleGrid->SetPadding(0, 0);
	subtitleGrid->SetFixedHeight(40);
	subtitleGrid->SetPos(DialogScene::MARGIN_WIDTH, 0);

	rulebookLbl = subtitleGrid->AddGridCell(0, 0, new Display::Label(
		rulebook->GetTitle() + " //",
		Display::UiFont(fontName, 30), 0xffd0d0d0))->GetContents();
	rulebookLbl->SetAlignment(Alignment::SW);

	rulebookDescLbl = subtitleGrid->AddGridCell(0, 1, new Display::Label(
		rulebook->GetDescription(),
		Display::UiFont(fontName, 25), 0xff6d6d6d))->GetContents();
	rulebookDescLbl->SetAlignment(Alignment::SW);

	subtitleRule = root->AddChild(new Display::RuleLine(
		Display::RuleLine::Direction::H,
		1280 - (DialogScene::MARGIN_WIDTH * 2), 1, 0xffffffff));
	subtitleRule->SetPos(DialogScene::MARGIN_WIDTH, 60);

	trackPanel = root->AddChild(new Display::Container(display));
	trackPanel->SetPos(DialogScene::MARGIN_WIDTH, 100);

	//TODO: A better list UI (categories, sorting, etc.).
	trackGrid = trackPanel->AddChild(new Display::FlexGrid(display));
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
}

TrackSelectScene::~TrackSelectScene()
{
}

void TrackSelectScene::OnTrackSelected(Model::TrackEntryPtr entry)
{
	Log::Info("Selected track: %s", entry->name.c_str());

	rules->SetTrackEntry(entry);

	okSignal(rules);
}

void TrackSelectScene::OnPhaseTransition(double progress)
{
	double f = 1 - pow(1.0 - progress, 4);

	subtitleGrid->SetPos(MARGIN_WIDTH, 720.0 + f * -720.0);
	subtitleRule->SetPos(MARGIN_WIDTH, 780.0 + f * -720.0);
	trackPanel->SetPos(MARGIN_WIDTH, 820.0 + f * -720.0);
}

}  // namespace Client
}  // namespace HoverRace
