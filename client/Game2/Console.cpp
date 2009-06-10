
// Console.cpp
// Base class for debug consoles.
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

#include "../../engine/Script/Env.h"

#include "Console.h"

using namespace HoverRace;
using namespace HoverRace::Script;

namespace HoverRace {

// Stream bits to redirect output to the console log.

class Console::LogStreamBuf : public std::stringbuf
{
	typedef std::stringbuf SUPER;
	public:
		LogStreamBuf(Console *console);
		virtual ~LogStreamBuf();

	protected:
		virtual int sync();

	private:
		Console *console;
};

class Console::LogStream : public std::ostream
{
	typedef std::ostream SUPER;
	public:
		LogStream(Console *console) :
			SUPER(new LogStreamBuf(console)) { }
		virtual ~LogStream() { delete rdbuf(); }
};

}

Console::Console() :
	inputState(ISTATE_COMMAND)
{
	chunk.reserve(1024);

	scripting = new Env();
	boost::shared_ptr<std::ostream> logStream(new LogStream(this));
	scripting->setOutput(logStream);
}

Console::~Console()
{
	delete scripting;
}

/**
 * Submit a chunk to the script engine.
 * @param s The chunk.
 */
void Console::SubmitChunk(const std::string &s)
{
	chunk += s;

	try {
		scripting->Execute(chunk);
	}
	catch (const IncompleteExn&) {
		SetInputState(ISTATE_CONTINUE);
		return;
	}
	catch (const ScriptExn &exn) {
		LogError(exn.GetMessage());
	}

	chunk.clear();
	SetInputState(ISTATE_COMMAND);
}

void Console::SetInputState(inputState_t newState)
{
	inputState = newState;
}

Console::inputState_t Console::GetInputState() const
{
	return inputState;
}

Console::LogStreamBuf::LogStreamBuf(Console *console) :
	SUPER(), console(console)
{
}

Console::LogStreamBuf::~LogStreamBuf()
{
	sync();
}

int Console::LogStreamBuf::sync()
{
	std::string s = str();

	// Write each line to the log.
	std::string line;
	line.reserve(1024);
	for (std::string::iterator iter = s.begin();
		iter != s.end(); ++iter)
	{
		if (*iter == '\n') {
			console->LogInfo(line);
			line.clear();
		}
		else {
			line += *iter;
		}
	}
	if (!line.empty()) console->LogInfo(line);

	// Reset the string buffer.
	str(std::string());

	return 0;
}
