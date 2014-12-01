
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
#include "../../engine/Util/Log.h"
#include "../../engine/Util/OS.h"

#include "DisplaySelectScene.h"

using namespace HoverRace::Util;

namespace {
const int MIN_RES_W = 800;
const int MIN_RES_H = 600;
}

namespace HoverRace {
namespace Client {

DisplaySelectScene::DisplaySelectScene(Display::Display &display,
	GameDirector &director, int monitorIdx, int xRes, int yRes) :
	SUPER(display, director,
		std::string(_("Settings")) + " // " +
			_("Video") + " // " +
			_("Select Resolution"),
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

	resGroup.reset(new RadioGroup<int>());

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

	//TODO: Arrange by aspect ratio.
	size_t row = 0;

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

		auto name = boost::str(boost::format("%dx%d (%d Hz)", OS::stdLocale) %
			mode.w % mode.h % mode.refresh_rate);

		auto cell = resGrid->AddGridCell(row++, 0,
			new RadioButton<int>(display, name, i));
		resGroup->Add(cell->GetContents());
	}
}

}  // namespace Client
}  // namespace HoverRace
