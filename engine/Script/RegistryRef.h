
// RegistryRef.h
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
		class Core;
	}
}

namespace HoverRace {
namespace Script {

/**
 * Encapsulates a ref to the registry.
 *
 * This handles copy and move operations.
 *
 * @author Michael Imamura
 */
class MR_DllDeclare RegistryRef
{
public:
	constexpr RegistryRef(Core *scripting) noexcept :
		scripting(scripting), ref(LUA_NOREF) { }
	RegistryRef(const RegistryRef &o) noexcept;
	RegistryRef(RegistryRef &&o) noexcept :
		scripting(o.scripting), ref(o.ref)
	{
		o.ref = LUA_NOREF;
	}

	~RegistryRef()
	{
		Clear();
	}

	RegistryRef &operator=(const RegistryRef &o) noexcept;
	RegistryRef &operator=(RegistryRef &&o) noexcept
	{
		scripting = o.scripting;
		ref = o.ref;
		o.ref = LUA_NOREF;
		return *this;
	}

	RegistryRef &operator=(const luabind::object &obj)
	{
		Set(obj);
		return *this;
	}

	constexpr operator bool() const noexcept
	{
		return ref != LUA_NOREF && ref != LUA_REFNIL;
	}

public:
	constexpr Core *GetScripting() const noexcept { return scripting; }

public:
	void Clear() noexcept;

	void SetFromStack() noexcept;
	void Set(const luabind::object &obj);

	void Push() const noexcept;

private:
	Core *scripting;
	int ref;
};

}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
