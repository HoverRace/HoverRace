
// Method.h
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
 * API documentation for a Lua method.
 * @author Michael Imamura
 */
class MR_DllDeclare Method
{
private:
	Method() = delete;
public:
	Method(const std::string &name);
	virtual ~Method() { }

	virtual void Load(Util::yaml::MapNode *node);

public:
	using sigs_t = std::vector<std::string>;
	using examples_t = std::vector<std::string>;
protected:
	void SetBrief(const std::string &s);
	void SetDesc(const std::string &s);
	sigs_t &GetSigs() { return sigs; }

public:
	const std::string &GetName() const { return name; }
	const std::string &GetBrief() const { return brief; }
	const std::string &GetDesc() const { return desc; }
	const sigs_t &GetSigs() const { return sigs; }

private:
	std::string name;
	std::string brief;
	std::string desc;
	sigs_t sigs;
	examples_t examples;
};
typedef std::shared_ptr<Method> MethodPtr;

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace

#undef MR_DllDeclare
