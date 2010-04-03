
// Method.h
// Help text for a method in the scripting API.
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
 * API documentation for a Lua method.
 * @author Michael Imamura
 */
class MR_DllDeclare Method
{
	private:
		Method() { }
	public:
		Method(const std::string &name);
		virtual ~Method();

		virtual void Load(yaml::MapNode *node);

	protected:
		void SetBrief(const std::string &s);
		void SetDesc(const std::string &s);

	public:
		const std::string &GetName() const;
		const std::string &GetBrief() const;
		const std::string &GetDesc() const;

	private:
		std::string name;
		std::string brief;
		std::string desc;
};
typedef boost::shared_ptr<Method> MethodPtr;

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
