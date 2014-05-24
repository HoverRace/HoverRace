
// Wrapper.h
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

#include "../Util/Log.h"

namespace HoverRace {
namespace Script {

/**
 * Base class for Luabind class wrappers.
 *
 * This extends Luabind's @c wrap_base with a version of call() that handles
 * script errors by logging the error.
 *
 * @author Michael Imamura
 */
class Wrapper : public luabind::wrap_base
{
	typedef luabind::wrap_base SUPER;
	public:
		Wrapper() : SUPER() { }
		virtual ~Wrapper() { }

	protected:
		void HandleError(luabind::error &ex)
		{
			lua_State *L = ex.state();
			std::string msg(lua_tostring(L, -1));
			lua_pop(L, 1);
			Util::Log::Error("%s", msg.c_str());
		}

		//TODO: Support other returns than void.
		template<typename Ret, class... Params>
		void pcall(Params... params)
		{
			try {
				call<void>(params...);
			}
			catch (luabind::error &ex) {
				HandleError(ex);
			}
		}

};

}  // namespace Script
}  // namespace HoverRace
