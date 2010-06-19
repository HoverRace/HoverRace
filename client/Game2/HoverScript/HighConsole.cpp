
// HighConsole.cpp
// On-screen debug console.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <deque>

#include <boost/thread/locks.hpp>

#include <lua.hpp>
#include <luabind/luabind.hpp>

#include "../../../engine/Script/Core.h"
#include "../../../engine/Script/Help/HelpHandler.h"
#include "../../../engine/Script/Help/Class.h"
#include "../../../engine/Util/Config.h"
#include "../../../engine/VideoServices/2DViewPort.h"
#include "../../../engine/VideoServices/StaticText.h"
#include "../../../engine/VideoServices/VideoBuffer.h"
#include "../Control/Controller.h"
#include "../Control/InputHandler.h"
#include "../GameApp.h"
#include "GamePeer.h"
#include "SessionPeer.h"

#include "HighConsole.h"

using HoverRace::Util::Config;
using HoverRace::Util::OS;
using HoverRace::VideoServices::Font;
using HoverRace::VideoServices::StaticText;

#ifdef max
#	undef max
#endif
#ifdef HELP_KEY
#	undef HELP_KEY
#endif

namespace {
	static const std::string COMMAND_PROMPT(">> ");
	static const std::string CONTINUE_PROMPT(":> ");

	static const Config::cfg_control_t HELP_KEY = Config::cfg_control_t::Key(OIS::KC_F1);

	static const Config::cfg_control_t SCROLL_TOP_KEY = Config::cfg_control_t::Key(OIS::KC_HOME);
	static const Config::cfg_control_t SCROLL_UP_KEY = Config::cfg_control_t::Key(OIS::KC_PGUP);
	static const Config::cfg_control_t SCROLL_DOWN_KEY = Config::cfg_control_t::Key(OIS::KC_PGDOWN);
	static const Config::cfg_control_t SCROLL_BOTTOM_KEY = Config::cfg_control_t::Key(OIS::KC_END);

	static const int SCROLL_SPEED = 3;
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

class HighConsole::LogLines
{
	public:
		LogLines();
		~LogLines();

	private:
		void RecalcHeight();
	public:
		int GetHeight() const { return height; }

		void ScrollTop();
		void Scroll(int i);
		void ScrollBottom();

	public:
		void Add(const std::string &s, const Font &font, MR_UInt8 color);
		void Clear();

		void Render(MR_2DViewPort *vp, int x, int y);

	private:
		typedef std::deque<StaticText*> lines_t;
		lines_t lines;
		unsigned int pos;
		unsigned int num;
		int height;

		static const int MAX_LINES = 100;
};

class HighConsole::Input : public Control::InputHandler
{
	typedef Control::InputHandler SUPER;
	public:
		Input(HighConsole *cons) : SUPER(), cons(cons) { }
		virtual ~Input() { }

		void Detach() { cons = NULL; }

	public:
		virtual bool KeyPressed(OIS::KeyCode kc, unsigned int text);
		virtual bool KeyReleased(OIS::KeyCode kc, unsigned int text);

