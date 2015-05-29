
// DebugScene.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include <boost/algorithm/string.hpp>

#include "../../engine/Display/ActiveText.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Display.h"

#include "DebugScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

DebugScene::DebugScene(Display::Display &display,
	GameDirector &director) :
	SUPER("Debug Overlay"),
	director(director), prevUpdateTick(0)
{
	using namespace Display;

	const auto &s = display.styles;

	debugBox.reset(new FillBox(1, 1, 0x3f000000));
	debugBox->AttachView(display);
	debugBox->SetPos(0, 30);

	debugLbl.reset(new ActiveText("", s.bodyFont, COLOR_WHITE));
	debugLbl->AttachView(display);
	debugLbl->SetPos(10, 40);
}

DebugScene::~DebugScene()
{
}

void DebugScene::Advance(Util::OS::timestamp_t tick)
{
	// Update the debug text no more than roughly 60 Hz.
	if (prevUpdateTick == 0 || Util::OS::TimeDiff(tick, prevUpdateTick) > 17) {
		prevUpdateTick = tick;
		Scene *scene = director.GetForegroundScene();

		if (!scene) {
			debugLbl->SetText("No active foreground scene.");
		}
		else {
			std::ostringstream oss;
			oss << "Scene: " << scene->GetName();
			scene->OutputDebugText(oss);
			auto s = oss.str();
			boost::trim_right(s);
			debugLbl->SetText(s);
		}

		auto sz = debugLbl->Measure();
		debugBox->SetSize(sz.x + 20, sz.y + 20);
	}

	SUPER::Advance(tick);
}

void DebugScene::PrepareRender()
{
	SUPER::PrepareRender();

	debugBox->PrepareRender();
	debugLbl->PrepareRender();
}

void DebugScene::Render()
{
	SUPER::Render();

	debugBox->Render();
	debugLbl->Render();
}

}  // namespace HoverScript
}  // namespace Client
