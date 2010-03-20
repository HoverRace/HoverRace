
// Core.h
// Header for the scripting engine environment.
//
// Copyright (c) 2009, 2010 Michael Imamura.
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

#include <list>

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
class IncompleteExn : public ScriptExn
{
	typedef ScriptExn SUPER;
	public:
		IncompleteExn(const std::string &s) : SUPER(s) { }
};

/**
 * A script environment.
 * @author Michael Imamura
 */
class MR_DllDeclare Core
{
	public:
		Core();
		virtual ~Core();

	public:
		lua_State *GetState() const { return state; }
		virtual Core *Reset();
		void ActivateSandbox();

	private:
		typedef std::list<boost::shared_ptr<std::ostream> > outs_t;
	public:
		typedef outs_t::iterator OutHandle;
		OutHandle AddOutput(boost::shared_ptr<std::ostream> out);
		void RemoveOutput(const OutHandle &handle);

		std::string GetVersionString() const;

	public:
		void Print(const std::string &s);

		static const std::string DEFAULT_CHUNK_NAME;
		void Compile(const std::string &chunk, const std::string &name=DEFAULT_CHUNK_NAME);
		void CallAndPrint();

		void Execute(const std::string &chunk);

		void PrintStack();

	private:
		std::string PopError();

		static int LPrint(lua_State *state);
		static int LSandboxedFunction(lua_State *state);

	private:
		outs_t outs;
		lua_State *state;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
