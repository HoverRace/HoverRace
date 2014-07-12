
// TestLabScene.cpp
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

#include "StdAfx.h"

#include "../../engine/Control/Controller.h"
#include "../../engine/Display/ActionButton.h"
#include "../../engine/Display/Button.h"
#include "../../engine/Display/Checkbox.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Display.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/FuelGauge.h"
#include "../../engine/Display/Hud.h"
#include "../../engine/Display/Label.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Display/Speedometer.h"
#include "../../engine/Display/SymbolIcon.h"
#include "../../engine/MainCharacter/MainCharacter.h"
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

	private:
		void OnCancel();

	public:
		virtual void AttachController(Control::InputEventController &controller);
		virtual void DetachController(Control::InputEventController &controller);
		virtual void PrepareRender();
		virtual void Render();

	protected:
		Display::Display &display;
		GameDirector &director;
		const std::string title;
	private:
		std::unique_ptr<Display::ScreenFade> fader;
		std::shared_ptr<Display::ActionButton> cancelBtn;
		boost::signals2::connection cancelConn;
}; //}}}

class TestLabScene::ModuleButtonBase : public Display::Button /*{{{*/
{
	typedef Display::Button SUPER;
	public:
		ModuleButtonBase(Display::Display &display, GameDirector &director,
			const std::string &text) :
			SUPER(display, text), display(display), director(director)
		{
		}

		virtual void LaunchScene() = 0;

	protected:
		Display::Display &display;
		GameDirector &director;
}; //}}}

namespace {
	template<typename Module>
	class ModuleButton : public TestLabScene::ModuleButtonBase /*{{{*/
	{
		typedef TestLabScene::ModuleButtonBase SUPER;
		public:
			ModuleButton(Display::Display &display, GameDirector &director,
				const std::string &text) :
				SUPER(display, director, text)
			{
				GetClickedSignal().connect(std::bind(&ModuleButton::LaunchScene, this));
			}

			virtual void LaunchScene()
			{
				director.RequestPushScene(std::make_shared<Module>(display, director));
			}
	}; //}}}

	template<class T>
	void AddModule(TestLabScene &scene, Display::Display &display,
                   GameDirector &director, const std::string &name)
	{
		scene.AddModuleButton(new ModuleButton<T>(display, director, name));
	}
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
				Display::UiViewModel::Alignment alignment,
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

	class IconModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			IconModule(Display::Display &display, GameDirector &director);
			virtual ~IconModule() { }
	}; //}}}

	class TransitionModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			TransitionModule(Display::Display &display, GameDirector &director);
			virtual ~TransitionModule() { }

		protected:
			virtual void OnPhaseChanged(Phase oldPhase);
			virtual void OnStateChanged(State oldState);
			virtual void OnPhaseTransition(double progress);
			virtual void OnStateTransition(double progress);

		private:
			std::shared_ptr<Display::Label> phaseLbl;
			std::shared_ptr<Display::FillBox> phaseBox;
			std::shared_ptr<Display::Label> stateLbl;
			std::shared_ptr<Display::FillBox> stateBox;
	}; //}}}

	class HudModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			HudModule(Display::Display &display, GameDirector &director);
			virtual ~HudModule() { }

		public:
			virtual void Advance(Util::OS::timestamp_t tick);
			virtual void PrepareRender();
			virtual void Render();

		private:
			std::unique_ptr<MainCharacter::MainCharacter> player;
			std::unique_ptr<Display::Hud> hud;
	}; //}}}

	class FlexGridModule : public TestLabScene::LabModule /*{{{*/
	{
		typedef TestLabScene::LabModule SUPER;
		public:
			FlexGridModule(Display::Display &display, GameDirector &director);
			virtual ~FlexGridModule() { }

		public:
			virtual void Layout() override;

		private:
			std::shared_ptr<Display::FlexGrid> grid;
			std::shared_ptr<Display::FlexGrid> sideGrid;
			std::shared_ptr<Display::FillBox> gridSizeBox;
	}; //}}}
}

