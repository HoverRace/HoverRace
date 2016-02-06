
// Core.h
//
// Copyright (c) 2009, 2010, 2014, 2016 Michael Imamura.
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

#include <luabind/object.hpp>

#include "Lua.h"
#include "ScriptExn.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
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
			class Class;
			class HelpHandler;
		}
	}
}

namespace HoverRace {
namespace Script {

/// Exception to signal that the current chunk is incomplete.
class IncompleteExn : public ScriptExn
{
	using SUPER = ScriptExn;

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
	Core(const Core &core) = delete;
	virtual ~Core();

	Core &operator=(const Core&) = delete;

public:
	lua_State *GetState() const { return state; }
	virtual Core *Reset();
	void ActivateSandbox();

private:
	static int ErrorFunc(lua_State *L);

private:
	using outs_t = std::list<std::shared_ptr<std::ostream>>;
public:
	using OutHandle = outs_t::iterator;
	OutHandle AddOutput(std::shared_ptr<std::ostream> out);
	void RemoveOutput(const OutHandle &handle);

	std::string GetVersionString() const;

public:
	struct Chunk
	{
		/**
		 * Constructor.
		 * @param src The source code of the chunk.
		 * @param name Optional name for the chunk.
		 *             This name is used in error messages.
		 *             Prefix the name with @c "=" to use the name verbatim,
		 *             without decoration, in error messages.
		 */
		Chunk(const std::string &src,
			const std::string &name = DEFAULT_CHUNK_NAME) :
			src(src), name(name) { }
		Chunk(const Chunk &o) = default;
		Chunk(Chunk &&o) = default;

		Chunk &operator=(const Chunk &o) = default;
		Chunk &operator=(Chunk &&o) = default;

		std::string src;
		std::string name;
	};

public:
	/// Ensures that the Lua stack is unchanged in a block.
	struct MR_DllDeclare StackRestore
	{
		StackRestore(lua_State *state);
		~StackRestore();

		lua_State *state;
		int initStack;
	};

private:
	static void PrintFromStack(lua_State *state, int n);
public:
	/// Return policy that leaves the return values on the stack.
	struct PassReturn
	{
		int operator()(lua_State*, int n) const
		{
			return n;
		}
	};

	/**
	 * Return policy that calls Lua's print() function on each value.
	 * All return values are consumed.
	 */
	struct PrintReturn
	{
		int operator()(lua_State *state, int n) const
		{
			PrintFromStack(state, n);
			return 0;
		}
	};

public:
	void Print(const std::string &s);

	static const std::string DEFAULT_CHUNK_NAME;
	void Compile(const Chunk &chunk);
	int Call(int numParams = 0, Help::HelpHandler *helpHandler = nullptr);

	/**
	 * Pop a function off the stack and execute it, printing any return values.
	 * @param numParams The number of params being passed to the function.
	 * @param helpHandler Optional callback for when a script requests API help.
	 * @param rp Optional policy to handle return values.
	 * @return The number of return values remaining on the stack.
	 * @throw ScriptExn The code signaled an error while executing.
	 */
	template<class ReturnPolicy = PrintReturn>
	int Invoke(int numParams = 0, Help::HelpHandler *helpHandler = nullptr,
		ReturnPolicy rp = ReturnPolicy())
	{
		return rp(state, Call(numParams, helpHandler));
	}

	/// @deprecated Use Invoke instead.
	void CallAndPrint(int numParams = 0, Help::HelpHandler *helpHandler = nullptr)
	{
		Invoke(numParams, helpHandler);
	}

	/**
	 * Safely compile and execute a chunk of code.
	 *
	 * This is a convenience function.
	 *
	 * @param chunk The code to execute.
	 * @param helpHandler Optional callback for when a script requests API help.
	 * @param rp Optional policy to handle return values.
	 * @throw IncompleteExn If the code does not complete a statement; i.e.,
	 *                      expecting more tokens.  Callers can catch this
	 *                      to keep reading more data to finish the statement.
	 * @throw ScriptExn The code either failed to compile or signaled an error
	 *                  while executing.
	 */
	template<class ReturnPolicy = PrintReturn>
	void Execute(const Chunk &chunk, Help::HelpHandler *helpHandler = nullptr,
		ReturnPolicy rp = ReturnPolicy())
	{
		// Explicitly throw away any return values leftover from the
		// return policy so that the stack is exactly how we began.
		StackRestore sr(state);

		Compile(chunk);
		Invoke(0, helpHandler, rp);
	}

	void PrintStack();

	void ReqHelp(const std::string &className);
	void ReqHelp(const std::string &className, const std::string &methodName);

public:
	std::unique_ptr<luabind::object> NIL;
private:
	void LoadClassHelp(const std::string &className);
	static std::string PopError(lua_State *state);

	static int LPrint(lua_State *state);
	static int LSandboxedFunction(lua_State *state);

private:
	outs_t outs;
	lua_State *state;
	Help::HelpHandler *curHelpHandler;
	using helpClasses_t = std::map<
		const std::string, std::shared_ptr<Help::Class>>;
	helpClasses_t helpClasses;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
