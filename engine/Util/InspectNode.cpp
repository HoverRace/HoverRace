
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

#include <boost/make_shared.hpp>

#include "InspectNode.h"

namespace HoverRace {
namespace Util {

InspectNode::InspectNode()
{
}

InspectNode::~InspectNode()
{
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
