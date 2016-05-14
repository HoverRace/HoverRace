
// WrapperFactory.h
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "Core.h"
#include "RegistryRef.h"
#include "ScriptExn.h"

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

		switch (type(obj)) {
			case LUA_TUSERDATA:
			case LUA_TFUNCTION:
				ref = obj;
				break;
			case LUA_TNIL:
				ref.Clear();
				break;
			default:
				throw ScriptExn("Expected a constructor or function.");
		}
	}

	/**
	 * Wrap a native object in the wrapper.
	 * @param inside The native object to wrap.
	 * @return The wrapped object.  If no factory has been set or there was
	 *         an error while executing the factory, then a
	 *         new instance of the Outside class will be returned.
	 */
	std::shared_ptr<Outside> operator()(std::shared_ptr<Inside> inside) const
	{
		using namespace luabind;
		using namespace Util;

		if (!ref) {
			return std::make_shared<Outside>(inside);
		}

		lua_State *L = scripting->GetState();

		object obj(L, inside);
		std::shared_ptr<Outside> retv;

		ref.Push();
		obj.push(L);
		scripting->Invoke(1, nullptr, [&](lua_State *L, int num) {
			if (num < 1) {
				Log::Error("Expected a return value.");
			}
			else {
				object retObj(from_stack(L, -num));
				try {
					retv = object_cast<std::shared_ptr<Outside>>(retObj);
				}
				catch (cast_failed &ex)
				{
					Log::Error("Unexpected return value: %s", ex.what());
				}
			}
			lua_pop(L, num);
			return 0;
		});

		return retv ? retv : std::make_shared<Outside>(inside);
	}

private:
	Script::Core *scripting;
	RegistryRef ref;
};

}  // namespace Script
}  // namespace HoverRace
