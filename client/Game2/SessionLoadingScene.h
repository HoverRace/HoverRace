
// SessionLoadingScene.h
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "../../engine/Display/Res.h"

#include "LoadingScene.h"

namespace HoverRace {
	namespace Display {
		class Picture;
		class Texture;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Something pleasant to look at while the track loads, the server starts,
 * and all players connect.
 * @author Michael Imamura
 */
class SessionLoadingScene : public LoadingScene
{
	using SUPER = LoadingScene;

public:
	SessionLoadingScene(Display::Display &display, GameDirector &director,
		std::shared_ptr<Display::Res<Display::Texture>> mapRes);
	virtual ~SessionLoadingScene();

public:
	void OnPhaseTransition(double progress) override;

private:
	std::shared_ptr<Display::Picture> mapPic;
};

}  // namespace Client
}  // namespace HoverRace
