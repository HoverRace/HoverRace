
// SettingsMenuScene.cpp
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

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/FlexGrid.h"

#include "AudioSettingsScene.h"
#include "GameDirector.h"
#include "VideoSettingsScene.h"

#include "SettingsMenuScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

template<class T>
class MenuItemButton : public Display::Button
{
	using SUPER = Display::Button;

public:
	MenuItemButton(Display::Display &display, GameDirector &director,
		const std::string &parentTitle, const std::string &label,
		bool enabled = true) :
		SUPER(display, label)
	{
		SetEnabled(enabled);

		clickedConn = GetClickedSignal().connect([&](Display::ClickRegion&) {
			director.RequestPushScene(
				std::make_shared<T>(display, director, parentTitle));
		});
	}
	virtual ~MenuItemButton() { }

private:
	boost::signals2::scoped_connection clickedConn;
};

}  // namespace

SettingsMenuScene::SettingsMenuScene(Display::Display &display,
	GameDirector &director) :
	SUPER(display, director, _("Settings"), "Settings")
{
	using Alignment = Display::UiViewModel::Alignment;

	SupportCancelAction(_("Back"));

	auto root = GetContentRoot();

	menuGrid = root->NewChild<Display::FlexGrid>(display);
	menuGrid->SetPos(640, 140);
	menuGrid->SetAlignment(Alignment::N);

	menuGrid->GetColumnDefault(0).SetFill(true);

	size_t row = 0;
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		display, director, GetTitle(), _("Profile"), false);
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		display, director, GetTitle(), _("Audio"));
	menuGrid->At(row++, 0).NewChild<MenuItemButton<VideoSettingsScene>>(
		display, director, GetTitle(), _("Video"));
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		display, director, GetTitle(), _("Network"), false);
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		display, director, GetTitle(), _("Advanced"), false);

	menuGrid->RequestFocus();
}

SettingsMenuScene::~SettingsMenuScene()
{
}

}  // namespace Client
}  // namespace HoverRace
