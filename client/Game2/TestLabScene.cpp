
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

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Log.h"

#include "MessageScene.h"

#include "TestLabScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

class TestLabScene::LabModule : public FormScene /*{{{*/
{
	typedef FormScene SUPER;
	public:
		LabModule(Display::Display &display, GameDirector &director,
			const std::string &title);
		virtual ~LabModule() { }

	public:
		virtual void PrepareRender();
		virtual void Render();

	protected:
		Display::Display &display;
		GameDirector &director;
		const std::string title;
	private:
		std::unique_ptr<Display::ScreenFade> fader;
}; //}}}

namespace {
	template<typename Module>
	class ModuleButton : public Display::Button /*{{{*/
	{
		typedef Display::Button SUPER;
		public:
			ModuleButton(Display::Display &display, GameDirector &director,
				const std::string &text, double x, double y) :
				SUPER(display, text)
			{
				SetPos(x, y);
				GetClickedSignal().connect([&](Display::ClickRegion&) {
					director.RequestPushScene(std::make_shared<Module>(display, director));
				});
			}
	}; //}}}
}

namespace Module {
	class LayoutModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			LayoutModule(Display::Display &display, GameDirector &director);
			virtual ~LayoutModule() { }

		private:
			void AddAlignmentTestElem(
				Display::UiViewModel::Alignment::alignment_t alignment,
				const std::string &label, double x, double y);

		public:
			void OnDisplayConfigChanged();

		private:
			boost::signals2::scoped_connection displayConfigChangedConn;
			std::shared_ptr<Display::FillBox> displayInfoBox;
			std::shared_ptr<Display::Label> displayInfoLbl;
	}; //}}}

	class ButtonModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			ButtonModule(Display::Display &display, GameDirector &director);
			virtual ~ButtonModule() { }

		private:
			void OnMessageClicked();

		private:
			std::shared_ptr<Display::Button> messageBtn;
	}; //}}}

	class LabelModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			LabelModule(Display::Display &display, GameDirector &director);
			virtual ~LabelModule() { }

		private:
			void AdjustWrapWidth(double amt);

		private:
			std::shared_ptr<Display::FillBox> wrapBox;
			std::shared_ptr<Display::Label> wrapLbl;
	}; //}}}
}

TestLabScene::TestLabScene(Display::Display &display, GameDirector &director) :
	SUPER(display, "Test Lab"),
	display(display), director(director)
{
	// Clear the screen on every frame.
	fader.reset(new Display::ScreenFade(Display::COLOR_BLACK, 1.0));
	fader->AttachView(display);

	auto root = GetRoot();

	const double yStep = 60;
	double y = 60;
	root->AddChild(new ModuleButton<Module::LayoutModule>(display, director, "Layout", 0, y));
	y += yStep;
	root->AddChild(new ModuleButton<Module::ButtonModule>(display, director, "Button", 0, y));
	y += yStep;
	root->AddChild(new ModuleButton<Module::LabelModule>(display, director, "Label", 0, y));
}

TestLabScene::~TestLabScene()
{
}

void TestLabScene::PrepareRender()
{
	fader->PrepareRender();
	SUPER::PrepareRender();
}

void TestLabScene::Render()
{
	fader->Render();
	SUPER::Render();
}

//{{{ TestLabScene::LabModule //////////////////////////////////////////////////

TestLabScene::LabModule::LabModule(Display::Display &display,
                                   GameDirector &director,
                                   const std::string &title) :
	SUPER(display, "Lab Module (" + title + ")"),
	display(display), director(director), title(title)
{
	typedef Display::UiViewModel::Alignment Alignment;

	fader.reset(new Display::ScreenFade(Display::COLOR_BLACK, 1.0));
	fader->AttachView(display);

	auto btn = GetRoot()->AddChild(new Display::Button(display, "Close"));
	btn->SetPos(1280, 0);
	btn->SetAlignment(Alignment::NE);
	btn->GetClickedSignal().connect([&](Display::ClickRegion&) {
		director.RequestPopScene();
	});
}

void TestLabScene::LabModule::PrepareRender()
{
	fader->PrepareRender();
	SUPER::PrepareRender();
}

