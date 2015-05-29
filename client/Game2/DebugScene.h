
// DebugScene.h
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

#pragma once

#include "GameDirector.h"

#include "UiScene.h"

namespace HoverRace {
	namespace Client {
		class Scene;
	}
	namespace Display {
		class Display;
		class ActiveText;
		class FillBox;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Overlay for displaying debug information.
 * @author Michael Imamura
 */
class DebugScene : public UiScene
{
	using SUPER = UiScene;

public:
	DebugScene(Display::Display &display, GameDirector &director);
	virtual ~DebugScene();

public:
	bool IsMouseCursorEnabled() const override { return true; }

public:
	void Advance(Util::OS::timestamp_t tick) override;
	void PrepareRender() override;
	void Render() override;

private:
	GameDirector &director;
	Util::OS::timestamp_t prevUpdateTick;
	std::unique_ptr<Display::ActiveText> debugLbl;
	std::unique_ptr<Display::FillBox> debugBox;
};

}  // namespace HoverScript
}  // namespace Client
