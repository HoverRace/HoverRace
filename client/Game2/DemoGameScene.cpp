
// DemoGameScene.cpp
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

#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/Log.h"

#include "Rulebook.h"
#include "Rules.h"

#include "DemoGameScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

std::shared_ptr<Rules> GenRules(Script::Core *scripting)
{
	// Pick a random track from our standard list.
	const char *tracks[] = {
		"ClassicH",
		"Steeplechase",
		"The Alley2",
		"The River",
	};
	const char *trackName = tracks[(unsigned)rand() % (sizeof(tracks) / sizeof(tracks[0]))];
	Log::Info("Selected main menu track: %s", trackName);

	// Pick a random craft.
	char craftId = static_cast<char>(1 << (rand() % 4));

	// Use a special "dummy" rulebook for the demo mode.
	auto rulebook = std::make_shared<Rulebook>(scripting, OS::path_t());
	rulebook->SetMetadata("Demo", "Demo", "", 1);
	auto rules = std::make_shared<Rules>(rulebook);
	rules->SetTrackEntry(Config::GetInstance()->GetTrackBundle()->
		OpenTrackEntry(trackName));
	rules->SetGameOpts(static_cast<char>(0x70 + craftId));

	return rules;
}

}  // namespace

DemoGameScene::DemoGameScene(Display::Display &display, GameDirector &director,
                             Script::Core *scripting,
                             std::shared_ptr<Loader> loader) :
	SUPER("Demo", display, director, scripting, GenRules(scripting), loader)
{
}

DemoGameScene::~DemoGameScene()
{
}

void DemoGameScene::OnFinishedLoading()
{
	SUPER::OnFinishedLoading();

	SetHudVisible(false);
	SetMuted(true);

	for (auto &viewport : viewports) {
		viewport.observer->StartDemoMode();
	}
}

}  // namespace HoverScript
}  // namespace Client
