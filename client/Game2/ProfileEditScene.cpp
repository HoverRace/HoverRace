
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

#include "../../engine/Player/Profile.h"
#include "../../engine/Player/ProfileExn.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

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
		SUPER(profile.GetUid(), profile.GetName(), profile.GetAvatarName(),
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
	SUPER(display, director, parentTitle, _("Profile"), "Edit Profile"),
	profile((new FormProfile(*origProfile))->Edit()),
	origProfile(std::move(origProfile))
{
	using namespace Display;

	//TODO: Init widgets.

	GetSettingsGrid()->RequestFocus();
}

void ProfileEditScene::LoadFromConfig()
{
	//TODO: Update widgets.
}

void ProfileEditScene::ResetToDefaults()
{
	//TODO: Remove button.
}

void ProfileEditScene::OnOk()
{
	//TODO: Save in background w/ loading scene.
	try {
		profile->Save();
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
