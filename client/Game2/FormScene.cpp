
// FormScene.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../../engine/Util/Log.h"
#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Container.h"

#include "FormScene.h"

namespace Log = HoverRace::Util::Log;

namespace HoverRace {
namespace Client {

FormScene::FormScene(Display::Display &display, const std::string &name) :
	SUPER(name), display(display),
	root(std::make_shared<Display::Container>(display, Vec2(1280, 720)))
{
	root->AttachView(display);
	SetFocusRoot(root);
}

FormScene::~FormScene()
{
}

void FormScene::AttachController(Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	mouseMovedConn = controller.actions.ui.mouseMoved->Connect(
		std::bind(&FormScene::OnMouseMoved, this, std::placeholders::_1));
	mousePressedConn = controller.actions.ui.mousePressed->Connect(
		std::bind(&FormScene::OnMousePressed, this, std::placeholders::_1));
	mouseReleasedConn = controller.actions.ui.mouseReleased->Connect(
		std::bind(&FormScene::OnMouseReleased, this, std::placeholders::_1));
	mouseScrolledConn = controller.actions.ui.mouseScrolled->Connect(
		std::bind(&FormScene::OnMouseScrolled, this, std::placeholders::_1));
}

void FormScene::DetachController(Control::InputEventController &controller)
{
	mouseScrolledConn.disconnect();
	mouseReleasedConn.disconnect();
	mousePressedConn.disconnect();
	mouseMovedConn.disconnect();

	SUPER::DetachController(controller);
}

void FormScene::OnMouseMoved(const Vec2 &pos)
{
	root->OnMouseMoved(pos);
}

bool FormScene::OnMousePressed(const Control::Mouse::Click &click)
{
	return root->OnMousePressed(click);
}

bool FormScene::OnMouseReleased(const Control::Mouse::Click &click)
{
	return root->OnMouseReleased(click);
}

bool FormScene::OnMouseScrolled(const Control::Mouse::Scroll &scroll)
{
	return root->OnMouseScrolled(scroll);
}

void FormScene::PrepareRender()
{
	SUPER::PrepareRender();

	root->PrepareRender();
}

void FormScene::Render()
{
	SUPER::Render();

	root->Render();
}

}  // namespace Client
}  // namespace HoverRace
