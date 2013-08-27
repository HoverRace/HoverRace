
// TrackSelectScene.cpp
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
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Model/TrackEntry.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "Rulebook.h"

#include "TrackSelectScene.h"

using HoverRace::Model::TrackEntry;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	class TrackSelButton : public Display::Button
	{
		typedef Display::Button SUPER;
		public:
			TrackSelButton(Display::Display &display, TrackEntry *entry) :
				SUPER(display, entry->name), entry(entry) { }
			virtual ~TrackSelButton() { }

		public:
			TrackEntry *GetTrackEntry() const { return entry; }

		private:
			TrackEntry *entry;
	};
}

TrackSelectScene::TrackSelectScene(Display::Display &display, GameDirector &director) :
	SUPER(display, "Track Select"),
	display(display), director(director),
	trackList()
{
	trackList.Reload(Config::GetInstance()->GetTrackBundle());

	auto root = GetRoot();

	//TODO: A better list UI (categories, sorting, etc.).
	double y = 60;
	BOOST_FOREACH(TrackEntry *ent, trackList) {
		auto trackSel = root->AddChild(new TrackSelButton(display, ent));
		trackSel->SetPos(60, y);
		trackSel->GetClickedSignal().connect(std::bind(
			&TrackSelectScene::OnTrackSelected, this, ent));
		y += 50;
	}
}

TrackSelectScene::~TrackSelectScene()
{
}

void TrackSelectScene::OnTrackSelected(Model::TrackEntry *entry)
{
	Log::Info("Selected track: %s", entry->name.c_str());

	okSignal(std::make_shared<Rulebook>(entry->name, 1, 0x7f));
}

}  // namespace Client
}  // namespace HoverRace
