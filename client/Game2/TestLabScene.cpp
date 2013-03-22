
// TestLabScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/Util/Config.h"

#include "TestLabScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

TestLabScene::TestLabScene(Display::Display &display) :
	SUPER("Test Lab"),
	display(display)
{
	typedef Display::UiViewModel::Alignment Alignment;

	Config *cfg = Config::GetInstance();
	std::string fontName = cfg->GetDefaultFontName();

	displayConfigChangedConn = display.GetDisplayConfigChangedSignal().
		connect(std::bind(&TestLabScene::OnDisplayConfigChanged, this));

	// Clear the screen on every frame.
	AddElem(new Display::ScreenFade(Display::COLOR_BLACK, 1.0));

	Display::FillBox *fillBox;
	Display::Label *lbl;

	displayInfoBox = AddElem(new Display::FillBox(1280, 720, 0xff3f3f3f));
	displayInfoLbl = AddElem(new Display::Label("Res",
		Display::UiFont(fontName, 20),
		Display::COLOR_WHITE));
	displayInfoLbl->SetPos(640, 360);
	displayInfoLbl->SetAlignment(Alignment::CENTER);
	OnDisplayConfigChanged();

	fillBox = AddElem(new Display::FillBox(100, 100, 0x7fff0000));
	fillBox->SetPos(100, 20);
	fillBox = AddElem(new Display::FillBox(100, 100, 0x7f00ff00));
	fillBox->SetPos(150, 70);

	lbl = AddElem(new Display::Label("Red 20 Normal",
		Display::UiFont(fontName, 20), 0xffff0000));
	lbl->SetPos(0, 20);
	lbl = AddElem(new Display::Label("Yellow (75%) 25 Italic",
		Display::UiFont(fontName, 25, Display::UiFont::ITALIC), 0xbfffff00));
	lbl->SetPos(0, 40);
	lbl = AddElem(new Display::Label("Magenta (50%) 30 Bold+Italic",
		Display::UiFont(fontName, 30, Display::UiFont::BOLD | Display::UiFont::ITALIC),
		0x7fff00ff));
	lbl->SetPos(0, 65);
}

TestLabScene::~TestLabScene()
{
}

void TestLabScene::OnDisplayConfigChanged()
{
	double uiScale = display.GetUiScale();
	const Display::Vec2 &uiOffset = display.GetUiOffset();

	static boost::format resFmt("UI Scale: %0.2f  Offset: %d,%d");
	displayInfoLbl->SetText(boost::str(resFmt % uiScale % uiOffset.x % uiOffset.y));
}

void TestLabScene::OnPhaseChanged(Phase::phase_t oldPhase)
{
	// Act like the starting and stopping phases don't even exist.
	switch (GetPhase()) {
		case Phase::STARTING:
			SetPhase(Phase::RUNNING);
			break;
		case Phase::STOPPING:
			SetPhase(Phase::STOPPED);
			break;
	}
}

void TestLabScene::Advance(Util::OS::timestamp_t)
{
}

void TestLabScene::PrepareRender()
{
	std::for_each(elems.begin(), elems.end(),
		std::mem_fn(&Display::ViewModel::PrepareRender));
}

void TestLabScene::Render()
{
	std::for_each(elems.begin(), elems.end(),
		std::mem_fn(&Display::ViewModel::Render));
}

}  // namespace HoverScript
}  // namespace Client
