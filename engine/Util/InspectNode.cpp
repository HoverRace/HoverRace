
// InspectNode.h
// Inspection state..
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

#include "StdAfx.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "yaml/Emitter.h"
#include "yaml/MapNode.h"
#include "yaml/ScalarNode.h"

#include "InspectNode.h"

namespace HoverRace {
namespace Util {

InspectNode::InspectNode()
{
}

InspectNode::~InspectNode()
{
}

/**
 * Render this node to an output stream.
 * @param os The output stream.
 * @throw std::exception An error occurred.
 */
void InspectNode::RenderToStream(std::ostream &os)
{
	try {
		yaml::Emitter emitter(os);
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
void InspectNode::RenderToYaml(yaml::Emitter &emitter)
{
	emitter.StartMap();
	BOOST_FOREACH(fields_t::value_type &ent, fields) {
		emitter.MapKey(ent.first);
		emitter.Value(ent.second);
	}
	BOOST_FOREACH(subobjects_t::value_type &ent, subobjects) {
		emitter.MapKey(ent.first);
		if (ent.second.get() == NULL) {
			emitter.Value("NULL");
		}
		else {
			ent.second->RenderToYaml(emitter);
		}
	}
	emitter.EndMap();
}

void InspectNode::AddStringField(const std::string &name, const std::string &value)
{
	fields.insert(fields_t::value_type(name, value));
}

InspectNode &InspectNode::AddSubobject(const std::string &name, const Inspectable *obj)
{
	InspectNodePtr node = boost::make_shared<InspectNode>();
	if (obj != NULL)
		obj->Inspect(*node);
	subobjects.insert(subobjects_t::value_type(name, node));
	return *this;
}

}  // namespace Util
}  // namespace HoverRace
