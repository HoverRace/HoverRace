
// SessionLoadingScene.cpp
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

#include "../../engine/Display/Container.h"
#include "../../engine/Display/Picture.h"

#include "SessionLoadingScene.h"

namespace HoverRace {
namespace Client {

/**
 * Constructor.
 * @param display The target display.
 * @param director The game director.
 * @param mapRes The map texture resource for the track being loaded
 *               (may be @c nullptr).
 */
SessionLoadingScene::SessionLoadingScene(Display::Display &display,
	GameDirector &director,
	std::shared_ptr<Display::Res<Display::Texture>> mapRes) :
	SUPER(display, director, "Session Loading")
{
	using namespace Display;
	typedef UiViewModel::Alignment Alignment;

	auto root = GetRoot();

	mapPic = root->AddChild(new Picture(mapRes, 300, 300));
	mapPic->SetAlignment(Alignment::CENTER);
	mapPic->SetPos(640, 360);
}

SessionLoadingScene::~SessionLoadingScene()
{
}

void SessionLoadingScene::OnPhaseTransition(double progress)
{
	if (GetPhase() == Phase::STOPPING) {
		// Fade the map out.
		auto alpha = static_cast<MR_UInt8>(progress * 0xff);
		mapPic->SetColor(Display::Color(alpha,  0xff, 0xff, 0xff));
	} else {
		// Slide up.
		double f = 1 - pow(1.0 - progress, 4);
		mapPic->SetTranslation(0, 720.0 + (f * -720.0));
		mapPic->SetColor(Display::COLOR_WHITE);
	}

	SUPER::OnPhaseTransition(progress);
}

}  // namespace Client
}  // namespace HoverRace
