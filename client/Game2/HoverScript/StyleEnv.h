
// StyleEnv.h
//
// Copyright (c) 2014 Michael Imamura.
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

#pragma once

#include "RuntimeEnv.h"

namespace HoverRace {
	namespace Client {
		class Rulebook;
	}
	namespace Display {
		class Display;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Limited environment for stylesheets.
 * @author Michael Imamura
 */
class StyleEnv : protected RuntimeEnv {
	using SUPER = RuntimeEnv;

public:
	StyleEnv(Script::Core *scripting, Display::Display &display,
		const Util::OS::path_t &basePath);
	virtual ~StyleEnv() { }

public:
	static void Register(Script::Core *scripting);

protected:
	virtual void InitEnv();

public:
	bool RunStylesheet();

private:
	Display::Display &display;
	Util::OS::path_t basePath;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
