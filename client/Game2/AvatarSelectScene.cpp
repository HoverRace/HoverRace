
// AvatarSelectScene.cpp
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

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Picture.h"
#include "../../engine/Player/AvatarGallery.h"
#include "../../engine/Util/Locale.h"
#include "../../engine/Util/Log.h"

#include "AvatarSelectScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {

class AvatarButton : public Display::Button
{
	using SUPER = Display::Button;

public:
	AvatarButton(Display::Display &display,
		std::shared_ptr<Display::Res<Display::Texture>> tex,
		std::shared_ptr<Display::Picture> previewPic) :
		SUPER(display, Vec2{ 80, 80 }, ""),
		previewPic(std::move(previewPic))
	{
		SetTexture(tex);
	}

protected:
	void FireModelUpdate(int prop) override
	{
		if (prop == UiViewModel::Props::FOCUSED && IsFocused()) {
			previewPic->SetTexture(ShareTexture());
		}

		SUPER::FireModelUpdate(prop);
	}

private:
	std::shared_ptr<Display::Picture> previewPic;
};

}  // namespace


/**
 * Constructor.
 * @param display The target display.
 * @param director The current director.
 * @param parentTitle The title of the parent dialog (may be blank).
 * @param gallery The avatar gallery from which to pick an avatar.
 * @param avatarName The initially-selected avatar.
 */
AvatarSelectScene::AvatarSelectScene(Display::Display &display,
	GameDirector &director, const std::string &parentTitle,
	std::shared_ptr<Player::AvatarGallery> gallery,
	const std::string &avatarName) :
	SUPER(display, director, parentTitle, _("SELECT AVATAR"),
		"Avatar Select"),
	gallery(std::move(gallery)), avatarName(avatarName)
{
	using namespace Display;

	SupportCancelAction();

	auto root = GetContentRoot();

	previewPic = root->NewChild<Picture>(
		this->gallery->FindName(avatarName), 280, 280);
	previewPic->SetPos(240, 60);

	auto selGrid = root->NewChild<FlexGrid>(display);
	selGrid->SetPos(580, 60);
	selGrid->SetMargin(20, 20);

	bool found = false;
	size_t col = 0;
	size_t row = 0;
	for (auto &ent : *(this->gallery)) {
		const auto name = ent.first;
		const auto &tex = ent.second;

		auto selBtn = selGrid->At(row, col).NewChild<AvatarButton>(
			display, tex, previewPic)->GetContents();
		conns.emplace_back(selBtn->GetClickedSignal().connect([=](ClickRegion&) {
			OnAvatarSelected(name);
		}));

		if (name == avatarName) {
			selBtn->RequestFocus();
			found = true;
		}

		col++;
		if (col >= 5) {
			col = 0;
			row++;
		}
	}

	if (!found) {
		selGrid->RequestFocus();
	}
}

void AvatarSelectScene::OnAvatarSelected(const std::string &name)
{
	avatarName = name;
	previewPic->SetTexture(gallery->FindName(name));

	confirmSignal(name);
	director.RequestPopScene();
}

}  // namespace Client
}  // namespace HoverRace
