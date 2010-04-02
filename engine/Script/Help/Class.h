
// Class.h
// Help text for a class in the scripting API.
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

#include "Method.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace yaml {
	class MapNode;
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
		Class() { }
	public:
		Class(const std::string &name);
		~Class();

		void Load(yaml::MapNode *node);

	public:
		const std::string &GetName() const;

	public:
		typedef std::map<const std::string,MethodPtr> methods_t;
		const methods_t &GetMethods() const;

		void AddMethod(MethodPtr method);
		MethodPtr GetMethod(const std::string &methodName) const;

	private:
		std::string name;
		methods_t methods;
};
typedef boost::shared_ptr<Class> ClassPtr;

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
