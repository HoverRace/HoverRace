
// LoadingScene.cpp
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

#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Util/Log.h"

#include "LoadingScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

LoadingScene::LoadingScene(Display::Display &display, GameDirector &director,
                           const std::string &name) :
	SUPER(display, name),
	director(director), loading(false)
{
	using namespace Display;

	SetPhaseTransitionDuration(1000);

	const auto &s = display.styles;

	fader.reset(new ScreenFade(s.dialogBg, 0.0));
	fader->AttachView(display);
}

LoadingScene::~LoadingScene()
{
}

void LoadingScene::OnPhaseChanged(Phase oldPhase)
{
	if (GetPhase() == Phase::RUNNING) {
		loading = true;
	}

	SUPER::OnPhaseChanged(oldPhase);
}

void LoadingScene::OnPhaseTransition(double progress)
{
	if (fader) {
		fader->SetOpacity(progress);
	}

	SUPER::OnPhaseTransition(progress);
}

void LoadingScene::PrepareRender()
{
	if (loading) {
		//TODO: Load the next loadable.
		//TODO: Only finish when actually finished.
		if (loaders.empty()) {
			loading = false;

			// We assume that we're the foreground scene.
			// We pop ourselves before triggering the signal so that the
			// handler can push a new scene in response.
			director.RequestPopScene();
			finishedLoadingSignal();
		}
		else {
			auto &loader = loaders.front();
			Log::Info("%s: Loading: %s",
				GetName().c_str(), loader.first.c_str());
			loader.second();
			loaders.pop_front();
		}
	}

	if (fader) {
		fader->PrepareRender();
	}

	SUPER::PrepareRender();
}

void LoadingScene::Render()
{
	if (fader) {
		fader->Render();
	}

	SUPER::Render();
}

}  // namespace Client
}  // namespace HoverRace
