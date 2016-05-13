
// ConsoleScene.cpp
//
// Copyright (c) 2013-2016 Michael Imamura.
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

#include "../../../engine/Control/Controller.h"
#include "../../../engine/Display/ActiveText.h"
#include "../../../engine/Display/Display.h"
#include "../../../engine/Display/Label.h"
#include "../../../engine/Display/ScreenFade.h"
#include "../../../engine/Util/Config.h"
#include "../../../engine/Util/Log.h"
#include "../GameDirector.h"

#include "ConsoleScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {

static const std::string COMMAND_PROMPT(">> ");
static const std::string CONTINUE_PROMPT(":> ");

}

class ConsoleScene::LogLines /*{{{*/
{
public:
	LogLines(Display::Display &display, const Vec2 &charSize);
	~LogLines();

	LogLines &operator=(const LogLines&) = delete;

public:
	void ScrollTop();
	void Scroll(int i);
	void ScrollBottom();

public:
	void Add(const std::string &s, const Display::UiFont &font, Display::Color color);
	void Clear();

public:
	void PrepareRender();
	void Render();

private:
	Display::Display &display;

	const Vec2 &charSize;
	std::deque<Display::ActiveText*> lines;
	size_t pos;
	size_t num;

	static const unsigned int MAX_LINES = 100;
}; //}}}

ConsoleScene::ConsoleScene(Display::Display &display, GameDirector &director,
	SysConsole &console) :
	SUPER("Console"),
	director(director), console(console),
	lastLogIdx(-1), logsChanged(true), layoutChanged(true),
	cursorOn(true), cursorTick(0), charSize(0, 0)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	const auto &s = display.styles;

	displayConfigChangedConn = display.GetDisplayConfigChangedSignal().
		connect(std::bind(&ConsoleScene::OnDisplayConfigChanged, this));

	fader.reset(new ScreenFade(s.consoleBg, 1.0));
	fader->AttachView(display);

	const auto &font = s.consoleFont;

	inputLbl.reset(new ActiveText(COMMAND_PROMPT, font, s.consoleFg));
	inputLbl->SetPos(0, 719);
	inputLbl->SetAlignment(Alignment::SW);
	inputLbl->AttachView(display);

	measureLbl.reset(new Label(" ", font, s.consoleFg));
	measureLbl->AttachView(display);

	logClearedConn = console.GetLogClearedSignal().connect(
		std::bind(&ConsoleScene::OnLogCleared, this));
	logAddedConn = console.GetLogAddedSignal().connect(
		std::bind(&ConsoleScene::OnLogAdded, this, std::placeholders::_1));

	logLines.reset(new LogLines(display, charSize));

	UpdateCommandLine();
}

ConsoleScene::~ConsoleScene()
{
}

void ConsoleScene::OnDisplayConfigChanged()
{
	layoutChanged = true;

	// We need to redo the line-wrapping since the font size changed.
	logLines->Clear();
	lastLogIdx = -1;
	logsChanged = true;
}

void ConsoleScene::OnConsoleToggle()
{
	director.RequestPopScene();
}

void ConsoleScene::OnConsoleUp()
{
	logLines->Scroll(1);
}

void ConsoleScene::OnConsoleDown()
{
	logLines->Scroll(-1);
}

void ConsoleScene::OnConsoleTop()
{
	logLines->ScrollTop();
}

void ConsoleScene::OnConsoleBottom()
{
	logLines->ScrollBottom();
}

void ConsoleScene::OnConsolePrevCmd()
{
	console.LoadPrevCmd();
	UpdateCommandLine();
}

void ConsoleScene::OnConsoleNextCmd()
{
	console.LoadNextCmd();
	UpdateCommandLine();
}

void ConsoleScene::OnTextInput(const std::string &s)
{
	cursorTick = OS::Time();

	console.GetCommandLine() += s;
	UpdateCommandLine();
}

void ConsoleScene::OnTextControl(Control::TextControl::key_t key)
{
	cursorTick = OS::Time();

	std::string &commandLine = console.GetCommandLine();
	switch (key) {
		case Control::TextControl::BACKSPACE:
			if (!commandLine.empty()) {
				commandLine.resize(commandLine.length() - 1);
				UpdateCommandLine();
			}
			break;

		case Control::TextControl::ENTER:
			console.LogHistory(inputLbl->GetText());
			console.SubmitChunkWithHistory(commandLine);
			commandLine.clear();
			UpdateCommandLine();
			break;

		default:
			HR_LOG(warning) <<
				"ConsoleScene: Unhandled text control key: " << key;
	}
}

