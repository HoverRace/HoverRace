
// MessageScene.h
//
// Copyright (c) 2013 Michael Imamura.
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

#include "../../engine/Util/Config.h"

#include "GameDirector.h"

#include "UiScene.h"

namespace HoverRace {
	namespace Display {
		class Display;
		class Label;
		class ScreenFade;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Displays a message to the user.
 * @author Michael Imamura
 */
class MessageScene : public UiScene
{
	typedef UiScene SUPER;
	public:
		MessageScene(Display::Display &display, const std::string &title,
			const std::string &message);
		virtual ~MessageScene();

	public:
		// Scene
		virtual void OnPhaseChanged(Phase::phase_t oldPhase);
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		Display::Display &display;
		Display::ScreenFade *fader;
		Display::Label *titleLbl;
		Display::Label *messageLbl;
};

}  // namespace HoverScript
}  // namespace Client