	private:
		HighConsole *cons;
};

HighConsole::HighConsole(Script::Core *scripting, GameApp *gameApp,
                         GamePeer *gamePeer, SessionPeerPtr sessionPeer) :
	SUPER(scripting), gameApp(gameApp), gamePeer(gamePeer), sessionPeer(sessionPeer),
	visible(false), helpVisible(false), cursorOn(true), cursorTick(0)
{
	vp = new MR_2DViewPort();

	logFont = new Font("Courier New", 16);

	commandPrompt = new StaticText(COMMAND_PROMPT, *logFont, 0x0a, StaticText::EFFECT_SHADOW);
	continuePrompt = new StaticText(CONTINUE_PROMPT, *logFont, 0x0a, StaticText::EFFECT_SHADOW);
	cursor = new StaticText("_", *logFont, 0x0e, StaticText::EFFECT_SHADOW);

	charWidth = commandPrompt->GetWidth() / COMMAND_PROMPT.length();
	consoleWidth = 800;  // Will be corrected on first rendered frame.

	submitBuffer.reserve(1024);
	historyBuffer.reserve(1024);
	commandLine.reserve(1024);
	commandLineDisplay = new StaticText("", *logFont, 0x0a, StaticText::EFFECT_SHADOW);

	logLines = new LogLines();
	helpLines = new LogLines();

	// Introductory text for the console log.
	Config *cfg = cfg->GetInstance();
	logLines->Add(PACKAGE_NAME " version " + cfg->GetVersion(),
		Font("Arial", 20, true), 0x0a);

	Script::Core *env = GetScripting();
	std::string intro = env->GetVersionString();
	intro += " :: Console active.";
	logLines->Add(intro, *logFont, 0x0e);

	logLines->Add("Available global objects: game, session", *logFont, 0x0e);

	std::string helpInstructions = boost::str(boost::format(
		_("For help on a class or method, call the %s method: %s")) %
		"help()" % "game:help(); game:help(\"on_init\")");

	helpLines->Add(helpInstructions, *logFont, 0x0e);
	logLines->Add(helpInstructions, *logFont, 0x0e);

	Control::Controller *controller = gameApp->GetController();

	// The heading for the console.
	Font titleFont("Arial", 12);
	consoleTitle = new StaticText(_("Console"), titleFont, 0x0e);
	std::string consoleControlsText = boost::str(
		boost::format("%s [%s/%s]  %s [%s]") %
		_("Scroll") % controller->toString(SCROLL_UP_KEY) % controller->toString(SCROLL_DOWN_KEY) %
		_("Hide") % controller->toString(cfg->ui.console)
		);
	consoleControls = new StaticText(consoleControlsText, titleFont, 0x0e);

	// The heading for the help window.
	helpTitle = new StaticText(_("Help"), titleFont, 0x0e);
	std::string helpControlsText = boost::str(
		boost::format("%s [%s]") %
		_("Hide") % controller->toString(HELP_KEY)
		);
	helpControls = new StaticText(helpControlsText, titleFont, 0x0e);

	input = boost::make_shared<Input>(this);
}

HighConsole::~HighConsole()
{
	input->Detach();

	delete helpControls;
	delete helpTitle;
	delete consoleControls;
	delete consoleTitle;

	delete helpLines;
	delete logLines;

	delete cursor;
	delete continuePrompt;
	delete commandPrompt;
	delete commandLineDisplay;

	delete logFont;

	delete vp;
}

void HighConsole::InitEnv()
{
	using namespace luabind;

	SUPER::InitEnv();

	lua_State *L = GetScripting()->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	object env(from_stack(L, -1));
	env["game"] = gamePeer;
	env["session"] = sessionPeer;
}

void HighConsole::Advance(Util::OS::timestamp_t tick)
{
	// Cursor visibility is based on the last character typed
	// (so that the cursor stays visible while typing).
	cursorOn = (OS::TimeDiff(tick, cursorTick) % 1000) < 500;

	if (submitBuffer.empty()) return;

	std::string history;
	std::string chunk;
	{
		// See OnChar() for why we're careful about this.
		boost::lock_guard<boost::mutex> lock(submitBufferMutex);
		history.swap(historyBuffer);
		chunk.swap(submitBuffer);
	}
	
	// Add the history lines to the log.
	// We assume that each line in the log terminates with "\n".
	std::string line;
	line.reserve(1024);
	for (std::string::iterator iter = history.begin();
		iter != history.end(); ++iter)
	{
		if (*iter == '\n') {
			logLines->Add(line, *logFont, 0x0a);
			line.clear();
		}
		else {
			line += *iter;
		}
	}

	SubmitChunk(chunk);
}

void HighConsole::Clear()
{
	logLines->Clear();
}

/**
 * Add a log entry, pre-processing the string for display purposes.
 * @param lines The log lines.
 * @param s The log string.  We assume that the trailing "\n" has been stripped.
 * @param color The color.
 */
void HighConsole::AddLogEntry(LogLines *lines, const std::string &s, MR_UInt8 color)
{
	std::string buf;
	buf.reserve(1024);
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
				lines->Add(buf, *logFont, color);
				buf.clear();
				break;

			default:
				if (c >= 32 && c < 127) {
					// Line wrap if necessary.
					if ((buf.length() + 1) * charWidth > consoleWidth) {
						lines->Add(buf, *logFont, color);
						buf.clear();
					}
					buf += c;
				}
		}
	}
	lines->Add(buf, *logFont, color);
}

