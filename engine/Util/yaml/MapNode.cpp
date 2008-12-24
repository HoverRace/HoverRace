
/* yaml/MapNode.cpp
	Wrapper for LibYAML mapping nodes. */

#include "StdAfx.h"

#include "NodeFactory.h"

#include "MapNode.h"

using namespace yaml;

/**
 * Constructor.
 * @param doc The containing document.
 * @param node The underlying LibYAML node (may not be NULL).
 */
MapNode::MapNode(yaml_document_t *doc, yaml_node_t *node) :
	SUPER(doc, node), children(NULL)
{
}

/// Destructor.
MapNode::~MapNode()
{
	if (children != NULL) {
		for (children_t::iterator iter = children->begin();
			iter != children->end(); ++iter)
		{
			delete iter->second;
		}
		delete children;
	}
}

/// Lazy initialization of the child nodes.
void MapNode::Init()
{
	yaml_document_t *doc = GetDocument();
	yaml_node_t *node = GetNode();

	children = new children_t();
	
	// Note: This unfortunately relies on some implementation details of how
	//       LibYAML implements stacks, since there doesn't appear to be a 
	//       public API for iterating over the stack.
	for (yaml_node_pair_t *pair = node->data.mapping.pairs.start;
		pair < node->data.mapping.pairs.top; ++pair)
	{
		// Retrieve the key string.
		yaml_node_t *keynode =
			yaml_document_get_node(doc, pair->key);
		if (keynode == NULL) continue;
		if (keynode->type != YAML_SCALAR_NODE) continue;
		std::string key((char*)keynode->data.scalar.value);

		// Retrieve the value node.
		yaml_node_t *valnode =
			yaml_document_get_node(doc, pair->value);
		if (valnode == NULL) continue;

		children->insert(children_t::value_type(
			key, NodeFactory::MakeNode(doc, valnode)));
	}
}

/**
 * Retrieve the child node for a given key.
 * @param key The key to look up.
 * @return A Node instance or NULL if the key was not found.
 */
Node *MapNode::Get(const std::string &key)
{
	if (children == NULL) Init();

	children_t::iterator iter = children->find(key);
	return (iter == children->end()) ? NULL : iter->second;
}
