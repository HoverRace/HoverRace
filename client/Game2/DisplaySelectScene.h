
// DisplaySelectScene.h
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

#pragma once

#include "../../engine/Display/RadioButton.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Selector for monitor and resolution.
 * @author Michael Imamura
 */
class DisplaySelectScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	DisplaySelectScene(Display::Display &display, GameDirector &director,
		int monitorIdx, int xRes, int yRes);
	virtual ~DisplaySelectScene() { }

private:
	void UpdateResGrid();

private:
	Display::RadioGroup<int> monitorGroup;
	std::unique_ptr<Display::RadioGroup<int>> resGroup;
	std::shared_ptr<Display::FlexGrid> resGrid;

	boost::signals2::scoped_connection monitorConn;
};

}  // namespace Client
}  // namespace HoverRace