TestLabScene::TestLabScene(Display::Display &display, GameDirector &director,
                           const std::string &startingModuleName) :
	SUPER(display, "Test Lab"),
	startingModuleName(startingModuleName), btnPosY(60)
{
	// Clear the screen on every frame.
	fader.reset(new Display::ScreenFade(Display::COLOR_BLACK, 1.0));
	fader->AttachView(display);

	AddModule<Module::LayoutModule>(*this, display, director, "Layout");
	AddModule<Module::ButtonModule>(*this, display, director, "Button");
	AddModule<Module::LabelModule>(*this, display, director, "Label");
	AddModule<Module::IconModule>(*this, display, director, "Icon");
	AddModule<Module::TransitionModule>(*this, display, director, "Transition");
	AddModule<Module::HudModule>(*this, display, director, "HUD");
	AddModule<Module::FlexGridModule>(*this, display, director, "FlexGrid");

	if (!startingModuleName.empty() && !startingModuleBtn) {
		Log::Warn("Not a test lab module: %s", startingModuleName.c_str());
	}
}

TestLabScene::~TestLabScene()
{
}

void TestLabScene::AddModuleButton(ModuleButtonBase *btn)
{
	auto mbtn = GetRoot()->AddChild(btn);
	btn->SetPos(0, btnPosY);
	btnPosY += 60;

	if (btn->GetText() == startingModuleName) {
		startingModuleBtn = mbtn;
	}
}

void TestLabScene::OnScenePushed()
{
	if (startingModuleBtn) {
		startingModuleBtn->LaunchScene();
		startingModuleBtn.reset();
	}
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

	cancelBtn = GetRoot()->AddChild(new Display::ActionButton(display));
	cancelBtn->SetPos(1280, 0);
	cancelBtn->SetAlignment(Alignment::NE);
}

void TestLabScene::LabModule::OnCancel()
{
	director.RequestPopScene();
}

void TestLabScene::LabModule::AttachController(
	Control::InputEventController &controller)
{
	SUPER::AttachController(controller);

	controller.AddMenuMaps();

	auto &menuCancelAction = controller.actions.ui.menuCancel;

	cancelConn = menuCancelAction->Connect(std::bind(
		&TestLabScene::LabModule::OnCancel, this));

	cancelBtn->AttachAction(controller, menuCancelAction);
}

void TestLabScene::LabModule::DetachController(
	Control::InputEventController &controller)
{
	cancelConn.disconnect();

	SUPER::DetachController(controller);
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
	Display::UiViewModel::Alignment alignment,
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
	const Vec2 &uiOffset = display.GetUiOffset();

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

	std::shared_ptr<Display::Button> btn;

	messageBtn = root->AddChild(new Display::Button(display, "Show Message"));
	messageBtn->SetPos(640, 0);
	messageBtn->SetAlignment(Alignment::N);
	messageBtn->GetClickedSignal().connect(
		std::bind(&ButtonModule::OnMessageClicked, this));

	btn = root->AddChild(new Display::Button(display, "Disabled Button"));
	btn->SetEnabled(false);
	btn->SetPos(640, 60);
	btn->SetAlignment(Alignment::N);
	btn->GetClickedSignal().connect([](Display::ClickRegion&) {
		Log::Error("Clicked on disabled button :(");
	});

	auto icon = std::make_shared<Display::SymbolIcon>(60, 60, 0xf0ad, 0xbfffffff);
	icon->AttachView(display);

	btn = root->AddChild(new Display::Button(display, "Button With Icon"));
	btn->SetPos(640, 120);
	btn->SetIcon(icon);
	btn->SetAlignment(Alignment::N);

	btn = root->AddChild(new Display::Checkbox(display, "Checkbox"));
	btn->SetPos(640, 180);
	btn->SetAlignment(Alignment::N);
}

void ButtonModule::OnMessageClicked()
{
	director.RequestPushScene(std::make_shared<MessageScene>(display, director,
		"Test Lab", "Hello, world!"));
}

//}}} ButtonModule

