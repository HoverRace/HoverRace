
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

/**
 * Constructor.
 * @param display The target display.
 * @param director The current director.
 * @param gallery The avatar gallery from which to pick an avatar.
 * @param avatarName The initially-selected avatar.
 */
AvatarSelectScene::AvatarSelectScene(Display::Display &display,
	GameDirector &director, std::shared_ptr<Player::AvatarGallery> gallery,
	const std::string &avatarName) :
	SUPER(display, director, _("Select Avatar"), "Avatar Select"),
	gallery(std::move(gallery)), avatarName(avatarName)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SupportCancelAction();

	auto root = GetContentRoot();

	previewPic = root->NewChild<Picture>(
		this->gallery->FindName(avatarName), 280, 280);
	previewPic->SetPos(240, 60);

	auto selGrid = root->NewChild<FlexGrid>(display);
	selGrid->SetPos(580, 60);
	selGrid->SetMargin(20, 20);

	size_t col = 0;
	size_t row = 0;
	for (auto &ent : *(this->gallery)) {
		const auto name = ent.first;
		const auto &tex = ent.second;

		auto selBtn = selGrid->At(row, col).NewChild<Button>(display,
			Vec2{80, 80}, "")->GetContents();
		selBtn->SetTexture(tex);
		conns.emplace_back(selBtn->GetClickedSignal().connect([=](ClickRegion&) {
			OnAvatarSelected(name);
		}));

		col++;
		if (col >= 5) {
			col = 0;
			row++;
		}
	}
}

void AvatarSelectScene::OnAvatarSelected(const std::string &name)
{
	avatarName = name;
	previewPic->SetTexture(gallery->FindName(avatarName));

	/*
	confirmSignal();
	director.RequestPopScene();
	*/
}

}  // namespace Client
}  // namespace HoverRace