void HighConsole::LogInfo(const std::string &s)
{
	AddLogEntry(logLines, s, 0x0a);
}

void HighConsole::LogError(const std::string &s)
{
	AddLogEntry(logLines, s, 0x23);
}

void HighConsole::ToggleVisible()
{
	if (visible) {
		gameApp->GetController()->LeaveControlLayer();
		visible = false;
	}
	else {
		gameApp->GetController()->EnterControlLayer(input);
		visible = true;
	}
}

/**
 * Handle a character keypress.
 * Assume that the keypress is always consumed.
 * @param c The character.
 */
void HighConsole::OnChar(char c)
{
	cursorTick = OS::Time();

	switch (c) {
		case 8:   // Backspace.
		case 127: // DEL.
			if (commandLine.length() > 0) {
				commandLine.resize(commandLine.length() - 1);
				//TODO: Update wrapped version of line.
			}
			break;
		case 13:  // CR.
			commandLine += '\n';
			{
				// We assume that OnChar() will probably be called on one
				// thread while Advance() is called on another.
				boost::lock_guard<boost::mutex> lock(submitBufferMutex);
				historyBuffer +=
					(GetInputState() == ISTATE_COMMAND) ?
					COMMAND_PROMPT :
					CONTINUE_PROMPT;
				historyBuffer += commandLine;
				submitBuffer += commandLine;
			}
			commandLine.clear();
			break;
		default:
			if (c >= 32 && c <= 126) {
				commandLine += c;
				//TODO: Update wrapped version of line.
			}
	}
}

/**
 * Renders the console.
 * @param dest The destination viewport (may not be @c NULL).
 */
void HighConsole::Render(MR_VideoBuffer *dest)
{
	if (!visible) return;

	vp->Setup(dest, 0, 0, dest->GetXRes(), dest->GetYRes());

	RenderConsole();
	if (helpVisible) RenderHelp();
}

void HighConsole::RenderConsole()
{
	const int viewHeight = vp->GetYRes();
	const int viewWidth = vp->GetXRes();

	consoleWidth = viewWidth - (PADDING_LEFT * 2);

	// Prepare the command-line.
	// Select prompt based on state.
	const StaticText *prompt =
		(GetInputState() == ISTATE_COMMAND) ?
		commandPrompt :
		continuePrompt;
	commandLineDisplay->SetText(commandLine);

	// Calculate the total height of the console (for background).
	int commandLineHeight =
		std::max(prompt->GetHeight(),
			std::max(commandLineDisplay->GetHeight(), cursor->GetHeight()));
	int totalHeight =
		PADDING_TOP +
		logLines->GetHeight() +
		commandLineHeight +
		PADDING_BOTTOM;

	// Draw the background.
	for (int ly = viewHeight - totalHeight - 1;
		ly < viewHeight - commandLineHeight - PADDING_BOTTOM; ++ly)
	{
		vp->DrawHorizontalLine(ly, 0, viewWidth - 1, 0x18);
	}
	for (int i = 0, ly = viewHeight - commandLineHeight - PADDING_BOTTOM;
		ly < viewHeight; ++i, ++ly)
	{
		vp->DrawHorizontalLine(ly, 0, viewWidth - 1, 0x1a - ((i >= 5) ? 5 : i));
	}

	int x = 0;
	int y = viewHeight - totalHeight + PADDING_TOP;

	// Render the title.
	RenderHeading(consoleTitle, consoleControls,
		y - PADDING_TOP - TITLE_PADDING_BOTTOM - consoleTitle->GetHeight() - TITLE_PADDING_TOP,
		0x18);

	// Render the log lines.
	logLines->Render(vp, PADDING_LEFT, y);
	y += logLines->GetHeight() + (PADDING_BOTTOM / 2);

	// Render the command line.
	x = PADDING_LEFT;
	prompt->Blt(x, y, vp);
	x += prompt->GetWidth();
	commandLineDisplay->Blt(x, y, vp);
	if (cursorOn) {
		x += commandLineDisplay->GetWidth();
		cursor->Blt(x, y, vp);
	}
}

