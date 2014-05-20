
// WrapperFactory.h
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

#include "Core.h"
#include "RegistryRef.h"
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
		class Core;
	}
}

namespace HoverRace {
namespace Script {

/**
 * A reference to a Lua function or userdata that wraps a native object.
 * @author Michael Imamura
 */
template<class Inside, class Outside>
class WrapperFactory
{
	public:
		WrapperFactory(Script::Core *scripting) :
			scripting(scripting), ref(scripting) { }
		WrapperFactory(const WrapperFactory&) = default;
		WrapperFactory(WrapperFactory&&) = default;

		WrapperFactory &operator=(const WrapperFactory&) = default;
		WrapperFactory &operator=(WrapperFactory&&) = default;

		WrapperFactory &operator=(const luabind::object &obj)
		{
			Set(obj);
			return *this;
		}

		void Set(const luabind::object &obj)
		{
			using namespace luabind;

			auto t = type(obj);
			if (t == LUA_TUSERDATA || t == LUA_TFUNCTION) {
				ref = obj;
			}
			else {
				throw ScriptExn("Expected a constructor or function.");
			}
		}

		/**
		 * Wrap a native object in the wrapper.
		 * @param inside The native object to wrap.
		 * @return The wrapped object, or @c nullptr if no wrapper factory has
		 *         been set.
		 */
		std::shared_ptr<Outside> operator()(std::shared_ptr<Inside> inside) const
		{
			using namespace luabind;

			if (!ref) {
				return std::shared_ptr<Outside>();
			}

			object obj(inside);
			std::shared_ptr<Outside> retv;

			ref.Push();
			obj.push();
			scripting->Invoke(1, nullptr, [&](lua_State *L, int num) {
				if (num < 1) {
					//TODO
				}
				else {
					try {
						retv = object_cast<std::shared_ptr<Outside>>(
							object(from_stack(L, -n)));
					}
					catch (cast_failed &ex)
					{
						//TODO
					}
				}
				lua_pop(L, num);
				return 0;
			});

			return retv;
		}

	private:
		Script::Core *scripting;
		RegistryRef ref;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
