
// MessageScene.cpp
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

#include "../../engine/Display/Display.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/Util/Config.h"

#include "MessageScene.h"

#define FINAL_OPACITY 0.8
#define START_DURATION 500

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

MessageScene::MessageScene(Display::Display &display,
                           const std::string &title,
                           const std::string &message) :
	SUPER(),
	display(display)
{
	Config *cfg = Config::GetInstance();

	fader = new Display::ScreenFade(Display::Color(0xff, 0x00, 0x00, 0x00), 0.0);
	fader->AttachView(&display);

	std::string fontName = cfg->GetDefaultFontName();

	titleLbl = new Display::Label(title,
		Display::UiFont(fontName, 40, Display::UiFont::BOLD),
		Display::Color(0xff, 0xff, 0xff, 0xff));
	titleLbl->AttachView(&display);

	messageLbl = new Display::Label(message,
		Display::UiFont(fontName, 20),
		Display::Color(0xff, 0xbf, 0xbf, 0xbf));
	messageLbl->AttachView(&display);
}

MessageScene::~MessageScene()
{
	delete messageLbl;
	delete titleLbl;
	delete fader;
}

void MessageScene::Advance(Util::OS::timestamp_t tick)
{
	switch (GetPhase()) {
		case Phase::STARTING: {
			Util::OS::timestamp_t duration = GetPhaseDuration(tick);
			if (duration > START_DURATION) {
				fader->SetOpacity(FINAL_OPACITY);
				SetPhase(Phase::RUNNING);
			}
			else {
				// Fade-in the screen fader.
				double opacity = static_cast<double>(duration) * FINAL_OPACITY / START_DURATION;
				fader->SetOpacity(opacity);
			}
			break;
		}

		case Phase::STOPPING: {
			Util::OS::timestamp_t duration = GetPhaseDuration(tick);
			Util::OS::timestamp_t startingDuration = GetStartingPhaseTime();
			if (duration > startingDuration) {
				fader->SetOpacity(0.0);
				SetPhase(Phase::STOPPED);
			}
			else {
				// Fade-out the screen fader.
				double opacity = static_cast<double>(startingDuration - duration) * FINAL_OPACITY / startingDuration;
				fader->SetOpacity(opacity);
			}
			break;
		}
	}
}

void MessageScene::OnPhaseChanged(Phase::phase_t oldPhase)
{
	std::ostringstream oss;
	oss << "Phase change: " << oldPhase << " -> " << GetPhase() << '\n';
	OutputDebugString(oss.str().c_str());
	// Do nothing.
}

void MessageScene::PrepareRender()
{
	//TODO: Only update positions when screen size changes.
	Config::cfg_video_t &vidCfg = Config::GetInstance()->video;

	int leftMargin = vidCfg.xRes / 50;
	int centerH = vidCfg.yRes / 2;
	titleLbl->SetPos(Display::Vec2(leftMargin, centerH - 50));
	messageLbl->SetPos(Display::Vec2(leftMargin, centerH));

	fader->PrepareRender();
	titleLbl->PrepareRender();
	messageLbl->PrepareRender();
}

void MessageScene::Render()
{
	fader->Render();
	titleLbl->Render();
	messageLbl->Render();
}

}  // namespace HoverScript
}  // namespace Client
