
// Method.cpp
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

#include <boost/algorithm/string/trim.hpp>

#include "../../Util/yaml/MapNode.h"
#include "../../Util/yaml/ScalarNode.h"
#include "../../Util/yaml/SeqNode.h"

#include "Method.h"

using boost::algorithm::trim;
using namespace HoverRace::Util;

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

void Method::Load(yaml::MapNode *node)
{
	node->ReadString("brief", brief);
	trim(brief);
	node->ReadString("desc", desc);
	trim(desc);

	yaml::Node *sigNode = node->Get("sig");
	if (auto scalar = dynamic_cast<yaml::ScalarNode*>(sigNode)) {
		sigs.push_back(scalar->AsString());
	}
	else if (auto seq = dynamic_cast<yaml::SeqNode*>(sigNode)) {
		for (yaml::Node *seqNode : *seq) {
			if (auto seqScalar = dynamic_cast<yaml::ScalarNode*>(seqNode)) {
				sigs.push_back(seqScalar->AsString());
			}
		}
	}
}

void Method::SetBrief(const std::string &s)
{
	brief = s;
}

void Method::SetDesc(const std::string &s)
{
	desc = s;
}

}  // namespace Help
}  // namespace Script
}  // namespace HoverRace
