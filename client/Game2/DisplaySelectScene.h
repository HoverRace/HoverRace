
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
		const std::string &parentTitle, int monitorIdx, int xRes, int yRes,
		int refreshRate);
	virtual ~DisplaySelectScene() { }

public:
	struct Resolution
	{
		Resolution(int xRes, int yRes, int refreshRate) :
			xRes(xRes), yRes(yRes), refreshRate(refreshRate) { }

		int xRes;
		int yRes;
		int refreshRate;

		bool operator==(const Resolution &o) const
		{
			return xRes == o.xRes &&
				yRes == o.yRes &&
				refreshRate == o.refreshRate;
		}
	};

public:
	int GetMonitorIdx() const;
	const Resolution &GetResolution() const;

private:
	void UpdateResGrid();

public:
	using okSignal_t = boost::signals2::signal<void()>;
	okSignal_t &GetOkSignal() { return okSignal; }

	void OnOk() override;

private:
	Resolution reqRes;
	Display::RadioGroup<int> monitorGroup;
	std::unique_ptr<Display::RadioGroup<Resolution>> resGroup;
	std::shared_ptr<Display::FlexGrid> resGrid;

	okSignal_t okSignal;

	boost::signals2::scoped_connection monitorConn;
};

}  // namespace Client
}  // namespace HoverRace
