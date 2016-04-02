
// AvatarSelectScene.h
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

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
		class Picture;
	}
	namespace Player {
		class AvatarGallery;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Select an avatar from the avatar gallery.
 * @author Michael Imamura
 */
class AvatarSelectScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	AvatarSelectScene(Display::Display &display, GameDirector &director,
		std::shared_ptr<Player::AvatarGallery> gallery,
		const std::string &avatarName = {});
	virtual ~AvatarSelectScene() { }

public:
	const std::string &GetAvatarName() const { return avatarName; }

public:
	void OnAvatarSelected(const std::string &name);

public:
	using confirmSignal_t = boost::signals2::signal<void(const std::string&)>;
	confirmSignal_t &GetConfirmSignal() { return confirmSignal; }

private:
	std::shared_ptr<Player::AvatarGallery> gallery;
	std::string avatarName;

	std::shared_ptr<Display::Picture> previewPic;

	std::list<boost::signals2::scoped_connection> conns;

	confirmSignal_t confirmSignal;
};

}  // namespace Client
}  // namespace HoverRace
