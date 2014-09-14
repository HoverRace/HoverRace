
// PaletteScene.h
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

#include "../../engine/Util/Config.h"

#include "GameDirector.h"

#include "UiScene.h"

namespace HoverRace {
	namespace Display {
		class Display;
		class Label;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Draws a color grid to debug the color palette of the legacy display.
 * @author Michael Imamura
 */
class PaletteScene : public UiScene
{
	typedef UiScene SUPER;
	public:
		PaletteScene(GameDirector &director);
		virtual ~PaletteScene();

	public:
		bool IsMouseCursorEnabled() const override { return false; }

	protected:
		virtual void OnOk();

	public:
		void AttachController(Control::InputEventController &controller) override;
		void DetachController(Control::InputEventController &controller) override;
		void PrepareRender() override { }
		void Render() override;

	private:
		GameDirector &director;
		Display::Display &display;

		boost::signals2::connection okConn;
		boost::signals2::connection cancelConn;
};

}  // namespace HoverScript
}  // namespace Client
