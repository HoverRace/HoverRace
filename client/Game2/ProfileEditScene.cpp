
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
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Random.h"
#include "AvatarSelectScene.h"
#include "TextEditScene.h"

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
	Player::RenamableProfile *EditName() override { return this; }

protected:
	void Save() override
	{
		if (auto editor = profile.EditName()) {
			editor->Rename(GetName());
		}

		if (auto editor = profile.Edit()) {
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

class ColorButton : public Display::Button
{
	using SUPER = Display::Button;

	static const Vec2 SIZE;

public:
	ColorButton(Display::Display &display) :
		SUPER(display, SIZE, ""),
		colorContainer(std::make_shared<Display::Container>(display))
	{
		colorContainer->AttachView(display);
		colorBox = colorContainer->NewChild<Display::FillBox>(SIZE, 0xffffffff);
		SetContents(colorContainer);
	}

public:
	void SetColor(Display::Color color)
	{
		colorBox->SetColor(color);
	}

private:
	std::shared_ptr<Display::Container> colorContainer;
	std::shared_ptr<Display::FillBox> colorBox;
};

const Vec2 ColorButton::SIZE{ 200, 40 };

class ColorGrid : public Display::FlexGrid
{
	using SUPER = Display::FlexGrid;

public:
	ColorGrid(Display::Display &display, Player::Profile &profile,
		Player::EditableProfile &editProfile) :
		SUPER(display), profile(profile), editProfile(editProfile), col(0) { }
	virtual ~ColorGrid() { }

public:
	std::shared_ptr<ColorButton> AddColorButton(size_t i)
	{
		using namespace Display;

		assert(i < 2);

		auto btn = At(0, col++).NewChild<ColorButton>(display)->GetContents();
		btn->SetColor(profile.GetColor(i));
		auto btnPtr = btn.get();
		conns[i] = btn->GetClickedSignal().connect([=](ClickRegion&) {
			Color newColor(RandomInt<MR_UInt32>(0, 0xffffff)());
			newColor.bits.a = 0xff;

			btnPtr->SetColor(newColor);
			editProfile.SetColor(i, newColor);
		});

		return btn;
	}

private:
	Player::Profile &profile;
	Player::EditableProfile &editProfile;
	size_t col;
	std::array<boost::signals2::scoped_connection, 2> conns;
};

}  // namespace

ProfileEditScene::ProfileEditScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle,
	std::shared_ptr<Player::Profile> origProfile) :
	SUPER(display, director, parentTitle, _("PROFILE"), "Profile"),
	profile(new FormProfile(*origProfile)),
	editProfile(profile->Edit()), renameProfile(profile->EditName()),
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
	nameLbl = mainGrid->At(row++, 0).NewChild<Label>(
		this->origProfile->GetName(),
		s.headingFont, s.headingFg)->GetContents();

	auto renameBtn = mainGrid->At(row++, 0).
		NewChild<Button>(display, _("Change Name"))->GetContents();
	renameConn = renameBtn->GetClickedSignal().connect(
		std::bind(&ProfileEditScene::OnRename, this));
	renameBtn->SetEnabled(editProfile != nullptr);

	auto colorGrid = mainGrid->At(row++, 0).
		NewChild<ColorGrid>(display, *(this->origProfile), *editProfile)->
			GetContents();
	primaryColorBtn = colorGrid->AddColorButton(0);
	secondaryColorBtn = colorGrid->AddColorButton(1);

	auto saveCell = mainGrid->At(row++, 0).
		NewChild<Button>(display, _("Save Settings"));
	saveCell->SetAlignment(Alignment::N);
	saveConn = saveCell->GetContents()->GetClickedSignal().connect(
		std::bind(&ProfileEditScene::OnOk, this));

	avatarBtn->RequestFocus();
}

void ProfileEditScene::OnRename()
{
	auto scene = std::make_shared<TextEditScene>(display, director,
		GetFullTitle(), _("EDIT NAME"), Player::Profile::MAX_NAME_LENGTH,
		profile->GetName());
	scene->SetHint(_("Select a new name. "
		"Changes will take effect immediately."));

	scene->GetConfirmSignal().connect([=](const std::string &s) {
		nameLbl->SetText(s);
		renameProfile->Rename(s);
	});
	director.RequestPushScene(scene);
}

void ProfileEditScene::OnAvatarSelect()
{
	auto avatarScene = std::make_shared<AvatarSelectScene>(display, director,
		GetFullTitle(), director.ShareAvatarGallery(),
		profile->GetAvatarName());
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
