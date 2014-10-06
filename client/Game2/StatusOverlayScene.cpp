
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
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Player/Player.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "Announcement.h"
#include "BulletinBoard.h"
#include "Roster.h"

#include "StatusOverlayScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

StatusOverlayScene::StatusOverlayScene(Display::Display &display,
	GameDirector &director) :
	SUPER("Status Overlay"),
	display(display), director(director),
	bulletinBoard(new BulletinBoard(display, Display::UiLayoutFlags::FLOATING))
{
	using namespace Display;
	typedef UiViewModel::Alignment Alignment;

	bulletinBoard->AttachView(display);
	bulletinBoard->SetAlignment(Alignment::N);
	bulletinBoard->SetPos(640, 0);

	displayConfigChangedConn =
		display.GetDisplayConfigChangedSignal().connect(
			std::bind(&StatusOverlayScene::RequestLayout, this));

	playerAddedConn =
		director.GetConnectedPlayers()->GetPlayerAddedSignal().connect(
			std::bind(&StatusOverlayScene::OnPlayerAdded, this,
				std::placeholders::_1));
}

StatusOverlayScene::~StatusOverlayScene()
{
}

/**
 * Display an announcement.
 * @param ann The announcement (may not be @c nullptr).
 */
void StatusOverlayScene::Announce(std::shared_ptr<Announcement> ann)
{
	HR_LOG(debug) << "Announcing: " << *ann;
	bulletinBoard->Announce(ann);
}

void StatusOverlayScene::AttachController(Control::InputEventController &controller)
{
	mouseMovedConn = controller.actions.ui.mouseMoved->Connect(
		std::bind(&StatusOverlayScene::OnMouseMoved, this, std::placeholders::_1));
	mousePressedConn = controller.actions.ui.mousePressed->Connect(
		std::bind(&StatusOverlayScene::OnMousePressed, this, std::placeholders::_1));
	mouseReleasedConn = controller.actions.ui.mouseReleased->Connect(
		std::bind(&StatusOverlayScene::OnMouseReleased, this, std::placeholders::_1));
}

void StatusOverlayScene::DetachController(Control::InputEventController&)
{
	mouseReleasedConn.disconnect();
	mousePressedConn.disconnect();
	mouseMovedConn.disconnect();
}

void StatusOverlayScene::OnPlayerAdded(std::shared_ptr<Player::Player> player)
{
	HR_LOG(info) << "Player added: " << player->GetProfile()->GetName();
}

void StatusOverlayScene::OnMouseMoved(const Vec2 &pos)
{
	bulletinBoard->OnMouseMoved(pos);
}

void StatusOverlayScene::OnMousePressed(const Control::Mouse::Click &click)
{
	bulletinBoard->OnMousePressed(click);
}

void StatusOverlayScene::OnMouseReleased(const Control::Mouse::Click &click)
{
	bulletinBoard->OnMouseReleased(click);
}

void StatusOverlayScene::Layout()
{
	// Keep the board at the top-center of the screen.
	bulletinBoard->SetPos(display.GetUiScreenSize().x / 2.0, 0);
}

void StatusOverlayScene::Advance(Util::OS::timestamp_t tick)
{
	bulletinBoard->Advance(tick);
}

void StatusOverlayScene::PrepareRender()
{
	SUPER::PrepareRender();

	bulletinBoard->PrepareRender();
}

void StatusOverlayScene::Render()
{
	SUPER::Render();

	bulletinBoard->Render();
}

}  // namespace HoverScript
}  // namespace Client
