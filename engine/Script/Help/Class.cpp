
// Class.cpp
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

#include "../../Util/yaml/MapNode.h"
#include "../../Util/yaml/ScalarNode.h"

#include "Event.h"

#include "Class.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Script {
namespace Help {

/**
 * Constructor.
 * @param name The name of the class.
 */
Class::Class(const std::string &name) :
	name(name)
{
}

void Class::Load(yaml::MapNode *node)
{
	for (const yaml::MapNode::value_type &ent : *node) {
		if (auto root = dynamic_cast<yaml::MapNode*>(ent.second)) {
			const std::string &methodName = ent.first;

			// Determine the type of method.
			std::string type;
			root->ReadString("type", type);
			std::shared_ptr<Method> method;
			if (type == "event") {
				method = std::make_shared<Event>(methodName);
			}
			else if (type == "method") {
				method = std::make_shared<Method>(methodName);
			}
			else {
				//TODO
				continue;
			}

			method->Load(root);
			AddMethod(method);
		}
	}
}

void Class::AddMethod(std::shared_ptr<Method> method)
{
	if (!method) return;

	methods.insert(methods_t::value_type(method->GetName(), method));
}

std::shared_ptr<Method> Class::GetMethod(const std::string &methodName) const
{
	methods_t::const_iterator iter = methods.find(methodName);
	return (iter == methods.end()) ? std::shared_ptr<Method>() : iter->second;
}

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace
