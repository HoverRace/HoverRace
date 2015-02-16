
// TestLabScene.cpp
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
#include "../../engine/Display/MediaRes.h"
#include "../../engine/Display/Picture.h"
#include "../../engine/Display/PickList.h"
#include "../../engine/Display/RadioButton.h"
#include "../../engine/Display/ScreenFade.h"
#include "../../engine/Display/Slider.h"
#include "../../engine/Display/Speedometer.h"
#include "../../engine/Display/SymbolIcon.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Player/DemoProfile.h"
#include "../../engine/Player/LocalPlayer.h"
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
	using SUPER = FormScene;

public:
	LabModule(Display::Display &display, GameDirector &director,
		const std::string &title);
	virtual ~LabModule() { }

private:
	void OnCancel();

public:
	void AttachController(Control::InputEventController &controller) override;
	void DetachController(Control::InputEventController &controller) override;
	void PrepareRender() override;
	void Render() override;

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
	using SUPER = Display::Button;

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
	using SUPER = TestLabScene::ModuleButtonBase;

public:
	ModuleButton(Display::Display &display, GameDirector &director,
		const std::string &text) :
		SUPER(display, director, text)
	{
		GetClickedSignal().connect(std::bind(&ModuleButton::LaunchScene, this));
	}

	void LaunchScene() override
	{
		director.RequestPushScene(std::make_shared<Module>(display, director));
	}
}; //}}}

class ModuleMenuGrid : public Display::FlexGrid /*{{{*/
{
	using SUPER = Display::FlexGrid;

public:
	ModuleMenuGrid(Display::Display &display, GameDirector &director,
		const std::string &startingModuleName) :
		SUPER(display), director(director),
		startingModuleName(startingModuleName), startingModuleBtn(),
		row(0), col(0) { }
	virtual ~ModuleMenuGrid() { }

public:
	template<class T>
	void AddModule(const std::string &name)
	{
		auto btn = At(row, col).NewChild<ModuleButton<T>>(
			display, director, name)->GetContents();

		if (name == startingModuleName) {
			startingModuleBtn = btn;
		}

		++col;
		if (col == 6) {
			++row;
			col = 0;
		}
	}

	std::shared_ptr<TestLabScene::ModuleButtonBase> ShareStartingModuleBtn() const { return startingModuleBtn; }

private:
	GameDirector &director;
	std::string startingModuleName;
	std::shared_ptr<TestLabScene::ModuleButtonBase> startingModuleBtn;
	size_t row;
	size_t col;
}; //}}}

}  // namespace

namespace Module {

class ClickablesModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	ClickablesModule(Display::Display &display, GameDirector &director);
	virtual ~ClickablesModule() { }

private:
	std::shared_ptr<Display::Slider> AddSlider(double min, double max,
		double step);

private:
	void OnMessageClicked();

private:
	size_t curSliderRow;
	std::shared_ptr<Display::FlexGrid> grid;
	std::shared_ptr<Display::Button> messageBtn;
	std::unique_ptr<Display::RadioGroup<int>> group;

	boost::signals2::scoped_connection groupValueConn;
}; //}}}

class FlexGridModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	FlexGridModule(Display::Display &display, GameDirector &director);
	virtual ~FlexGridModule() { }

public:
	void Layout() override;

private:
	std::shared_ptr<Display::FlexGrid> grid;
	std::shared_ptr<Display::FlexGrid> sideGrid;
	std::shared_ptr<Display::FillBox> gridSizeBox;
}; //}}}

class HudModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	HudModule(Display::Display &display, GameDirector &director);
	virtual ~HudModule();

private:
	static std::unique_ptr<Player::Player> InitPlayer();

public:
	void Advance(Util::OS::timestamp_t tick) override;
	void PrepareRender() override;
	void Render() override;

private:
	std::shared_ptr<Player::Player> player;
	std::unique_ptr<Display::Hud> hud;
}; //}}}

class IconModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	IconModule(Display::Display &display, GameDirector &director);
	virtual ~IconModule() { }
}; //}}}

class LabelModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	LabelModule(Display::Display &display, GameDirector &director);
	virtual ~LabelModule() { }

private:
	void AdjustWrapWidth(double amt);

private:
	std::shared_ptr<Display::FillBox> wrapBox;
	std::shared_ptr<Display::Label> wrapLbl;
}; //}}}

class LayoutModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

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

class PickListModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	PickListModule(Display::Display &display, GameDirector &director);
	virtual ~PickListModule() { }

private:
	std::shared_ptr<Display::PickList<int>> list;
}; //}}}

class PictureModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	PictureModule(Display::Display &display, GameDirector &director);
	virtual ~PictureModule() { }

private:
	std::shared_ptr<Display::Picture> picture;
}; //}}}

class TransitionModule : public TestLabScene::LabModule /*{{{*/
{
	using SUPER = TestLabScene::LabModule;

public:
	TransitionModule(Display::Display &display, GameDirector &director);
	virtual ~TransitionModule() { }

protected:
	void OnPhaseChanged(Phase oldPhase) override;
	void OnStateChanged(State oldState) override;
	void OnPhaseTransition(double progress) override;
	void OnStateTransition(double progress) override;

private:
	std::shared_ptr<Display::Label> phaseLbl;
	std::shared_ptr<Display::FillBox> phaseBox;
	std::shared_ptr<Display::Label> stateLbl;
	std::shared_ptr<Display::FillBox> stateBox;
}; //}}}

}  // namespace Module

TestLabScene::TestLabScene(Display::Display &display, GameDirector &director,
	const std::string &startingModuleName) :
	SUPER(display, "Test Lab"),
	startingModuleName(startingModuleName)
{
	using namespace Display;

	// Clear the screen on every frame.
	fader.reset(new ScreenFade(COLOR_BLACK, 1.0));
	fader->AttachView(display);

	auto grid = GetRoot()->NewChild<ModuleMenuGrid>(
		display, director, startingModuleName);
	grid->SetPos(100, 100);

	grid->AddModule<Module::ClickablesModule>("Clickables");
	grid->AddModule<Module::FlexGridModule>("FlexGrid");
	grid->AddModule<Module::HudModule>("HUD");
	grid->AddModule<Module::IconModule>("Icon");
	grid->AddModule<Module::LabelModule>("Label");
	grid->AddModule<Module::LayoutModule>("Layout");
	grid->AddModule<Module::PickListModule>("PickList");
	grid->AddModule<Module::PictureModule>("Picture");
	grid->AddModule<Module::TransitionModule>("Transition");

	startingModuleBtn = grid->ShareStartingModuleBtn();

	if (!startingModuleName.empty() && !startingModuleBtn) {
		Log::Warn("Not a test lab module: %s", startingModuleName.c_str());
	}
}

TestLabScene::~TestLabScene()
{
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
	GameDirector &director, const std::string &title) :
	SUPER(display, "Lab Module (" + title + ")"),
	display(display), director(director), title(title)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	fader.reset(new ScreenFade(COLOR_BLACK, 1.0));
	fader->AttachView(display);

	cancelBtn = GetRoot()->NewChild<ActionButton>(display);
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

//{{{ ClickablesModule /////////////////////////////////////////////////////////////

ClickablesModule::ClickablesModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Button"),
	curSliderRow(1), group(new Display::RadioGroup<int>())
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;
	const auto &s = display.styles;

	Container *root = GetRoot();

	grid = root->NewChild<FlexGrid>(display);
	grid->SetPos(640, 360);
	grid->SetAlignment(Alignment::CENTER);

	grid->At(0, 0).NewChild<Label>("Buttons", s.bodyHeadFont, s.bodyHeadFg);
	grid->At(0, 1).NewChild<Label>("Sliders", s.bodyHeadFont, s.bodyHeadFg);
	grid->At(0, 2).NewChild<Label>("Radio", s.bodyHeadFont, s.bodyHeadFg);

	std::shared_ptr<Button> btn;
	std::shared_ptr<Slider> slider;
	std::shared_ptr<RadioButton<int>> radio;

	size_t row = 1;

	messageBtn = grid->At(row++, 0).NewChild<Button>(display, "Show Message")->
		GetContents();
	messageBtn->GetClickedSignal().connect(
		std::bind(&ClickablesModule::OnMessageClicked, this));

	btn = grid->At(row++, 0).NewChild<Button>(display, "Disabled Button")->
		GetContents();
	btn->SetEnabled(false);
	btn->GetClickedSignal().connect([](ClickRegion&) {
		Log::Error("Clicked on disabled button :(");
	});

	auto icon = std::make_shared<SymbolIcon>(60, 60, 0xf0ad, 0xbfffffff);
	icon->AttachView(display);

	btn = grid->At(row++, 0).NewChild<Button>(display, "Button With Icon")->
		GetContents();
	btn->SetIcon(icon);

	auto toggleChk = grid->At(row++, 0).NewChild<Checkbox>(
		display, "Checkbox")->GetContents();
	auto chk = grid->At(row++, 0).NewChild<Checkbox>(
		display, "Disableable")->GetContents();
	chk->SetChecked(true);
	chk->SetEnabled(false);
	toggleChk->GetClickedSignal().connect([=](ClickRegion&) {
		chk->SetEnabled(toggleChk->IsChecked());
	});

	slider = AddSlider(0, 100, 10);
	slider->SetValue(30);

	slider = AddSlider(-50, 50, 10);
	slider->SetValue(-30);

	slider = AddSlider(-100, 0, 10);
	slider->SetValue(-30);

	slider = AddSlider(50, 150, 10);
	slider->SetValue(80);

	slider = AddSlider(-150, -50, 10);
	slider->SetValue(-80);

	row = 1;

	radio = grid->At(row++, 2).NewChild<RadioButton<int>>(
		display, "Radio 1", 1)->GetContents();
	group->Add(radio);

	radio = grid->At(row++, 2).NewChild<RadioButton<int>>(
		display, "Radio 2", 2)->GetContents();
	group->Add(radio);

	groupValueConn = group->GetValueChangedSignal().connect([&]() {
		HR_LOG(info) << "Radio value: " << group->GetValue();
	});
}

