
/* yaml/SeqNode.cpp
	Wrapper for LibYAML sequence nodes. */

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
void SeqNode::Init()
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

/**
 * Retrieve the child nodes.
 * @return A pointer to a vector of nodes (never NULL, may be empty).
 */
SeqNode::children_t *SeqNode::GetChildren()
{
	if (children == NULL) Init();

	return children;
}