void HighConsole::RenderHelp()
{
	const int viewHeight = vp->GetYRes();
	const int viewWidth = vp->GetXRes();

	int totalHeight = PADDING_TOP + helpLines->GetHeight() + PADDING_BOTTOM;

	// Draw the background.
	for (int ly = 0; ly < totalHeight; ++ly) {
		vp->DrawHorizontalLine(ly, 0, viewWidth - 1, 0x18);
	}

	// Render the title.
	RenderHeading(helpTitle, helpControls, totalHeight, 0x18, true);

	helpLines->Render(vp, PADDING_LEFT, PADDING_TOP);
}

void HighConsole::RenderHeading(const VideoServices::StaticText *title,
                                const VideoServices::StaticText *controls,
                                int y, MR_UInt8 bgColor, bool reversed)
{
	const int viewWidth = vp->GetXRes();

	// We assume that both bits of text are the same height.
	const int totalHeight = TITLE_PADDING_TOP + title->GetHeight() + TITLE_PADDING_BOTTOM;
	const int titleTotalWidth = PADDING_LEFT + title->GetWidth() + PADDING_RIGHT;
	const int controlsTotalWidth = PADDING_LEFT + controls->GetWidth() + PADDING_RIGHT;

	for (int i = 0; i < totalHeight; ++i) {
		int x2 = titleTotalWidth + (reversed ? (totalHeight - i - 1) : i);
		vp->DrawHorizontalLine(y + i, 0, x2, bgColor);
	}
	for (int i = 0; i < totalHeight; ++i) {
		int x1 = viewWidth - controlsTotalWidth -
			(reversed ? (totalHeight - i - 1) : i);
		vp->DrawHorizontalLine(y + i, x1, viewWidth - 1, bgColor);
	}

	title->Blt(PADDING_LEFT, y + TITLE_PADDING_TOP, vp);
	controls->Blt(viewWidth - PADDING_RIGHT - controls->GetWidth(),
		y + TITLE_PADDING_TOP, vp);
}

void HighConsole::HelpClass(const Script::Help::Class &cls)
{
	using Script::Help::Class;
	using Script::Help::MethodPtr;

	AddLogEntry(helpLines,
		boost::str(boost::format(_("Methods for class %s:")) % cls.GetName()),
		0x0a);

	std::string s;
	s.reserve(1024);
	BOOST_FOREACH(const Class::methods_t::value_type &ent, cls.GetMethods()) {
		MethodPtr method = ent.second;
		s.clear();
		s += "  ";
		s += method->GetName();
		
		const std::string &brief = method->GetBrief();
		if (!brief.empty()) {
			s += " - ";
			s += brief;
		}

		AddLogEntry(helpLines, s, 0x0a);
	}

	helpVisible = true;
}

void HighConsole::HelpMethod(const Script::Help::Class &cls, const Script::Help::Method &method)
{
	using Script::Help::Method;
	
	AddLogEntry(helpLines, "---", 0x0a);

	BOOST_FOREACH(const std::string &s, method.GetSigs()) {
		AddLogEntry(helpLines, s, 0x0a);
	}

	AddLogEntry(helpLines, method.GetBrief(), 0x0e);
	const std::string &desc = method.GetDesc();
	if (!desc.empty()) {
		AddLogEntry(helpLines, method.GetDesc(), 0x0e);
	}

	helpVisible = true;
}

