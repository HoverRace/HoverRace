
// Class.h
//
// Copyright (c) 2010, 2015-2016 Michael Imamura.
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

#include "Method.h"

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
	namespace Util {
		namespace yaml {
			class MapNode;
		}
	}
}

namespace HoverRace {
namespace Script {
namespace Help {

/**
 * API documentation for a Lua class.
 * @author Michael Imamura
 */
class MR_DllDeclare Class
{
private:
	Class() = delete;
public:
	Class(const std::string &name);

	void Load(Util::yaml::MapNode *node);

public:
	const std::string &GetName() const { return name; }

public:
	using methods_t = std::map<const std::string, std::shared_ptr<Method>>;
	const methods_t &GetMethods() const { return methods; }

	void AddMethod(std::shared_ptr<Method> method);
	std::shared_ptr<Method> GetMethod(const std::string &methodName) const;

private:
	std::string name;
	methods_t methods;
};
typedef std::shared_ptr<Class> ClassPtr;

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
