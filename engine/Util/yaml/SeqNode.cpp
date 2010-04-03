
// yaml/SeqNode.cpp
// Wrapper for LibYAML sequence nodes.
//
// Copyright (c) 2008, 2009 Michael Imamura.
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

#include "NodeFactory.h"

#include "SeqNode.h"

using namespace yaml;

/**
 * Constructor.
 * @param doc The containing document.
 * @param node The underlying LibYAML node (may not be NULL).
 */
SeqNode::SeqNode(yaml_document_t *doc, yaml_node_t *node) :
	SUPER(doc, node), children(NULL)
{
}

/// Destructor.
SeqNode::~SeqNode()
{
	if (children != NULL) {
		for (children_t::iterator iter = children->begin();
			iter != children->end(); ++iter)
		{
			delete *iter;
		}
		delete children;
	}
}

/// Lazy initialization of the child nodes.
void SeqNode::Init() const
{
	yaml_document_t *doc = GetDocument();
	yaml_node_t *node = GetNode();

	children = new children_t();
	
	// Note: This unfortunately relies on some implementation details of how
	//       LibYAML implements stacks, since there doesn't appear to be a 
	//       public API for iterating over the stack.
	for (yaml_node_item_t *item = node->data.sequence.items.start;
		item < node->data.sequence.items.top; ++item)
	{
		// Retrieve the node.
		yaml_node_t *valnode = yaml_document_get_node(doc, *item);
		if (valnode == NULL) continue;

		children->push_back(NodeFactory::MakeNode(doc, valnode));
	}
}

SeqNode::const_iterator SeqNode::begin() const
{
	if (children == NULL) Init();
	return children->begin();
}

SeqNode::iterator SeqNode::begin()
{
	if (children == NULL) Init();
	return children->begin();
}

SeqNode::const_iterator SeqNode::end() const
{
	if (children == NULL) Init();
	return children->end();
}

SeqNode::iterator SeqNode::end()
{
	if (children == NULL) Init();
	return children->end();
}