void ConsoleScene::OnLogCleared()
{
	logLines->Clear();
}

void ConsoleScene::OnLogAdded(int idx)
{
	if (layoutChanged) {
		// Haven't measured yet, so defer the update until later.
		logsChanged = true;
	}

	if (idx > lastLogIdx) {
		// Casts are safe since lastLogIdx can only be as low as -1.
		console.ReadLogs(
			static_cast<size_t>(lastLogIdx + 1),
			static_cast<size_t>(idx),
			std::bind(&ConsoleScene::AppendLogLine, this, std::placeholders::_1));
	}
}

void ConsoleScene::AppendLogLine(const SysConsole::LogLine &line)
{
	if (layoutChanged) {
		// Haven't measured yet, so defer the update until later.
		logsChanged = true;
	}

	const Display::UiFont &font = measureLbl->GetFont();

	// Colorize based on message type.
	Display::Color color(0xff7f7f7f);
	switch (line.level) {
		case SysConsole::LogLevel::HISTORY: color = 0xffffffff; break;
		case SysConsole::LogLevel::NOTE: color = 0xffffff00; break;
		case SysConsole::LogLevel::INFO: color = 0xffbfbfbf; break;
		case SysConsole::LogLevel::ERROR: color = 0xffff0000; break;
		default:
			HR_LOG(warning) << "ConsoleScene: Unhandled log level: " <<
				static_cast<int>(line.level);
	}

	// Split into into multiple lines based on the current font measurement.
	const std::string &s = line.line;
	std::string buf;
	buf.reserve(1024);
	double charWidth = charSize.x;
	double consoleWidth = 1280;
	int i = 0;
	for (std::string::const_iterator iter = s.begin(); iter != s.end();
		++iter, ++i)
	{
		char c = *iter;
		switch (c) {
			case '\t':
				buf.resize(buf.length() + (8 - (buf.length() % 8)), ' ');
				break;

			case '\n':
				logLines->Add(buf, font, color);
				buf.clear();
				break;

			default:
				if (c >= 32 && c < 127) {
					// Line wrap if necessary.
					if (static_cast<double>(buf.length() + 1) * charWidth > consoleWidth) {
						logLines->Add(buf, font, color);
						buf.clear();
					}
					buf += c;
				}
		}
	}
	logLines->Add(buf, font, color);

	lastLogIdx = line.idx;
}

void ConsoleScene::UpdateCommandLine()
{
	inputLbl->SetText(
		(console.GetInputState() == Console::InputState::COMMAND ?
			COMMAND_PROMPT : CONTINUE_PROMPT) + console.GetCommandLine());
}

void ConsoleScene::Layout()
{
	// Measure the size of a single character.
	// We assume that the monospace font we're using really has the same height
	// and width for all glyphs.
	const Vec3 charSize3 = measureLbl->Measure();
	charSize.x = charSize3.x;
	charSize.y = charSize3.y;

	layoutChanged = false;
}

void ConsoleScene::AttachController(Control::InputEventController &controller,
	ConnList &conns)
{
	controller.AddConsoleToggleMaps();
	conns <<
		controller.actions.sys.consoleToggle->Connect(
			std::bind(&ConsoleScene::OnConsoleToggle, this));

	controller.AddConsoleMaps();
	conns <<
		controller.actions.ui.consoleUp->Connect(
			std::bind(&ConsoleScene::OnConsoleUp, this)) <<
		controller.actions.ui.consoleDown->Connect(
			std::bind(&ConsoleScene::OnConsoleDown, this)) <<
		controller.actions.ui.consoleTop->Connect(
			std::bind(&ConsoleScene::OnConsoleTop, this)) <<
		controller.actions.ui.consoleBottom->Connect(
			std::bind(&ConsoleScene::OnConsoleBottom, this)) <<
		controller.actions.ui.consolePrevCmd->Connect(
			std::bind(&ConsoleScene::OnConsolePrevCmd, this)) <<
		controller.actions.ui.consoleNextCmd->Connect(
			std::bind(&ConsoleScene::OnConsoleNextCmd, this));

	conns <<
		controller.actions.ui.control->Connect(
			std::bind(&ConsoleScene::OnTextControl, this, std::placeholders::_1)) <<
		controller.actions.ui.text->Connect(
			std::bind(&ConsoleScene::OnTextInput, this, std::placeholders::_1));

	SDL_StartTextInput();
}

