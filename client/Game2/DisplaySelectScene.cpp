
// DisplaySelectScene.cpp
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

#include <SDL2/SDL.h>

#include "../../engine/Display/Button.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"

#include "DisplaySelectScene.h"

using namespace HoverRace::Util;

namespace {
const int MIN_RES_W = 1024;
const int MIN_RES_H = 576;
}  // namespace

namespace HoverRace {
namespace Client {

namespace {

/**
 * A column in the resolution selection grid.
 *
 * This base class represents the "Other" column for non-standard aspect ratios.
 *
 * @author Michael Imamura
 */
class ResBucket
{
protected:
	using Resolution = DisplaySelectScene::Resolution;

public:
	ResBucket(Display::Display &display, Display::FlexGrid &grid,
		Display::RadioGroup<Resolution> &resGroup, size_t col) :
		display(display), grid(grid), resGroup(resGroup), col(col), row(0) { }
	ResBucket(const ResBucket&) = delete;
	virtual ~ResBucket() { }

protected:
	/**
	 * Determine if a monitor resolution belongs to this column.
	 * @param res The resolution.
	 * @return @c true if the resolution matches, @c false otherwise.
	 */
	virtual bool Match(const Resolution &res)
	{
		HR_UNUSED(res);
		return true;
	}

	/**
	 * Generate the column heading.
	 * @return The formatted title.
	 */
	virtual std::string FormatTitle() const { return _("Other"); }

public:
	/**
	 * Try to add a resolution to the column.
	 * @param res The resolution.
	 * @return @c true if the resolution was added,
	 *         @c false if it was ignored (belongs to another column).
	 */
	bool Add(const Resolution &res)
	{
		using namespace Display;
		const auto &s = display.styles;

		if (!Match(res)) return false;

		// Add heading if necessary.
		if (row == 0) {
			grid.AddGridCell(0, col,
				new Label(FormatTitle(), s.bodyHeadFont, s.bodyHeadFg));
			row = 1;
		}

		std::ostringstream oss;
		oss.imbue(OS::stdLocale);
		oss << res.xRes << 'x' << res.yRes;
		if (res.refreshRate > 0) {
			oss << " (" << res.refreshRate << " Hz)";
		}

		resGroup.Add(
			grid.AddGridCell(row++, col,
				new RadioButton<Resolution>(display, oss.str(), res))->
					GetContents());

		return true;
	}

private:
	Display::Display &display;
	Display::FlexGrid &grid;
	Display::RadioGroup<Resolution> &resGroup;
	size_t col;
	size_t row;
};

/**
 * A column for a specific aspect ratio.
 * @author Michael Imamura
 */
class AspectBucket : public ResBucket
{
	using SUPER = ResBucket;

public:
	AspectBucket(Display::Display &display, Display::FlexGrid &grid,
		Display::RadioGroup<Resolution> &resGroup, size_t col, int rx, int ry) :
		SUPER(display, grid, resGroup, col), rx(rx), ry(ry) { }
	AspectBucket(const AspectBucket&) = delete;
	virtual ~AspectBucket() { }

public:
	bool Match(const Resolution &res) override
	{
		return
			(res.xRes % rx) == 0 &&
			(res.yRes % ry) == 0 &&
			(res.xRes / rx) == (res.yRes / ry);
	}

	std::string FormatTitle() const override
	{
		std::ostringstream oss;
		oss.imbue(OS::stdLocale);
		oss << rx << ':' << ry;
		return oss.str();
	}

private:
	int rx;
	int ry;
};

}  // namespace

DisplaySelectScene::DisplaySelectScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle,
	int monitorIdx, int xRes, int yRes) :
	SUPER(display, director, JoinTitles(parentTitle, _("Select Resolution")),
		"Display Select"),
	monitorGroup()
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	auto root = GetContentRoot();

	auto monGrid = root->AddChild(new FlexGrid(display));
	monGrid->SetPos(640, 0);
	monGrid->SetAlignment(Alignment::N);

	int numMonitors = SDL_GetNumVideoDisplays();
	if (numMonitors == 0) {
		throw Exception("No monitors detected.");
	}
	else if (numMonitors < 0) {
		throw Exception(std::string("No monitors detected: ") + SDL_GetError());
	}

	for (int i = 0; i < numMonitors; i++) {
		auto name = boost::str(boost::format(_("Monitor %d")) % (i + 1));

		auto cell = monGrid->AddGridCell(0, static_cast<size_t>(i),
			new RadioButton<int>(display, name, i));
		auto radio = cell->GetContents();
		if (numMonitors == 1) {
			radio->SetEnabled(false);
		}
		monitorGroup.Add(radio);
	}
	monitorGroup.SetValue(monitorIdx);

	if (numMonitors > 1) {
		monitorConn = monitorGroup.GetValueChangedSignal().connect(std::bind(
			&DisplaySelectScene::UpdateResGrid, this));
	}
	UpdateResGrid();

	resGroup->SetValue({ xRes, yRes, 0 });
}

int DisplaySelectScene::GetMonitorIdx() const
{
	return monitorGroup.GetValue();
}

const DisplaySelectScene::Resolution &DisplaySelectScene::GetResolution() const
{
	return resGroup->GetValue();
}

void DisplaySelectScene::UpdateResGrid()
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	auto root = GetContentRoot();

	if (resGrid) {
		root->RemoveChild(resGrid);
	}
	resGrid = root->AddChild(new FlexGrid(display));
	resGrid->SetPos(640, 60);
	resGrid->SetAlignment(Alignment::N);

	resGroup.reset(new RadioGroup<Resolution>());

	int selMonitor = monitorGroup.GetValue();
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

	std::vector<std::unique_ptr<ResBucket>> buckets;
	size_t col = 0;
	buckets.emplace_back(
		new AspectBucket(display, *resGrid, *resGroup, col++, 4, 3));
	buckets.emplace_back(
		new AspectBucket(display, *resGrid, *resGroup, col++, 16, 9));
	buckets.emplace_back(
		new AspectBucket(display, *resGrid, *resGroup, col++, 16, 10));
	buckets.emplace_back(
		new ResBucket(display, *resGrid, *resGroup, col));

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

		for (auto &bucket : buckets) {
			if (bucket->Add({ mode.w, mode.h, mode.refresh_rate })) break;
		}
	}
}

void DisplaySelectScene::OnOk()
{
	okSignal();
	SUPER::OnOk();
}

}  // namespace Client
}  // namespace HoverRace