void TestLabScene::LabModule::Render()
{
	fader->Render();
	SUPER::Render();
}

//}}} TestLabScene::LabModule

namespace Module {

//{{{ LayoutModule /////////////////////////////////////////////////////////////

LayoutModule::LayoutModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Layout")
{
	typedef Display::UiViewModel::Alignment Alignment;

	Display::Container *root = GetRoot();

	Config *cfg = Config::GetInstance();
	std::string fontName = cfg->GetDefaultFontName();

	displayConfigChangedConn = display.GetDisplayConfigChangedSignal().
		connect(std::bind(&LayoutModule::OnDisplayConfigChanged, this));

	// This box takes up the entire UI viewport, so we insert it at the
	// beginning of the list so we don't render on top of the "Close" button.
	displayInfoBox = root->InsertChild(0, new Display::FillBox(1280, 720, 0xff3f3f3f));

	displayInfoLbl = root->AddChild(new Display::Label("Res",
		Display::UiFont(fontName, 20),
		Display::COLOR_WHITE));
	displayInfoLbl->SetPos(640, 360);
	displayInfoLbl->SetAlignment(Alignment::CENTER);
	OnDisplayConfigChanged();

	std::shared_ptr<Display::FillBox> fillBox;
	std::shared_ptr<Display::Label> lbl;

	fillBox = root->AddChild(new Display::FillBox(100, 100, 0x7fff0000));
	fillBox->SetPos(100, 20);
	fillBox = root->AddChild(new Display::FillBox(100, 100, 0x7f00ff00));
	fillBox->SetPos(150, 70);

	AddAlignmentTestElem(Alignment::SW, "| Southwest", 0, 719);
	AddAlignmentTestElem(Alignment::S, "South", 639, 719);
	AddAlignmentTestElem(Alignment::SE, "Southeast |", 1279, 719);

	lbl = root->AddChild(new Display::Label("Red 20 Normal",
		Display::UiFont(fontName, 20), 0xffff0000));
	lbl->SetPos(0, 20);
	lbl = root->AddChild(new Display::Label("Yellow (75%) 25 Italic",
		Display::UiFont(fontName, 25, Display::UiFont::ITALIC), 0xbfffff00));
	lbl->SetPos(0, 40);
	lbl = root->AddChild(new Display::Label("Magenta (50%) 30 Bold+Italic",
		Display::UiFont(fontName, 30, Display::UiFont::BOLD | Display::UiFont::ITALIC),
		0x7fff00ff));
	lbl->SetPos(0, 65);
}

void LayoutModule::AddAlignmentTestElem(
	Display::UiViewModel::Alignment::alignment_t alignment,
	const std::string &label, double x, double y)
{
	Config *cfg = Config::GetInstance();
	std::string fontName = cfg->GetDefaultFontName();

	auto fillBox = GetRoot()->AddChild(new Display::FillBox(50, 50, 0x7f00ffff));
	fillBox->SetAlignment(alignment);
	fillBox->SetPos(x, y);

	auto lbl = GetRoot()->AddChild(new Display::Label(label,
		Display::UiFont(fontName, 40), 0xffffffff));
	lbl->SetAlignment(alignment);
	lbl->SetPos(x, y);
}

void LayoutModule::OnDisplayConfigChanged()
{
	double uiScale = display.GetUiScale();
	const Display::Vec2 &uiOffset = display.GetUiOffset();

	static boost::format resFmt("UI Scale: %0.2f  Offset: %d,%d");
	displayInfoLbl->SetText(boost::str(resFmt % uiScale % uiOffset.x % uiOffset.y));
}

//}}} LayoutModule

//{{{ ButtonModule /////////////////////////////////////////////////////////////

ButtonModule::ButtonModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Button")
{
	typedef Display::UiViewModel::Alignment Alignment;

	Display::Container *root = GetRoot();

	messageBtn = root->AddChild(new Display::Button(display, "Show Message"));
	messageBtn->SetPos(640, 0);
	messageBtn->SetAlignment(Alignment::N);
	messageBtn->GetClickedSignal().connect(
		std::bind(&ButtonModule::OnMessageClicked, this));

	auto btn = root->AddChild(new Display::Button(display, "Disabled Button"));
	btn->SetEnabled(false);
	btn->SetPos(640, 60);
	btn->SetAlignment(Alignment::N);
	btn->GetClickedSignal().connect([](Display::ClickRegion&) {
		Log::Error("Clicked on disabled button :(");
	});
}

void ButtonModule::OnMessageClicked()
{
	director.RequestPushScene(std::make_shared<MessageScene>(display, director,
		"Test Lab", "Hello, world!"));
}

//}}} ButtonModule

//{{{ LabelModule //////////////////////////////////////////////////////////////

LabelModule::LabelModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Button")
{
	Config *cfg = Config::GetInstance();
	const std::string &fontName = cfg->GetDefaultFontName();
	const std::string &monospaceFontName = cfg->GetDefaultMonospaceFontName();
	const std::string &symbolFontName = cfg->GetDefaultSymbolFontName();

	Display::Container *root = GetRoot();

	std::shared_ptr<Display::Button> btn;
	std::shared_ptr<Display::Label> lbl;

	lbl = root->AddChild(new Display::Label("Red 20 Normal",
		Display::UiFont(fontName, 20), 0xffff0000));
	lbl->SetPos(0, 20);
	lbl = root->AddChild(new Display::Label("Yellow (75%) 25 Italic",
		Display::UiFont(fontName, 25, Display::UiFont::ITALIC), 0xbfffff00));
	lbl->SetPos(0, 40);
	lbl = root->AddChild(new Display::Label("Magenta (50%) 30 Bold+Italic",
		Display::UiFont(fontName, 30, Display::UiFont::BOLD | Display::UiFont::ITALIC),
		0x7fff00ff));
	lbl->SetPos(0, 65);

	lbl = root->AddChild(new Display::Label("Default Font",
		Display::UiFont(fontName, 30), 0xffbfbfbf));
	lbl->SetPos(640, 20);
	lbl = root->AddChild(new Display::Label("Monospace Font",
		Display::UiFont(monospaceFontName, 30), 0xffbfbfbf));
	lbl->SetPos(640, 50);
	lbl = root->AddChild(new Display::Label("< "
		"\xef\x84\x9b "  // gamepad
		"\xef\x82\x91 "  // trophy
		"\xef\x83\xa4 "  // dashboard
		">",
		Display::UiFont(symbolFontName, 30), 0xffbfbfbf));
	lbl->SetPos(640, 80);

	// Wrapped text (with a background to visualize the width).

	std::string wrapText =
		"This is a string which should be wrapped to multiple lines to fit "
		"the fixed width label.\n\n"
		"ThisIsAVeryLongWordThatShouldBeClippedToTheFixedWidthOfTheLabel.\n\n"
		"This is the third line of the text.";

	wrapBox = root->AddChild(new Display::FillBox(1280 / 4, 720 - 150, 0xff3f3f3f));
	wrapBox->SetPos(100, 150);

	wrapLbl = root->AddChild(new Display::Label(1280 / 4, wrapText,
		Display::UiFont(fontName, 30), 0xffffffff));
	wrapLbl->SetPos(100, 150);

	// Buttons to increase / decrease the wrap width.

	btn = root->AddChild(new Display::Button(display, "->|"));
	btn->SetPos(0, 150);
	btn->GetClickedSignal().connect(std::bind(
		&LabelModule::AdjustWrapWidth, this, 50));

	btn = root->AddChild(new Display::Button(display, "|<-"));
	btn->SetPos(0, 200);
	btn->GetClickedSignal().connect(std::bind(
		&LabelModule::AdjustWrapWidth, this, -50));
}

void LabelModule::AdjustWrapWidth(double amt)
{
	const Display::Vec2 &curSize = wrapBox->GetSize();
	double newWidth = curSize.x + amt;

	if (newWidth > 0 && newWidth < (1280.0 - wrapBox->GetPos().x)) {
		wrapBox->SetSize(newWidth, curSize.y);
		wrapLbl->SetWrapWidth(newWidth);
	}
}

//}}} LabelModule

}  // namespace Module

}  // namespace Client
}  // namespace HoverRace
