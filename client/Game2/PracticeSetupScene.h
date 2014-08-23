
// PracticeSetupScene.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "GameDirector.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class RulebookLibrary;
	}
	namespace Display {
		class Display;
		class Texture;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Launcher for practice (single-player) session.
 * @author Michael Imamura
 */
class PracticeSetupScene : public DialogScene
{
	typedef DialogScene SUPER;
	public:
		PracticeSetupScene(Display::Display &display, GameDirector &director,
			RulebookLibrary &rulebookLibrary);
		virtual ~PracticeSetupScene();

	public:
		void OnScenePushed() override;

	private:
		Display::Display &display;
		GameDirector &director;
		RulebookLibrary &rulebookLibrary;
		std::shared_ptr<Display::Res<Display::Texture>> bgTex;
};

}  // namespace Client
}  // namespace HoverRace
