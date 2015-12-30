
// StatusOverlayScene.h
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#pragma once

#include "GameDirector.h"

#include "UiScene.h"

namespace HoverRace {
	namespace Client {
		class Announcement;
		class BulletinBoard;
		class PlayerBar;
	}
	namespace Display {
		class Display;
	}
	namespace Player {
		class Player;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Overlay for showing connected players, pop-up messages, etc.
 * @author Michael Imamura
 */
class StatusOverlayScene : public UiScene
{
	using SUPER = UiScene;

public:
	StatusOverlayScene(Display::Display &display, GameDirector &director);
	virtual ~StatusOverlayScene();

public:
	bool IsMouseCursorEnabled() const override { return true; }

public:
	void Announce(std::shared_ptr<Announcement> ann);

public:
	void AttachController(Control::InputEventController &controller,
		ConnList &conns) override;

private:
	void OnDisplayConfigChanged();
	void OnMouseMoved(const Vec2 &pos);
	bool OnMousePressed(const Control::Mouse::Click &click);
	bool OnMouseReleased(const Control::Mouse::Click &click);

public:
	void Layout() override;
	void Advance(Util::OS::timestamp_t tick) override;
	void PrepareRender() override;
	void Render() override;

private:
	Display::Display &display;
	GameDirector &director;
	std::unique_ptr<BulletinBoard> bulletinBoard;
	std::unique_ptr<PlayerBar> playerBar;
	boost::signals2::scoped_connection displayConfigChangedConn;
};

}  // namespace HoverScript
}  // namespace Client