// LogLines ////////////////////////////////////////////////////////////////////

HighConsole::LogLines::LogLines() :
	lines(), pos(0), num(10), height(0)
{
}

HighConsole::LogLines::~LogLines()
{
	Clear();
}

void HighConsole::LogLines::RecalcHeight()
{
	height = 0;
	unsigned int i = 0;
	for (lines_t::iterator iter = lines.begin() + pos;
		iter != lines.end() && i < num;
		++iter, ++i)
	{
		height += (*iter)->GetHeight();
	}
}

void HighConsole::LogLines::ScrollTop()
{
	if (pos != 0) {
		pos = 0;
		RecalcHeight();
	}
}

void HighConsole::LogLines::Scroll(int i)
{
	if (i < 0) {
		if ((unsigned int)-i >= pos) {
			ScrollTop();
		}
		else {
			pos += i;
			RecalcHeight();
		}
	}
	else if (lines.size() > num) {
		if (pos + i > lines.size() - num) {
			ScrollBottom();
		}
		else {
			pos += i;
			RecalcHeight();
		}
	}
}

void HighConsole::LogLines::ScrollBottom()
{
	pos = (lines.size() <= num) ? 0 : (lines.size() - num);
	RecalcHeight();
}

void HighConsole::LogLines::Add(const std::string &s, const Font &font, MR_UInt8 color)
{
	// Remove the top line if we're full.
	if (lines.size() == MAX_LINES) {
		StaticText *line = lines.front();
		delete line;
		lines.pop_front();
	}

	// Add the new line.
	// If the line is blank, then use a single space, since a blank string will
	// result in the StaticText not being full height.
	StaticText *line = new StaticText(s.empty() ? " " : s, font, color);
	lines.push_back(line);

	ScrollBottom();
}

void HighConsole::LogLines::Clear()
{
	for (lines_t::iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		delete *iter;
	}
	lines.clear();
	pos = 0;
	height = 0;
}

void HighConsole::LogLines::Render(MR_2DViewPort *vp, int x, int y)
{
	unsigned int i = 0;
	for (lines_t::iterator iter = lines.begin() + pos;
		iter != lines.end() && i < num;
		++iter, ++i)
	{
		const StaticText *line = *iter;
		line->Blt(x, y, vp);
		y += line->GetHeight();
	}
}

// Input ///////////////////////////////////////////////////////////////////////

bool HighConsole::Input::KeyPressed(OIS::KeyCode kc, unsigned int text)
{
	if (cons == NULL || !cons->IsVisible()) return true;

	if (Config::GetInstance()->ui.console.IsKey(kc)) {
		cons->ToggleVisible();
	}
	else if (HELP_KEY.IsKey(kc)) {
		cons->helpVisible = !cons->helpVisible;
	}
	else if (SCROLL_TOP_KEY.IsKey(kc)) {
		cons->logLines->ScrollTop();
	}
	else if (SCROLL_UP_KEY.IsKey(kc)) {
		cons->logLines->Scroll(-SCROLL_SPEED);
	}
	else if (SCROLL_DOWN_KEY.IsKey(kc)) {
		cons->logLines->Scroll(SCROLL_SPEED);
	}
	else if (SCROLL_BOTTOM_KEY.IsKey(kc)) {
		cons->logLines->ScrollBottom();
	}
	else {
		/*
		if (text) {
			if (text >= 32 && text < 127) {
				OutputDebugString(boost::str(boost::format("Text key: %c (%d)\n") % (char)text % text).c_str());
			}
			else {
				OutputDebugString(boost::str(boost::format("Text key: (%d)\n") % text).c_str());
			}
		}
		*/
	}

	return true;
}

bool HighConsole::Input::KeyReleased(OIS::KeyCode kc, unsigned int text)
{
	return true;
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
