
// FormScene.cpp
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

#include "../../engine/Util/Log.h"
#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Container.h"

#include "FormScene.h"

namespace Log = HoverRace::Util::Log;

namespace HoverRace {
namespace Client {

FormScene::FormScene(Display::Display &display, const std::string &name) :
	SUPER(name), display(display)
{
	root.reset(new Display::Container(Display::Vec2(1280, 720)));
	root->AttachView(display);
}

FormScene::~FormScene()
{
}

void FormScene::AttachController(Control::InputEventController &controller)
{
	mouseMovedConn = controller.actions.ui.mouseMoved->Connect(
		std::bind(&FormScene::OnMouseMoved, this, std::placeholders::_1));
	mousePressedConn = controller.actions.ui.mousePressed->Connect(
		std::bind(&FormScene::OnMousePressed, this, std::placeholders::_1));
	mouseReleasedConn = controller.actions.ui.mouseReleased->Connect(
		std::bind(&FormScene::OnMouseReleased, this, std::placeholders::_1));
}

void FormScene::DetachController(Control::InputEventController &controller)
{
	mouseReleasedConn.disconnect();
	mousePressedConn.disconnect();
	mouseMovedConn.disconnect();
}

void FormScene::OnMouseMoved(const Display::Vec2 &pos)
{
	//TODO: Convert to UI coords and pass to root container.
}

void FormScene::OnMousePressed(const Control::Mouse::Click &click)
{
	//TODO: Convert to UI coords and pass to root container.
	Log::Info("Mouse pressed: %.2f, %.2f, %d", click.pos.x, click.pos.y, click.btn);
}

void FormScene::OnMouseReleased(const Control::Mouse::Click &click)
{
	//TODO: Convert to UI coords and pass to root container.
	Log::Info("Mouse released: %.2f, %.2f, %d", click.pos.x, click.pos.y, click.btn);
}

void FormScene::PrepareRender()
{
	root->PrepareRender();
}

void FormScene::Render()
{
	root->Render();
}

}  // namespace Client
}  // namespace HoverRace
