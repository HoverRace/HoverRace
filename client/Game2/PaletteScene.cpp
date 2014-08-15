
// PaletteScene.cpp
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

#include "../StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/Label.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/Util/Config.h"

#include "PaletteScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

PaletteScene::PaletteScene(GameDirector &director) :
	SUPER("Palette Viewer"),
	director(director), display(*director.GetDisplay())
{
}

PaletteScene::~PaletteScene()
{
}

void PaletteScene::OnOk()
{
	director.RequestPopScene();
}

void PaletteScene::AttachController(Control::InputEventController &controller)
{
	controller.AddMenuMaps();

	// Both "OK" and "Cancel" actions close the scene.
	okConn = controller.actions.ui.menuOk->Connect(std::bind(&PaletteScene::OnOk, this));
	cancelConn = controller.actions.ui.menuCancel->Connect(std::bind(&PaletteScene::OnOk, this));
}

void PaletteScene::DetachController(Control::InputEventController&)
{
	cancelConn.disconnect();
	okConn.disconnect();
}

void PaletteScene::Render()
{
	VideoServices::VideoBuffer &legacyDisplay = display.GetLegacyDisplay();
	VideoServices::VideoBuffer::Lock lock(legacyDisplay);

	// Clear the buffer since we don't write to all of it.
	legacyDisplay.Clear(0);

	int pitch = legacyDisplay.GetPitch();
	MR_UInt8 *buf = legacyDisplay.GetBuffer();

	// Reserve some space for the label.
	buf += 16 * pitch;

	for (int y = 0; y < 256; y++, buf += pitch) {
		if ((y % 16) == 0) continue;
		MR_UInt8 *cur = buf;
		for (int x = 0; x < 256; x++, cur++) {
			if ((x % 16) == 0) continue;
			*cur = ((y >> 4) << 4) + (x >> 4);
		}
	}
}

}  // namespace HoverScript
}  // namespace Client
