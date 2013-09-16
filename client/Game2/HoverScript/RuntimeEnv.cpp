
// RuntimeEnv.cpp
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

#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Log.h"
#include "../../../engine/Util/Str.h"

#include "RulebookEnv.h"

using namespace HoverRace::Script;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

// Stream bits to redirect output to the console log.

class RuntimeEnv::LogStreamBuf : public std::stringbuf /*{{{*/
{
	typedef std::stringbuf SUPER;
	public:
		LogStreamBuf() : SUPER() { }
		virtual ~LogStreamBuf() { sync(); }

	protected:
		virtual int sync();
}; //}}}

class RuntimeEnv::LogStream : public std::ostream /*{{{*/
{
	typedef std::ostream SUPER;
	public:
		LogStream() :
			SUPER(new LogStreamBuf()) { }
		virtual ~LogStream() { delete rdbuf(); }
}; //}}}

RuntimeEnv::RuntimeEnv(Script::Core *scripting) :
	SUPER(scripting)
{
	//outHandle = scripting->AddOutput(std::make_shared<LogStream>());
}

RuntimeEnv::~RuntimeEnv()
{
	//GetScripting()->RemoveOutput(outHandle);
}

//{{{ RuntimeEnv::LogStreamBuf /////////////////////////////////////////////////

int RuntimeEnv::LogStreamBuf::sync()
{
	std::string s = str();

	// Write each line to the log.
	std::string line;
	line.reserve(1024);
	for (std::string::iterator iter = s.begin();
		iter != s.end(); ++iter)
	{
		if (*iter == '\n') {
			Log::Info("%s", line.c_str());
			line.clear();
		}
		else {
			line += *iter;
		}
	}
	if (!line.empty()) Log::Info("%s", line.c_str());

	// Reset the string buffer.
	str(std::string());

	return 0;
}

//}}} RuntimeEnv::LogStreamBuf

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

