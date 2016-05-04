
// SettingsMenuScene.cpp
//
// Copyright (c) 2014-2016 Michael Imamura.
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
#include "../../engine/Player/Player.h"

#include "AudioSettingsScene.h"
#include "GameDirector.h"
#include "LocaleSettingsScene.h"
#include "ProfileEditScene.h"
#include "VideoSettingsScene.h"

#include "SettingsMenuScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

template<class T>
void LaunchScene(Display::Display &display, GameDirector &director,
	const std::string &parentTitle)
{
	director.RequestPushScene(
		std::make_shared<T>(display, director, parentTitle));
}

// Specialization for profile editor, which takes a profile.
template<>
void LaunchScene<ProfileEditScene>(Display::Display &display,
	GameDirector &director, const std::string &parentTitle)
{
	director.RequestPushScene(
		std::make_shared<ProfileEditScene>(display, director, parentTitle,
			director.ShareUiPilot()->ShareProfile()));
}

}  // namespace

template<class T>
class SettingsMenuScene::MenuItemButton : public Display::Button
{
	using SUPER = Display::Button;

public:
	MenuItemButton(SettingsMenuScene &scene, const std::string &label,
		bool enabled = true) :
		SUPER(scene.display, label)
	{
		SetEnabled(enabled);

		clickedConn = GetClickedSignal().connect([&](Display::ClickRegion&) {
			LaunchScene<T>(scene.display, scene.director, scene.GetFullTitle());
		});
	}
	virtual ~MenuItemButton() { }

private:
	boost::signals2::scoped_connection clickedConn;
};

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

	// If there's no active player piloting the UI, then only enable
	// settings which are global to everybody.
	bool hasProfile = false;
	if (auto pilot = director.ShareUiPilot()) {
		hasProfile = !!pilot->ShareProfile();
	}

	size_t row = 0;
	menuGrid->At(row++, 0).NewChild<MenuItemButton<ProfileEditScene>>(
		*this, _("Profile"), hasProfile);
	menuGrid->At(row++, 0).NewChild<MenuItemButton<LocaleSettingsScene>>(
		*this, _("Language and Units"));
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		*this, _("Audio"));
	menuGrid->At(row++, 0).NewChild<MenuItemButton<VideoSettingsScene>>(
		*this, _("Video"));
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		*this, _("Network"), false);
	menuGrid->At(row++, 0).NewChild<MenuItemButton<AudioSettingsScene>>(
		*this, _("Advanced"), false);

	menuGrid->RequestFocus();
}

}  // namespace Client
}  // namespace HoverRace
