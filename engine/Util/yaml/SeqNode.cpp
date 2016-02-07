
// SeqNode.cpp
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

#include "NodeFactory.h"

#include "SeqNode.h"

namespace HoverRace {
namespace Util {
namespace yaml {

/**
 * Constructor.
 * @param doc The containing document.
 * @param node The underlying LibYAML node (may not be nullptr).
 */
SeqNode::SeqNode(yaml_document_t *doc, yaml_node_t *node) :
	SUPER(doc, node)
{
}

/// Destructor.
SeqNode::~SeqNode()
{
	if (children) {
		for (children_t::iterator iter = children->begin();
			iter != children->end(); ++iter)
		{
			delete *iter;
		}
	}
}

/// Lazy initialization of the child nodes.
void SeqNode::Init() const
{
	yaml_document_t *doc = GetDocument();
	yaml_node_t *node = GetNode();

	children.reset(new children_t());

	// Note: This unfortunately relies on some implementation details of how
	//       LibYAML implements stacks, since there doesn't appear to be a
	//       public API for iterating over the stack.
	for (yaml_node_item_t *item = node->data.sequence.items.start;
		item < node->data.sequence.items.top; ++item)
	{
		// Retrieve the node.
		yaml_node_t *valnode = yaml_document_get_node(doc, *item);
		if (!valnode) continue;

		children->push_back(NodeFactory::MakeNode(doc, valnode));
	}
}

size_t SeqNode::Size() const
{
	if (!children) Init();
	return children->size();
}

SeqNode::const_iterator SeqNode::begin() const
{
	if (!children) Init();
	return children->begin();
}

SeqNode::iterator SeqNode::begin()
{
	if (!children) Init();
	return children->begin();
}

SeqNode::const_iterator SeqNode::end() const
{
	if (!children) Init();
	return children->end();
}

SeqNode::iterator SeqNode::end()
{
	if (!children) Init();
	return children->end();
}

}  // namespace yaml
}  // namespace Util
}  // namespace HoverRace
