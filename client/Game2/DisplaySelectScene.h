
// DisplaySelectScene.h
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

#include "../../engine/Display/PickList.h"
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
	class ResBucket;

public:
	int GetMonitorIdx() const;
	const Resolution &GetResolution() const;

private:
	void FilterResGrid();
	void UpdateResGrid();
	void UpdateConfirmButtons();

	void OnResTest();
	void OnResConfirm();

public:
	using confirmSignal_t = boost::signals2::signal<void()>;
	confirmSignal_t &GetConfirmSignal() { return confirmSignal; }

private:
	Resolution reqRes;
	std::shared_ptr<Display::PickList<int>> monitorList;
	std::shared_ptr<Display::PickList<std::shared_ptr<ResBucket>>> bucketList;
	std::shared_ptr<Display::PickList<Resolution>> resList;
	std::shared_ptr<Display::Button> testBtn;
	std::shared_ptr<Display::Button> confirmBtn;

	confirmSignal_t confirmSignal;

	boost::signals2::scoped_connection monitorConn;
	boost::signals2::scoped_connection bucketConn;
	boost::signals2::scoped_connection resConn;
	boost::signals2::scoped_connection testConn;
	boost::signals2::scoped_connection confirmConn;
};

}  // namespace Client
}  // namespace HoverRace
