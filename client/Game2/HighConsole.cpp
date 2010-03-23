
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

#include "../../engine/Script/Core.h"
#include "../../engine/Util/Config.h"
#include "../../engine/VideoServices/2DViewPort.h"
#include "../../engine/VideoServices/StaticText.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "GamePeer.h"
#include "SessionPeer.h"

#include "HighConsole.h"

using namespace HoverRace;
using HoverRace::Util::Config;
using HoverRace::Util::OS;
using HoverRace::VideoServices::Font;
using HoverRace::VideoServices::StaticText;

#ifdef max
#	undef max
#endif

namespace {
	static const std::string COMMAND_PROMPT(">> ");
	static const std::string CONTINUE_PROMPT(":> ");
}

namespace HoverRace {
namespace Client {

class HighConsole::LogLines
{
	public:
		LogLines();
		~LogLines();

	public:
		int GetHeight() const { return height; }

	public:
		void Add(const std::string &s, const Font &font, MR_UInt8 color);
		void Clear();

		void Render(MR_2DViewPort *vp, int x, int y);

	private:
		typedef std::deque<StaticText*> lines_t;
		lines_t lines;
		int height;

		static const int MAX_LINES = 10;
};

HighConsole::HighConsole(Script::Core *scripting, GamePeer *gamePeer, SessionPeerPtr sessionPeer) :
	SUPER(scripting), gamePeer(gamePeer), sessionPeer(sessionPeer),
	visible(false), cursorOn(true), cursorTick(0)
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

	// Introductory text for the console log.
	Config *cfg = cfg->GetInstance();
	logLines->Add(PACKAGE_NAME " version " + cfg->GetVersion(),
		Font("Arial", 20, true), 0x0a);

	Script::Core *env = GetScripting();
	std::string intro = env->GetVersionString();
	intro += " :: Console active.";
	logLines->Add(intro, *logFont, 0x0e);

	// The heading for the console.
	Font titleFont("Arial", 12);
	consoleTitle = new StaticText(_("Console"), titleFont, 0x0e);
	std::string consoleControlsText;
	consoleControlsText += _("Hide");
	consoleControlsText += " [~]";
	consoleControls = new StaticText(consoleControlsText, titleFont, 0x0e);;
}

HighConsole::~HighConsole()
{
	delete consoleControls;
	delete consoleTitle;

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
 * @param s The log string.  We assume that the trailing "\n" has been stripped.
 * @param color The color.
 */
void HighConsole::AddLogEntry(const std::string &s, MR_UInt8 color)
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
				logLines->Add(buf, *logFont, color);
				buf.clear();
				break;

			default:
				if (c >= 32 && c < 127) {
					// Line wrap if necessary.
					if ((buf.length() + 1) * charWidth > consoleWidth) {
						logLines->Add(buf, *logFont, color);
						buf.clear();
					}
					buf += c;
				}
		}
	}
	logLines->Add(buf, *logFont, color);
}

void HighConsole::LogInfo(const std::string &s)
{
	AddLogEntry(s, 0x0a);
}

void HighConsole::LogError(const std::string &s)
{
	AddLogEntry(s, 0x23);
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

void HighConsole::RenderHeading(const VideoServices::StaticText *title,
                                const VideoServices::StaticText *controls,
                                int y, MR_UInt8 bgColor)
{
	const int viewWidth = vp->GetXRes();

	// We assume that both bits of text are the same height.
	const int totalHeight = TITLE_PADDING_TOP + title->GetHeight() + TITLE_PADDING_BOTTOM;
	const int titleTotalWidth = PADDING_LEFT + title->GetWidth() + PADDING_RIGHT;
	const int controlsTotalWidth = PADDING_LEFT + controls->GetWidth() + PADDING_RIGHT;

	for (int i = 0; i < totalHeight; ++i) {
		vp->DrawHorizontalLine(y + i, 0, titleTotalWidth + i, bgColor);
	}
	for (int i = 0; i < totalHeight; ++i) {
		vp->DrawHorizontalLine(y + i, viewWidth - controlsTotalWidth - i, viewWidth - 1, bgColor);
	}

	title->Blt(PADDING_LEFT, y + TITLE_PADDING_TOP, vp);
	controls->Blt(viewWidth - PADDING_RIGHT - controls->GetWidth(),
		y + TITLE_PADDING_TOP, vp);
}

HighConsole::LogLines::LogLines() :
	lines(), height(0)
{
}

HighConsole::LogLines::~LogLines()
{
	Clear();
}

void HighConsole::LogLines::Add(const std::string &s, const Font &font, MR_UInt8 color)
{
	// Remove the top line if we're full.
	if (lines.size() == MAX_LINES) {
		StaticText *line = lines.front();
		height -= line->GetHeight();
		delete line;
		lines.pop_front();
	}

	// Add the new line.
	StaticText *line = new StaticText(s, font, color);
	height += line->GetHeight();
	lines.push_back(line);
}

void HighConsole::LogLines::Clear()
{
	for (lines_t::iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		delete *iter;
	}
	lines.clear();
	height = 0;
}

void HighConsole::LogLines::Render(MR_2DViewPort *vp, int x, int y)
{
	for (lines_t::iterator iter = lines.begin(); iter != lines.end(); ++iter) {
		const StaticText *line = *iter;
		line->Blt(x, y, vp);
		y += line->GetHeight();
	}
}

}  // namespace Client
}  // namespace HoverRace
