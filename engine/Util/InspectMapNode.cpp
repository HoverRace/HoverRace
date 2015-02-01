
// InspectMapNode.cpp
//
// Copyright (c) 2010, 2015 Michael Imamura.
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

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"

#include "InspectMapNode.h"
#include "Inspectable.h"

namespace HoverRace {
namespace Util {

/**
 * Render this node to an output stream.
 * @param os The output stream.
 * @throw std::exception An error occurred.
 */
void InspectMapNode::RenderToStream(std::ostream &os)
{
	try {
		yaml::Emitter emitter(os, false);
		RenderToYaml(emitter);
	}
	catch (yaml::EmitterExn &ex) {
		throw std::exception(ex);
	}
}

/**
 * Render this node to a string.
 * @param s The output string.
 * @throw std::exception An error occurred.
 */
void InspectMapNode::RenderToString(std::string &s)
{
	try {
		yaml::Emitter emitter(s, false);
		RenderToYaml(emitter);
	}
	catch (yaml::EmitterExn &ex) {
		throw std::exception(ex);
	}
}

/**
 * Render this node to a YAML emitter.
 * @param emitter The emitter.
 */
void InspectMapNode::RenderToYaml(yaml::Emitter &emitter)
{
	emitter.StartMap();
	for (auto &ent : fields) {
		emitter.MapKey(ent.first);
		if (!ent.second) {
			emitter.Value("NULL");
		}
		else {
			ent.second->RenderToYaml(emitter);
		}
	}
	emitter.EndMap();
}

void InspectMapNode::AddStringField(
	const std::string &name,
	const std::string &value)
{
	fields.emplace_back(name, std::make_shared<InspectScalarNode>(value));
}

InspectMapNode &InspectMapNode::AddSubobject(
	const std::string &name,
	const Inspectable *obj)
{
	auto node = std::make_shared<InspectMapNode>();
	if (obj) {
		obj->Inspect(*node);
	}
	fields.emplace_back(name, node);
	return *this;
}

void InspectScalarNode::RenderToYaml(yaml::Emitter &emitter)
{
	emitter.Value(value);
}

void InspectSeqNode::RenderToYaml(yaml::Emitter &emitter)
{
	emitter.StartSeq();
	for (auto &field : fields) {
		if (!field) {
			emitter.Value("NULL");
		}
		else {
			field->RenderToYaml(emitter);
		}
	}
	emitter.EndSeq();
}

}  // namespace Util
}  // namespace HoverRace
