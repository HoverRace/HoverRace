
// SeqNode.h
//
// Copyright (c) 2008, 2009, 2015-2016 Michael Imamura.
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

#include "Node.h"

#include <vector>

namespace HoverRace {
namespace Util {
namespace yaml {

class SeqNode : public Node
{
	using SUPER = Node;

public:
	SeqNode(yaml_document_t *doc, yaml_node_t *node);
	virtual ~SeqNode();

public:
	bool IsEmpty() const { return !children || children->empty(); }

	size_t Size() const { return children ? children->size() : 0; }

private:
	void Init() const;

	using children_t = std::vector<Node*>;

public:
	// STL-like iteration, so we can use range-for on the node itself.
	using iterator = children_t::iterator;
	using const_iterator = children_t::const_iterator;
	using value_type = children_t::value_type;
	const_iterator begin() const;
	iterator begin();
	const_iterator end() const;
	iterator end();

private:
	mutable children_t *children;
};

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace
