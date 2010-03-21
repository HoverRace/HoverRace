
// Handlers.h
// Tracks event handlers registered in scripts.
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

#pragma once

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace luabind {
	class object;
}

namespace HoverRace {
namespace Script {

class Core;

/**
 * Tracks event handlers registered in scripts.
 * @author Michael Imamura
 */
class MR_DllDeclare Handlers
{
	private:
		Handlers() { }
	public:
		Handlers(Core *scripting);
		virtual ~Handlers();

	public:
		void CallHandlers() const;

		void AddHandler(const luabind::object &fn);

	private:
		Core *scripting;
		int seq;
		int ref;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
