
// Display.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Util/Config.h"
#include "TypeCase.h"

#include "Display.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const double MIN_VIRT_WIDTH = 1280.0;
	const double MIN_VIRT_HEIGHT = 720.0;
}

namespace {

const std::string TYPE_CASE_INIT =
	" !\"#$%&'()*+,-./0123456789:;<=>?"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmnopqrstuvwxyz{|}~"
	"\xc2\xa1"  // INVERTED EXCLAMATION MARK
	"\xc2\xa9"  // COPYRIGHT SIGN
	"\xc2\xb0"  // DEGREE SIGN
	"\xc2\xb7"  // MIDDLE DOT
	"\xc2\xbf"  // INVERTED QUESTION MARK
	;

}  // namespace

std::shared_ptr<TypeCase> Display::GetTypeCase(const UiFont &font)
{
	UiFont normalizedFont = font;
	normalizedFont.size = floor(font.size);

	//TODO: Cache instances.

	auto retv = MakeTypeCase(normalizedFont);
	retv->Prepare(TYPE_CASE_INIT);

	return retv;
}

void Display::OnDisplayConfigChanged()
{
	const auto &vidCfg = Config::GetInstance()->video;
	int w = vidCfg.xRes;
	int h = vidCfg.yRes;
	double wd = static_cast<double>(w);
	double hd = static_cast<double>(h);

	// Recalculate the UI coordinates.
	double newUiScale;
	double uiScaleW = wd / MIN_VIRT_WIDTH;
	double uiScaleH = hd / MIN_VIRT_HEIGHT;
	if (uiScaleW < uiScaleH) {
		newUiScale = uiScaleW;
		uiOffset.x = 0;
		uiOffset.y = floor((hd - (MIN_VIRT_HEIGHT * uiScaleW)) / 2);
	}
	else {
		newUiScale = uiScaleH;
		uiOffset.x = floor((wd - (MIN_VIRT_WIDTH * uiScaleH)) / 2);
		uiOffset.y = 0;
	}

	// Save the new UI scale before firing displayConfigChanged.
	// This way, handlers that need to know both the new dimensions and scale
	// can do so, even though we only pass the dimensions.
	bool scaleChanged = uiScale != newUiScale;
	if (scaleChanged) {
		uiScale = newUiScale;
	}

	uiScreenSize.x = 1280.0;
	uiScreenSize.y = 720.0;
	uiScreenSize += (uiOffset * 2.0) / uiScale;

	FireDisplayConfigChangedSignal(w, h);

	if (scaleChanged) {
		FireUiScaleChangedSignal(uiScale);
	}
}

void Display::FireDisplayConfigChangedSignal(int width, int height) const
{
	displayConfigChangedSignal(width, height);
}

void Display::FireUiScaleChangedSignal(double scale) const
{
	uiScaleChangedSignal(scale);
}

}  // namespace Display
}  // namespace HoverRace
