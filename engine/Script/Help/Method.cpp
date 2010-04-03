
// Method.cpp
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

#include "StdAfx.h"

#include <boost/foreach.hpp>

#include "../../Util/yaml/MapNode.h"
#include "../../Util/yaml/ScalarNode.h"
#include "../../Util/yaml/SeqNode.h"

#include "Method.h"

namespace HoverRace {
namespace Script {
namespace Help {

/**
 * Constructor.
 * @param name The name of the method.
 */
Method::Method(const std::string &name) :
	name(name)
{
}

Method::~Method()
{
}

void Method::Load(yaml::MapNode *node)
{
	node->ReadString("brief", brief);
	node->ReadString("desc", desc);
}

const std::string &Method::GetName() const
{
	return name;
}

void Method::SetBrief(const std::string &s)
{
	brief = s;
}

const std::string &Method::GetBrief() const
{
	return brief;
}

void Method::SetDesc(const std::string &s)
{
	desc = s;
}

const std::string &Method::GetDesc() const
{
	return desc;
}

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace
