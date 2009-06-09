
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

#include "../../engine/Util/Config.h"
#include "../../engine/VideoServices/2DViewPort.h"
#include "../../engine/VideoServices/StaticText.h"

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
}

HighConsole::HighConsole() :
	SUPER(), cursorOn(true), cursorTick(0)
{
	logFont = new Font("Courier New", 16);

	commandPrompt = new StaticText(COMMAND_PROMPT, *logFont, 0x0a, StaticText::EFFECT_SHADOW);
	continuePrompt = new StaticText(CONTINUE_PROMPT, *logFont, 0x0a, StaticText::EFFECT_SHADOW);
	cursor = new StaticText("_", *logFont, 0x0e, StaticText::EFFECT_SHADOW);

	submitBuffer.reserve(1024);
	historyBuffer.reserve(1024);
	commandLine.reserve(1024);
	commandLineDisplay = new StaticText("", *logFont, 0x0a, StaticText::EFFECT_SHADOW);

	logLines = new LogLines();

	// Introductory text for the console log.
	Config *cfg = cfg->GetInstance();
	logLines->Add(PACKAGE_NAME " version " + cfg->GetVersion(),
		Font("Arial", 20, true), 0x0a);
	logLines->Add("Console active.", *logFont, 0x0e);
}

HighConsole::~HighConsole()
{
	delete logLines;

	delete cursor;
	delete continuePrompt;
	delete commandPrompt;
	delete commandLineDisplay;

	delete logFont;
}

void HighConsole::Advance(OS::timestamp_t tick)
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

	/*
	OutputDebugString("Process: ");
	OutputDebugString(chunk.c_str());
	*/
	SubmitChunk(chunk);
}

void HighConsole::LogInfo(const std::string &s)
{
	logLines->Add(s, *logFont, 0x0a);
}

void HighConsole::LogError(const std::string &s)
{
	logLines->Add(s, *logFont, 0x23);
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
			if (commandLine.length() > 0)
				commandLine.resize(commandLine.length() - 1);
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
			if (c >= 32 && c <= 126)
				commandLine += c;
	}
}

/**
 * Renders the console.
 * @param vp The destination viewport (may not be @c NULL).
 */
void HighConsole::Render(MR_2DViewPort *vp)
{
	const int viewHeight = vp->GetYRes();
	const int viewWidth = vp->GetXRes();

	// Prepare the command-line.
	//TODO: Select prompt based on state.
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
	for (int ly = viewHeight - commandLineHeight - PADDING_BOTTOM;
		ly < viewHeight; ++ly)
	{
		vp->DrawHorizontalLine(ly, 0, viewWidth - 1, 0x1a);
	}

	int x = 0;
	int y = viewHeight - totalHeight + PADDING_TOP;

	// Render the log lines.
	logLines->Render(vp, PADDING_LEFT, y);
	y += logLines->GetHeight();

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
}

void HighConsole::LogLines::Render(MR_2DViewPort *vp, int x, int y)
{
	for (lines_t::iterator iter = lines.begin(); iter != lines.end(); ++iter) {
		const StaticText *line = *iter;
		line->Blt(x, y, vp);
		y += line->GetHeight();
	}
}
