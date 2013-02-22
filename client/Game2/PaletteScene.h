
// PaletteScene.h
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

#include "Scene.h"

namespace HoverRace {
	namespace Display {
		class Display;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Draws a color grid to debug the color palette of the legacy display.
 * @author Michael Imamura
 */
class PaletteScene : public Scene
{
	typedef Scene SUPER;
	public:
		PaletteScene(Display::Display &display);
		virtual ~PaletteScene();

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void Render();

	private:
		Display::Display &display;
};

}  // namespace HoverScript
}  // namespace Client
