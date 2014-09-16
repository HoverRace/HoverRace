
// StatusOverlayScene.cpp
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Util/Log.h"

#include "StatusOverlayScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

StatusOverlayScene::StatusOverlayScene(Display::Display &display,
	GameDirector &director) :
	SUPER("Status Overlay"),
	display(display), director(director)
{
}

StatusOverlayScene::~StatusOverlayScene()
{
}

void StatusOverlayScene::AttachController(Control::InputEventController &controller)
{
	HR_LOG(info) << "Attaching overlay scene.";
}

void StatusOverlayScene::DetachController(Control::InputEventController &controller)
{
	HR_LOG(info) << "Detaching overlay scene.";
}

void StatusOverlayScene::PrepareRender()
{
	SUPER::PrepareRender();
}

void StatusOverlayScene::Render()
{
	SUPER::Render();
}

}  // namespace HoverScript
}  // namespace Client
