
// ProfileEditScene.cpp
//
// Copyright (c) 2016 Michael Imamura.
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

#include "../../engine/Display/Container.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Player/AvatarGallery.h"
#include "../../engine/Player/Profile.h"
#include "../../engine/Player/ProfileExn.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"
#include "AvatarSelectScene.h"

#include "ProfileEditScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

class FormProfile : public Player::Profile
{
	using SUPER = Player::Profile;

public:
	FormProfile(Player::Profile &profile) :
		SUPER({}, profile.GetUid(), profile.GetName(), profile.GetAvatarName(),
			profile.GetPrimaryColor(), profile.GetSecondaryColor()),
		profile(profile)
	{
	}
	virtual ~FormProfile() { }

public:
	Player::EditableProfile *Edit() override { return this; }

protected:
	void Save() override
	{
		if (auto editor = profile.Edit()) {
			//editor->SetName(GetName());
			editor->SetAvatarName(GetAvatarName());
			editor->SetPrimaryColor(GetPrimaryColor());
			editor->SetSecondaryColor(GetSecondaryColor());
			editor->Save();
		}
		else {
			//TODO: Use a different exception for error messaging.
			throw Player::ProfileExn("Profile cannot be edited.");
		}
	}

private:
	Player::Profile &profile;
};

}  // namespace

ProfileEditScene::ProfileEditScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle,
	std::shared_ptr<Player::Profile> origProfile) :
	SUPER(display, director, JoinTitles(parentTitle, _("Profile")), "Profile"),
	profile(new FormProfile(*origProfile)), editProfile(profile->Edit()),
	origProfile(std::move(origProfile))
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	auto &s = display.styles;

	SupportCancelAction();

	auto root = GetContentRoot();

	avatarBtn = root->NewChild<Button>(display, Vec2{280, 280}, "");
	avatarBtn->SetTexture(this->origProfile->GetAvatar());
	avatarBtn->SetPos(240, 60);
	avatarClickedConn = avatarBtn->GetClickedSignal().connect(
		std::bind(&ProfileEditScene::OnAvatarSelect, this));

	auto mainGrid = root->NewChild<FlexGrid>(display);
	mainGrid->SetPos(580, 60);

	size_t row = 0;
	mainGrid->At(row++, 0).NewChild<Label>(this->origProfile->GetName(),
		s.headingFont, s.headingFg);

	auto saveCell = mainGrid->At(row++, 0).
		NewChild<Button>(display, _("Save Settings"));
	saveCell->SetAlignment(Alignment::N);
	saveConn = saveCell->GetContents()->GetClickedSignal().connect(
		std::bind(&ProfileEditScene::OnOk, this));

	/*
	AddSetting(_("Primary Color")).
		NewChild<FillBox>(20, 20, this->origProfile->GetPrimaryColor());
	AddSetting(_("Secondary Color")).
		NewChild<FillBox>(20, 20, this->origProfile->GetSecondaryColor());
	*/

	avatarBtn->RequestFocus();
}

void ProfileEditScene::OnAvatarSelect()
{
	auto avatarScene = std::make_shared<AvatarSelectScene>(display, director,
		GetTitle(), director.ShareAvatarGallery(), profile->GetAvatarName());
	avatarSelConn = avatarScene->GetConfirmSignal().connect([=](const std::string &name) {
		auto tex = director.ShareAvatarGallery()->FindName(name);

		avatarBtn->SetTexture(tex);
		editProfile->SetAvatarName(name);
	});
	director.RequestPushScene(avatarScene);
}

void ProfileEditScene::OnOk()
{
	//TODO: Save in background w/ loading scene.
	try {
		editProfile->Save();
		SUPER::OnOk();
	}
	catch (Player::ProfileExn &ex) {
		//TODO: Show error in message scene.
		HR_LOG(error) << ex.what();
	}
}

void ProfileEditScene::OnCancel()
{
	SUPER::OnCancel();
}

}  // namespace Client
}  // namespace HoverRace