std::shared_ptr<Display::Slider> ClickablesModule::AddSlider(
	double min, double max, double step)
{
	auto retv = grid->At(curSliderRow++, 1).NewChild<Display::Slider>(
		display, min, max, step)->GetContents();
	retv->SetSize(200, 20);
	return retv;
}

void ClickablesModule::OnMessageClicked()
{
	director.RequestPushScene(std::make_shared<MessageScene>(display, director,
		"Test Lab", "Hello, world!"));
}

//}}} ClickablesModule

//{{{ FlexGridModule //////////////////////////////////////////////////////////

FlexGridModule::FlexGridModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "FlexGrid")
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	const auto &s = display.styles;

	auto root = GetRoot();

	gridSizeBox = root->NewChild<FillBox>(0, 0, 0xff00003f);

	grid = root->NewChild<FlexGrid>(display);

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
	grid->At(r, c++).NewChild<Label>("Name", s.headingFont, s.headingFg);
	grid->At(r, c++).NewChild<Label>("Time", s.headingFont, s.headingFg);
	c++;
	grid->At(r, c++).NewChild<Label>("P", s.headingFont, s.headingFg);

	r++;
	c = 0;
	grid->At(r, c++).NewChild<Label>("1.", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Label>("Foo Bar", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Label>("3:44", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Button>(display, "Profile");
	grid->At(r, c++).NewChild<Button>(display, "View Statistics");

	r++;
	c = 0;
	grid->At(r, c++).NewChild<Label>("2.", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Label>("Baz Quux", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Label>("12:33", s.bodyFont, s.bodyFg);
	grid->At(r, c++).NewChild<Checkbox>(display, "Save Friend");
	grid->At(r, c++).NewChild<Button>(display, "Spectate");

	sideGrid = root->NewChild<FlexGrid>(display);
	sideGrid->SetPos(1280, 400);
	sideGrid->SetAlignment(Alignment::E);

	const auto AUTOSIZE = FlexGrid::AUTOSIZE;

	r = 0;
	sideGrid->At(r++, 0).NewChild<Button>(display, "Width")->
		GetContents()->GetClickedSignal().connect([&](ClickRegion&) {
			grid->SetFixedWidth(grid->IsFixedWidth() ? AUTOSIZE : 1280.0);
		});
	sideGrid->At(r++, 0).NewChild<Button>(display, "Height")->
		GetContents()->GetClickedSignal().connect([&](ClickRegion&) {
			grid->SetFixedHeight(grid->IsFixedHeight() ? AUTOSIZE : 720.0);
		});
}

void FlexGridModule::Layout()
{
	auto gridSize = grid->Measure();
	gridSizeBox->SetSize(gridSize.x, gridSize.y);
}

//}}} FlexGridModule

//{{{ HudModule ///////////////////////////////////////////////////////////////

HudModule::HudModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "HUD"),
	player(InitPlayer()),
	hud(new Display::Hud(display, player, std::shared_ptr<Model::Track>(),
		Display::UiLayoutFlags::FLOATING))
{
	using namespace Display;

	hud->AttachView(display);

	// A common HUD style.
	using HudAlignment = Hud::HudAlignment;
	auto fuelGauge = hud->At(HudAlignment::NE).NewChild<FuelGauge>(display);
	auto speedometer = hud->At(HudAlignment::NW).NewChild<Speedometer>(display);
}

HudModule::~HudModule()
{
	player->DetachMainCharacter();
}

std::unique_ptr<Player::Player> HudModule::InitPlayer()
{
	auto player = new Player::LocalPlayer(
		std::make_shared<Player::DemoProfile>(), false, true);
	player->AttachMainCharacter(MainCharacter::MainCharacter::New(0, 0x7f));

	return std::unique_ptr<Player::Player>(player);
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

//{{{ IconModule //////////////////////////////////////////////////////////////

IconModule::IconModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Icon")
{
	using namespace Display;

	auto root = GetRoot();

	std::shared_ptr<FillBox> box;
	std::shared_ptr<FillBox> icon;

	box = root->NewChild<FillBox>(60, 60, 0xff007f7f);
	box->SetPos(0, 40);
	icon = root->NewChild<SymbolIcon>(60, 60, 0xf046, 0xbfffffff);
	icon->SetPos(0, 40);
}

//}}} IconModule

//{{{ LabelModule //////////////////////////////////////////////////////////////

LabelModule::LabelModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Label")
{
	using namespace Display;

	Config *cfg = Config::GetInstance();
	const std::string &monospaceFontName = cfg->GetDefaultMonospaceFontName();
	const std::string &symbolFontName = cfg->GetDefaultSymbolFontName();

	auto root = GetRoot();

	std::shared_ptr<Button> btn;
	std::shared_ptr<Label> lbl;

	lbl = root->NewChild<Label>("Red 20 Normal", UiFont(20), 0xffff0000);
	lbl->SetPos(0, 20);
	lbl = root->NewChild<Label>("Yellow (75%) 25 Italic",
		UiFont(25, UiFont::ITALIC), 0xbfffff00);
	lbl->SetPos(0, 40);
	lbl = root->NewChild<Label>("Magenta (50%) 30 Bold+Italic",
		UiFont(30, UiFont::BOLD_ITALIC), 0x7fff00ff);
	lbl->SetPos(0, 65);

	lbl = root->NewChild<Label>("Default Font", UiFont(30), 0xffbfbfbf);
	lbl->SetPos(640, 20);
	lbl = root->NewChild<Label>("Monospace Font",
		UiFont(monospaceFontName, 30), 0xffbfbfbf);
	lbl->SetPos(640, 50);
	lbl = root->NewChild<Label>("< "
		"\xef\x84\x9b "  // gamepad
		"\xef\x82\x91 "  // trophy
		"\xef\x83\xa4 "  // dashboard
		">",
		UiFont(symbolFontName, 30), 0xffbfbfbf);
	lbl->SetPos(640, 80);

	// Wrapped text (with a background to visualize the width).

	std::string wrapText =
		"This is a string which should be wrapped to multiple lines to fit "
		"the fixed width label.\n\n"
		"ThisIsAVeryLongWordThatShouldBeClippedToTheFixedWidthOfTheLabel.\n\n"
		"This is the third line of the text.";

	wrapBox = root->NewChild<FillBox>(1280 / 4, 720 - 150, 0xff3f3f3f);
	wrapBox->SetPos(100, 150);

	wrapLbl = root->NewChild<Label>(1280 / 4, wrapText,
		UiFont(30), 0xffffffff);
	wrapLbl->SetPos(100, 150);

	// Buttons to increase / decrease the wrap width.

	btn = root->NewChild<Button>(display, "->|");
	btn->SetPos(0, 150);
	btn->GetClickedSignal().connect(std::bind(
		&LabelModule::AdjustWrapWidth, this, 50));

	btn = root->NewChild<Button>(display, "|<-");
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

//{{{ LayoutModule /////////////////////////////////////////////////////////////

LayoutModule::LayoutModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Layout")
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	auto root = GetRoot();

	displayConfigChangedConn = display.GetDisplayConfigChangedSignal().
		connect(std::bind(&LayoutModule::OnDisplayConfigChanged, this));

	// This box takes up the entire UI viewport, so we move it to the
	// beginning of the list so we don't render on top of the "Close" button.
	displayInfoBox = root->NewChild<FillBox>(1280, 720, 0xff3f3f3f);
	root->ReorderChild(displayInfoBox, 0);

	displayInfoLbl = root->NewChild<Label>("Res", UiFont(20), COLOR_WHITE);
	displayInfoLbl->SetPos(640, 360);
	displayInfoLbl->SetAlignment(Alignment::CENTER);
	OnDisplayConfigChanged();

	std::shared_ptr<FillBox> fillBox;
	std::shared_ptr<Label> lbl;

	fillBox = root->NewChild<FillBox>(100, 100, 0x7fff0000);
	fillBox->SetPos(100, 20);
	fillBox = root->NewChild<FillBox>(100, 100, 0x7f00ff00);
	fillBox->SetPos(150, 70);

	AddAlignmentTestElem(Alignment::SW, "| Southwest", 0, 719);
	AddAlignmentTestElem(Alignment::S, "South", 639, 719);
	AddAlignmentTestElem(Alignment::SE, "Southeast |", 1279, 719);

	lbl = root->NewChild<Label>("Red 20 Normal", UiFont(20), 0xffff0000);
	lbl->SetPos(0, 20);
	lbl = root->NewChild<Label>("Yellow (75%) 25 Italic",
		UiFont(25, UiFont::ITALIC), 0xbfffff00);
	lbl->SetPos(0, 40);
	lbl = root->NewChild<Label>("Magenta (50%) 30 Bold+Italic",
		UiFont(30, UiFont::BOLD_ITALIC), 0x7fff00ff);
	lbl->SetPos(0, 65);
}

void LayoutModule::AddAlignmentTestElem(
	Display::UiViewModel::Alignment alignment,
	const std::string &label, double x, double y)
{
	using namespace Display;

	auto fillBox = GetRoot()->NewChild<FillBox>(50, 50, 0x7f00ffff);
	fillBox->SetAlignment(alignment);
	fillBox->SetPos(x, y);

	auto lbl = GetRoot()->NewChild<Label>(label, UiFont(40), 0xffffffff);
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

//{{{ PickListModule ///////////////////////////////////////////////////////////

PickListModule::PickListModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "PickList")
{
	using namespace Display;

	auto root = GetRoot();

	list = root->NewChild<PickList<int>>(display, Vec2(200, 400));
	list->SetPos(60, 60);
	list->GetValueChangedSignal().connect([&]() {
		HR_LOG(info) << "Selected value: " << *(list->GetValue());
	});
	list->Add("1: Foo", 1);
	list->Add("2: Bar", 2);
	list->Add("3: Baz", 3);
	list->Add("4: Quux", 4);
	list->Add(5);
	list->Add(6);
	list->Add(7);

	auto grid = root->NewChild<FlexGrid>(display);
	grid->SetPos(300, 60);
	
	size_t r = 0;
	size_t c = 0;

	auto lp = list.get();
	grid->At(r++, c).NewChild<Button>(display, "Filter: All")->
		GetContents()->GetClickedSignal().connect([=](ClickRegion&) {
			lp->ApplyFilter([](int) { return true; });
		});
	grid->At(r++, c).NewChild<Button>(display, "Filter: Even")->
		GetContents()->GetClickedSignal().connect([=](ClickRegion&) {
			lp->ApplyFilter([](int x) { return x % 2 == 0; });
		});
	grid->At(r++, c).NewChild<Button>(display, "Filter: Odd")->
		GetContents()->GetClickedSignal().connect([=](ClickRegion&) {
			lp->ApplyFilter([](int x) { return x % 2 == 1; });
		});
}

//}}} PickListModule

//{{{ PictureModule ////////////////////////////////////////////////////////////

PictureModule::PictureModule(Display::Display &display, GameDirector &director) :
	SUPER(display, director, "Picture")
{
	using namespace Display;

	auto root = GetRoot();

	auto tex = std::make_shared<MediaRes<Texture>>("ui/bg/practice.png");
	root->NewChild<Picture>(tex, 640, 360);
}

//}}} PictureModule

//{{{ TransitionModule ////////////////////////////////////////////////////////

TransitionModule::TransitionModule(Display::Display &display,
	GameDirector &director) :
	SUPER(display, director, "Transition")
{
	using namespace Display;

	SetPhaseTransitionDuration(3000);
	SetStateTransitionDuration(3000);

	auto root = GetRoot();

	Config *cfg = Config::GetInstance();
	const std::string &fontName = cfg->GetDefaultFontName();
	UiFont font(fontName, 40);

	phaseLbl = root->NewChild<Label>("", font, 0xffffff00);
	phaseLbl->SetPos(60, 120);

	phaseBox = root->NewChild<FillBox>(0, 40, 0xffbfbf00);
	phaseBox->SetPos(60, 160);

	stateLbl = root->NewChild<Label>("", font, 0xff00ffff);
	stateLbl->SetPos(60, 200);

	stateBox = root->NewChild<FillBox>(0, 40, 0xff00bfbf);
	stateBox->SetPos(60, 240);

	auto msgBtn = root->NewChild<Button>(display, "Click");
	msgBtn->SetPos(60, 300);
	msgBtn->GetClickedSignal().connect([&](ClickRegion&) {
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

}  // namespace Module

}  // namespace Client
}  // namespace HoverRace