//{{{ LabelModule //////////////////////////////////////////////////////////////

LabelModule::LabelModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Label")
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
	const Vec2 &curSize = wrapBox->GetSize();
	double newWidth = curSize.x + amt;

	if (newWidth > 0 && newWidth < (1280.0 - wrapBox->GetPos().x)) {
		wrapBox->SetSize(newWidth, curSize.y);
		wrapLbl->SetWrapWidth(newWidth);
	}
}

//}}} LabelModule

//{{{ IconModule //////////////////////////////////////////////////////////////

IconModule::IconModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Icon")
{
	Display::Container *root = GetRoot();

	std::shared_ptr<Display::FillBox> box;
	std::shared_ptr<Display::FillBox> icon;

	box = root->AddChild(new Display::FillBox(60, 60, 0xff007f7f));
	box->SetPos(0, 40);
	icon = root->AddChild(new Display::SymbolIcon(60, 60, 0xf046, 0xbfffffff));
	icon->SetPos(0, 40);
}

//}}} IconModule

//{{{ TransitionModule ////////////////////////////////////////////////////////

TransitionModule::TransitionModule(Display::Display &display,
                                   GameDirector &director) :
	SUPER(display, director, "Transition")
{
	SetPhaseTransitionDuration(3000);
	SetStateTransitionDuration(3000);

	Display::Container *root = GetRoot();

	Config *cfg = Config::GetInstance();
	const std::string &fontName = cfg->GetDefaultFontName();
	Display::UiFont font(fontName, 40);

	phaseLbl = root->AddChild(new Display::Label("", font, 0xffffff00));
	phaseLbl->SetPos(60, 120);

	phaseBox = root->AddChild(new Display::FillBox(0, 40, 0xffbfbf00));
	phaseBox->SetPos(60, 160);

	stateLbl = root->AddChild(new Display::Label("", font, 0xff00ffff));
	stateLbl->SetPos(60, 200);

	stateBox = root->AddChild(new Display::FillBox(0, 40, 0xff00bfbf));
	stateBox->SetPos(60, 240);

	auto msgBtn = root->AddChild(new Display::Button(display, "Click"));
	msgBtn->SetPos(60, 300);
	msgBtn->GetClickedSignal().connect([&](Display::ClickRegion&) {
		director.RequestPushScene(std::make_shared<MessageScene>(display, director,
			"Test Lab", "The lab module has been moved to the background."));
	});
}

void TransitionModule::OnPhaseChanged(Phase oldPhase)
{
	SUPER::OnPhaseChanged(oldPhase);

	std::string s = "Phase: ";
	switch (GetPhase()) {
		case Phase::STARTING: s += "STARTING"; break;
		case Phase::RUNNING: s += "RUNNING"; break;
		case Phase::STOPPING: s += "STOPPING"; break;
		case Phase::STOPPED: s += "STOPPED"; break;
		default:
			s += boost::str(boost::format("UNKNOWN: %d") % (int)GetPhase());
	}
	phaseLbl->SetText(s);
}

void TransitionModule::OnStateChanged(State oldState)
{
	SUPER::OnStateChanged(oldState);

	std::string s = "State: ";
	switch (GetState()) {
		case State::BACKGROUND: s += "BACKGROUND"; break;
		case State::RAISING: s += "RAISING"; break;
		case State::FOREGROUND: s += "FOREGROUND"; break;
		case State::LOWERING: s += "LOWERING"; break;
		default:
			s += boost::str(boost::format("UNKNOWN: %d") % (int)GetState());
	}
	stateLbl->SetText(s);
}

void TransitionModule::OnPhaseTransition(double progress)
{
	phaseBox->SetSize(progress * 200, 40);
}

void TransitionModule::OnStateTransition(double progress)
{
	stateBox->SetSize(progress * 200, 40);
}

//}}} TransitionModule

//{{{ HudModule ///////////////////////////////////////////////////////////////

