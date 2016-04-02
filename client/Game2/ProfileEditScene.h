
// ProfileEditScene.h
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

#pragma once

#include "../../engine/Player/EditableProfile.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Player {
		class Profile;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Profile view and edit scene.
 * @author Michael Imamura
 */
class ProfileEditScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	ProfileEditScene(Display::Display &display, GameDirector &director,
		const std::string &parentTitle,
		std::shared_ptr<Player::Profile> origProfile);
	virtual ~ProfileEditScene() { }

private:
	void OnAvatarSelect();

protected:
	void OnOk() override;
	void OnCancel() override;

private:
	std::unique_ptr<Player::EditableProfile> profile;
	std::shared_ptr<Player::Profile> origProfile;

	std::shared_ptr<Display::Button> avatarBtn;

	boost::signals2::scoped_connection avatarClickedConn;
	boost::signals2::scoped_connection avatarSelConn;
};

}  // namespace Client
}  // namespace HoverRace
