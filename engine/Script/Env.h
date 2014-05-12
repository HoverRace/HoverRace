
// Env.h
//
// Copyright (c) 2010, 2014 Michael Imamura.
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

#include "../Util/OS.h"

#include "Core.h"

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
		namespace Help {
			class HelpHandler;
		}
	}
}

namespace HoverRace {
namespace Script {

/**
 * A script environment.
 * @author Michael Imamura
 */
class MR_DllDeclare Env
{
	private:
		Env() { }
	public:
		Env(Core *scripting);
		virtual ~Env();

	protected:
		Core *GetScripting() const { return scripting; }

		void LogScriptError(const Script::ScriptExn &ex);

		virtual void InitEnv() = 0;
		void CopyGlobals();

		void SetHelpHandler(Help::HelpHandler *helpHandler);

	private:
		void SetupEnv();

	protected:
		/**
		 * Execute a chunk of code in the current environment.
		 * @param chunk The code to execute.
		 * @throw IncompleteExn If the code does not complete a statement; i.e.,
		 *                      expecting more tokens.  Callers can catch this
		 *                      to keep reading more data to finish the
		 *                      statement.
		 * @throw ScriptExn The code either failed to compile or signaled an
		 *                  error while executing.
		 */
		void Execute(const Core::Chunk &chunk)
		{
			Core::StackRestore sr(scripting->GetState());

			// May throw ScriptExn or IncompleteExn, in which case the stack
			// will be unchanged.
			scripting->Compile(chunk);

			SetupEnv();

			//TODO: Use custom return value handler.
			// May throw ScriptExn, but the function on the stack will be
			// consumed anyway.
			scripting->Invoke(0, helpHandler);
		}

	private:
		Core::Chunk LoadChunkFromFile(const Util::OS::path_t &filename);

	public:
		/**
		 * Execute a script from a file.
		 *
		 * If there is an error executing the script, then the error message will be
		 * written to the error log and the function will return @c false.
		 *
		 * @param filename The script filename (must be an absolute path).
		 * @return @c true if the script executed successfully,
		 *         @c false if there was an error.
		 */
		bool RunScript(const Util::OS::path_t &filename)
		{
			try {
				Execute(LoadChunkFromFile(filename));
				return true;
			}
			catch (Script::ScriptExn &ex) {
				LogScriptError(ex);
				return false;
			}
		}

	private:
		Core *scripting;
		bool initialized;
		int envRef;
		Help::HelpHandler *helpHandler;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