HudModule::HudModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "HUD"),
	player(MainCharacter::MainCharacter::New(0, 0x7f)),
	hud(new Display::Hud(display, player.get(), Vec2(1280, 720)))
{
	hud->AttachView(display);

	// A common HUD style.
	typedef Display::Hud::HudAlignment HudAlignment;
	auto fuelGauge = hud->AddHudChild(HudAlignment::NE,
		new Display::FuelGauge(display));
	auto speedometer = hud->AddHudChild(HudAlignment::NW,
		new Display::Speedometer(display));
}

void HudModule::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);
	hud->Advance(tick);
}

void HudModule::PrepareRender()
{
	SUPER::PrepareRender();
	hud->PrepareRender();
}

void HudModule::Render()
{
	SUPER::Render();
	hud->Render();
}

//}}} HudModule

//{{{ FlexGridModule //////////////////////////////////////////////////////////

FlexGridModule::FlexGridModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "FlexGrid")
{
	typedef Display::UiViewModel::Alignment Alignment;

	const auto &s = display.styles;

	Display::Container *root = GetRoot();

	gridSizeBox = root->AddChild(new Display::FillBox(0, 0, 0xff00003f));

	grid = root->AddChild(new Display::FlexGrid(display));

	size_t r = 0;
	size_t c = 0;

	c = 0;
	{
		auto &column = grid->GetColumnDefault(c++);
		column.SetAlignment(Alignment::E);
	}
	{
		auto &column = grid->GetColumnDefault(c++);
		column.SetAlignment(Alignment::W);
	}
	{
		auto &column = grid->GetColumnDefault(c++);
		column.SetAlignment(Alignment::E);
	}
	{
		auto &column = grid->GetColumnDefault(c++);
		column.SetAlignment(Alignment::CENTER);
	}
	{
		auto &column = grid->GetColumnDefault(c++);
		column.SetAlignment(Alignment::CENTER);
		column.SetFill(true);
	}

	c = 1;
	grid->AddGridCell(r, c++, new Display::Label("Name",
		s.headingFont, s.headingFg));
	grid->AddGridCell(r, c++, new Display::Label("Time",
		s.headingFont, s.headingFg));
	c++;
	grid->AddGridCell(r, c++, new Display::Label("P",
		s.headingFont, s.headingFg));

	r++;
	c = 0;
	grid->AddGridCell(r, c++, new Display::Label("1.",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Label("Foo Bar",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Label("3:44",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Button(display, "Profile"));
	grid->AddGridCell(r, c++, new Display::Button(display, "View Statistics"));

	r++;
	c = 0;
	grid->AddGridCell(r, c++, new Display::Label("2.",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Label("Baz Quux",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Label("12:33",
		s.bodyFont, s.bodyFg));
	grid->AddGridCell(r, c++, new Display::Checkbox(display, "Save Friend"));
	grid->AddGridCell(r, c++, new Display::Button(display, "Spectate"));

	sideGrid = root->AddChild(new Display::FlexGrid(display));
	sideGrid->SetPos(0, 400);
	//sideGrid->SetPos(1280, 400);
	//sideGrid->SetAlignment(Alignment::E);

	const auto AUTOSIZE = Display::FlexGrid::AUTOSIZE;

	r = 0;
	sideGrid->AddGridCell(r++, 0, new Display::Button(display, "Width"))->
		GetContents()->GetClickedSignal().connect([&](Display::ClickRegion&) {
			grid->SetFixedWidth(grid->IsFixedWidth() ? AUTOSIZE : 1280.0);
		});
	sideGrid->AddGridCell(r++, 0, new Display::Button(display, "Height"))->
		GetContents()->GetClickedSignal().connect([&](Display::ClickRegion&) {
			grid->SetFixedHeight(grid->IsFixedHeight() ? AUTOSIZE : 720.0);
		});
}

void FlexGridModule::Layout()
{
	auto gridSize = grid->Measure();
	gridSizeBox->SetSize(gridSize.x, gridSize.y);
}

//}}} FlexGridModule

}  // namespace Module

}  // namespace Client
}  // namespace HoverRace