void ConsoleScene::DetachController(Control::InputEventController&, ConnList&)
{
	SDL_StopTextInput();
}

void ConsoleScene::Advance(Util::OS::timestamp_t tick)
{
	SUPER::Advance(tick);

	// Cursor visibility is based on the last character typed
	// (so that the cursor stays visible while typing).
	cursorOn = (OS::TimeDiff(tick, cursorTick) % 1000) < 500;
}

void ConsoleScene::PrepareRender()
{
	fader->PrepareRender();

	if (layoutChanged) {
		Layout();
	}

	// If there were logs that appeared before we measured the font size
	// in Layout(), then we need to read them now to catch up.
	if (logsChanged) {
		console.AddIntroLines();
		console.ReadLogs(static_cast<size_t>(lastLogIdx + 1),
			std::bind(&ConsoleScene::AppendLogLine, this, std::placeholders::_1));
		logsChanged = false;
	}

	inputLbl->SetCaretPos(inputLbl->GetText().length());
	inputLbl->SetCaretVisible(cursorOn);

	inputLbl->PrepareRender();
	logLines->PrepareRender();
}

void ConsoleScene::Render()
{
	fader->Render();
	inputLbl->Render();
	logLines->Render();
}

//{{{ ConsoleScene::LogLines ///////////////////////////////////////////////////

ConsoleScene::LogLines::LogLines(Display::Display &display,
	const Vec2 &charSize) :
	display(display), charSize(charSize),
	lines(), pos(0), num(10)
{
}

ConsoleScene::LogLines::~LogLines()
{
	Clear();
}

void ConsoleScene::LogLines::ScrollTop()
{
	pos = (lines.size() <= num) ? 0 : (lines.size() - num);
}

void ConsoleScene::LogLines::Scroll(int i)
{
	if (i < 0) {
		if ((unsigned int)-i >= pos) {
			ScrollBottom();
		}
		else {
			pos -= static_cast<size_t>(-i);
		}
	}
	else if (lines.size() > num) {
		size_t ct = static_cast<size_t>(i);
		if (pos + ct > lines.size() - num) {
			ScrollTop();
		}
		else {
			pos += ct;
		}
	}
}

void ConsoleScene::LogLines::ScrollBottom()
{
	pos = 0;
}

void ConsoleScene::LogLines::Add(const std::string &s,
	const Display::UiFont &font, Display::Color color)
{
	// Remove the top line if we're full.
	if (lines.size() == MAX_LINES) {
		delete lines.front();
		lines.pop_front();
	}

	Display::ActiveText *lbl = new Display::ActiveText(
		s.empty() ? " " : s, font, color);
	lbl->SetAlignment(Display::UiViewModel::Alignment::SW);
	lbl->AttachView(display);

	lines.push_back(lbl);
	ScrollBottom();
}

void ConsoleScene::LogLines::Clear()
{
	for (auto iter = lines.begin(); iter != lines.end(); ++iter)
	{
		delete *iter;
	}
	lines.clear();
	pos = 0;
}

void ConsoleScene::LogLines::PrepareRender()
{
	double charHeight = charSize.y;
	double y = 720.0 - charHeight;

	num = 0;
	auto iter = lines.rbegin();
	std::advance(iter, pos);
	while (iter != lines.rend() && y >= charHeight) {
		Display::ActiveText *lbl = *iter;
		lbl->SetPos(0, y);
		lbl->PrepareRender();

		++iter;
		++num;
		y -= charHeight;
	}
}

void ConsoleScene::LogLines::Render()
{
	unsigned int i = 0;
	auto iter = lines.rbegin();
	std::advance(iter, pos);
	while (iter != lines.rend() && i < num) {
		(*iter)->Render();

		++iter;
		++i;
	}
}

//}}} ConsoleScene::LogLines

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
