
// DisplaySelectScene.cpp
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

#include <SDL2/SDL.h>

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"

#include "MessageScene.h"

#include "DisplaySelectScene.h"

using namespace HoverRace::Util;

namespace {
const int MIN_RES_W = 1024;
const int MIN_RES_H = 576;
}  // namespace

namespace HoverRace {
namespace Client {

class DisplaySelectScene::ResBucket
{
public:
	virtual ~ResBucket() { }

public:
	using Resolution = DisplaySelectScene::Resolution;

	virtual void FilterResList(Display::PickList<Resolution> *list) = 0;
};

namespace {

class AllBucket : public DisplaySelectScene::ResBucket
{
public:
	virtual ~AllBucket() { }

public:
	void FilterResList(Display::PickList<Resolution> *list) override
	{
		list->RemoveFilter();
	}
};

class AspectBucket : public DisplaySelectScene::ResBucket
{
public:
	AspectBucket(int rx, int ry) : rx(rx), ry(ry) { }
	virtual ~AspectBucket() { }

public:
	void FilterResList(Display::PickList<Resolution> *list) override
	{
		list->ApplyFilter([=](const Resolution &res) {
			return
				(res.xRes % rx) == 0 &&
				(res.yRes % ry) == 0 &&
				(res.xRes / rx) == (res.yRes / ry);
		});
	}

private:
	int rx;
	int ry;
};

}  // namespace

DisplaySelectScene::DisplaySelectScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle,
	int monitorIdx, int xRes, int yRes, int refreshRate) :
	SUPER(display, director, JoinTitles(parentTitle, _("Select Resolution")),
		"Display Select"),
	reqRes(xRes, yRes, refreshRate)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SupportCancelAction();

	auto root = GetContentRoot();

	monitorList = root->NewChild<PickList<int>>(display, Vec2(260, 520));
	monitorList->SetPos(40, 0);

	int numMonitors = SDL_GetNumVideoDisplays();
	if (numMonitors == 0) {
		throw Exception("No monitors detected.");
	}
	else if (numMonitors < 0) {
		throw Exception(std::string("No monitors detected: ") + SDL_GetError());
	}

	for (int i = 0; i < numMonitors; i++) {
		auto name = boost::str(boost::format(_("Monitor %d")) % (i + 1));
		monitorList->Add(name, i);
	}
	monitorList->SetValue(monitorIdx);

	if (numMonitors > 1) {
		monitorConn = monitorList->GetValueChangedSignal().connect(std::bind(
			&DisplaySelectScene::UpdateResGrid, this));
	}

	bucketList = root->NewChild<PickList<std::shared_ptr<ResBucket>>>(
		display, Vec2(260, 520));
	auto allBucket = std::make_shared<AllBucket>();
	bucketList->Add(_("All"), allBucket);
	bucketList->Add("4:3", std::make_shared<AspectBucket>(4, 3));
	bucketList->Add("16:9", std::make_shared<AspectBucket>(16, 9));
	bucketList->Add("16:10", std::make_shared<AspectBucket>(16, 10));
	bucketList->SetPos(320, 0);
	bucketList->SetValue(allBucket);
	bucketConn = bucketList->GetValueChangedSignal().connect(std::bind(
		&DisplaySelectScene::FilterResGrid, this));

	resList = root->NewChild<PickList<Resolution>>(display, Vec2(260, 520));
	resList->SetPos(600, 0);

	auto infoBox = root->NewChild<FillBox>(360, 520, 0, 1, COLOR_WHITE);
	infoBox->SetPos(880, 0);

	auto goGrid = root->NewChild<FlexGrid>(display);
	goGrid->SetPos(940, 340);
	goGrid->SetFixedWidth(240);
	goGrid->SetMargin(0, 20);
	goGrid->GetColumnDefault(0).SetFill(true);

	size_t r = 0;

	testBtn = goGrid->At(r++, 0).NewChild<Button>(display,
		pgettext("Display Resolution", "Test"))->
		GetContents();
	testConn = testBtn->GetClickedSignal().connect(
		std::bind(&DisplaySelectScene::OnResTest, this));

	confirmBtn = goGrid->At(r++, 0).NewChild<Button>(display,
		pgettext("Display Resolution", "Confirm"))->
		GetContents();
	confirmConn = confirmBtn->GetClickedSignal().connect(
		std::bind(&DisplaySelectScene::OnResConfirm, this));

	resConn = resList->GetValueChangedSignal().connect(
		std::bind(&DisplaySelectScene::UpdateConfirmButtons, this));

	UpdateResGrid();
}

int DisplaySelectScene::GetMonitorIdx() const
{
	return *(monitorList->GetValue());
}

const DisplaySelectScene::Resolution &DisplaySelectScene::GetResolution() const
{
	return *(resList->GetValue());
}

/**
 * Apply the selected filter to the resolution list.
 */
void DisplaySelectScene::FilterResGrid()
{
	if (auto bucket = bucketList->GetValue()) {
		(*bucket)->FilterResList(resList.get());
	}
}

void DisplaySelectScene::UpdateResGrid()
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	resList->Clear();

	int selMonitor = *(monitorList->GetValue());
	int numRes = SDL_GetNumDisplayModes(selMonitor);
	if (numRes == 0) {
		throw Exception(boost::str(boost::format(
			"No resolutions for monitor %d") % selMonitor));
	}
	else if (numRes < 0) {
		throw Exception(boost::str(boost::format(
			"Unable to get resolutions for monitor %d: %s") % selMonitor %
			SDL_GetError()));
	}

	resList->Reserve(numRes);

	for (int i = 0; i < numRes; i++) {
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(selMonitor, i, &mode) < 0) {
			HR_LOG(warning) << "Failed to retrieve resolution info "
				"for monitor " << selMonitor << " and mode " << i;
			continue;
		}

		if (mode.w < MIN_RES_W || mode.h < MIN_RES_H) {
			HR_LOG(debug) << "Ignoring too-small resolution: " <<
				mode.w << "x" << mode.h;
			continue;
		}

		std::ostringstream oss;
		oss.imbue(OS::stdLocale);
		oss << mode.w << 'x' << mode.h;
		if (mode.refresh_rate > 0) {
			oss << " (" << mode.refresh_rate << " Hz)";
		}

		resList->Add(oss.str(), { mode.w, mode.h, mode.refresh_rate });
	}

	FilterResGrid();

	// Find the closest match to the previous resolution.
	SDL_DisplayMode reqMode;
	reqMode.format = 0;
	reqMode.w = reqRes.xRes;
	reqMode.h = reqRes.yRes;
	reqMode.refresh_rate = reqRes.refreshRate;
	reqMode.driverdata = nullptr;
	SDL_DisplayMode closest;
	if (SDL_GetClosestDisplayMode(selMonitor, &reqMode, &closest) != nullptr) {
		resList->SetValue({ closest.w, closest.h, closest.refresh_rate });
	}
}

void DisplaySelectScene::UpdateConfirmButtons()
{
	bool resSelected = resList->HasSelected();
	testBtn->SetEnabled(resSelected);
	confirmBtn->SetEnabled(resSelected);
}

void DisplaySelectScene::OnResTest()
{
	director.RequestPushScene(
		std::make_shared<MessageScene>(display, director,
			"Test Resolution",
			"This function is not implemented yet. :)"));
}

void DisplaySelectScene::OnResConfirm()
{
	confirmSignal();
	director.RequestPopScene();
}

}  // namespace Client
}  // namespace HoverRace
