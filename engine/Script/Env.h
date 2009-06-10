
// Env.h
// Header for the scripting engine environment.
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

#pragma once

#include <queue>

#include <lua.hpp>

#include "ScriptExn.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Script {

/// Exception to signal that the current chunk is incomplete.
class IncompleteExn : public ScriptExn { };

/**
 * A script environment.
 * @author Michael Imamura
 */
class MR_DllDeclare Env
{
	public:
		Env();
		~Env();

	public:
		void setOutput(boost::shared_ptr<std::ostream> out);

	public:
		void Execute(const std::string &chunk);

	private:
		std::string PopError();

		static int LPrint(lua_State *state);

	private:
		boost::shared_ptr<std::ostream> out;
		lua_State *state;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
