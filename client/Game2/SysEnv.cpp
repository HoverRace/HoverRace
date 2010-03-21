
// SysEnv.cpp
// The global system environment.
//
// Copyright (c) 2010 Michael Imamura.
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

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

#include <luabind/luabind.hpp>

#include "GamePeer.h"

#include "SysEnv.h"

namespace fs = boost::filesystem;

namespace {
	class LogStreamBuf : public std::stringbuf
	{
		typedef std::stringbuf SUPER;
		public:
			LogStreamBuf() { }
			virtual ~LogStreamBuf() { sync(); }

		protected:
			virtual int sync()
			{
				std::string s = str();

#				ifdef _WIN32
					OutputDebugString(s.c_str());
#				else
					std::cout << s << std::flush;
#				endif

				str(std::string());
				return 0;
			}
	};

	class LogStream : public std::ostream
	{
		typedef std::ostream SUPER;
		public:
			LogStream() : SUPER(new LogStreamBuf()) { }
			virtual ~LogStream() { delete rdbuf(); }
	};
}

namespace HoverRace {
namespace Client {

SysEnv::SysEnv(Script::Core *scripting, GamePeer *gamePeer) :
	SUPER(scripting), gamePeer(gamePeer),
	outHandle(scripting->AddOutput(boost::make_shared<LogStream>()))
{
}

SysEnv::~SysEnv()
{
	GetScripting()->RemoveOutput(outHandle);
}

void SysEnv::InitEnv()
{
	using namespace luabind;

	Script::Core *scripting = GetScripting();
	lua_State *L = scripting->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	object env(from_stack(L, -1));
	env["game"] = gamePeer;
}

void SysEnv::LogInfo(const std::string &s)
{
#	ifdef _WIN32
		OutputDebugString(s.c_str());
		OutputDebugString("\n");
#	else
		std::cout << s << std::endl;
#	endif
}

void SysEnv::LogError(const std::string &s)
{
#	ifdef _WIN32
		OutputDebugString(s.c_str());
		OutputDebugString("\n");
#	else
		std::cerr << s << std::endl;
#	endif
}

/**
 * Execute a script from a file.
 * @param filename The script filename (must be an absolute path).
 */
void SysEnv::RunScript(const std::string &filename)
{
	fs::path scriptPath = fs::system_complete(fs::path(filename));

	if (!fs::exists(scriptPath)) {
		LogError(boost::str(
			boost::format("Init script file not found: %s (interpreted as %s)") %
			filename % scriptPath));
		return;
	}
	
	std::string chunkName("@");
	chunkName += scriptPath.filename();

	// Read and submit the whole script at once.
	fs::ifstream ifs(scriptPath, std::ios_base::in);
	std::string ris((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	try {
		Execute(ris, chunkName);
	}
	catch (Script::ScriptExn &ex) {
		LogError(ex.what());
	}
}

}  // namespace Client
}  // namespace HoverRace
