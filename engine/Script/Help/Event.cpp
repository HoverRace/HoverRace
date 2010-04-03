
// Event.cpp
// Help text for an event method in the scripting API.
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

#include "../../Util/yaml/MapNode.h"
#include "../../Util/yaml/ScalarNode.h"
#include "../../Util/yaml/SeqNode.h"

#include "Event.h"

using boost::str;
using boost::format;

namespace HoverRace {
namespace Script {
namespace Help {

/**
 * Constructor.
 * @param name The name of the method.
 */
Event::Event(const std::string &name) :
	SUPER(name)
{
}

Event::~Event()
{
}

void Event::Load(yaml::MapNode *node)
{
	SUPER::Load(node);

	sigs_t &sigs = GetSigs();
	std::string base(GetName());
	sigs.push_back(base + "(function)        -- Add event handler");
	sigs.push_back(base + "(name, function)  -- Add/modify named handler");
	sigs.push_back(base + "(name, nil)       -- Remove named handler");
}

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace
