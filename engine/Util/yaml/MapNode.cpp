
// yaml/MapNode.cpp
// Wrapper for LibYAML mapping nodes.
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
#include "ScalarNode.h"

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
void MapNode::Init() const
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
Node *MapNode::Get(const std::string &key) const
{
	if (children == NULL) Init();

	children_t::iterator iter = children->find(key);
	return (iter == children->end()) ? NULL : iter->second;
}

/**
 * Retrieve an optional child node as a string.
 * If the child node does not exist or is not a scalar, then @p dest will not
 * be changed.
 * @param key The key to look up.
 * @param dest The destination string.
 */
void MapNode::ReadString(const std::string &key, std::string &dest) const
{
	if (children == NULL) Init();

	ScalarNode *scalar = dynamic_cast<ScalarNode*>(Get(key));
	if (scalar != NULL) dest = scalar->AsString();
}

MapNode::const_iterator MapNode::begin() const
{
	if (children == NULL) Init();
	return children->begin();
}

MapNode::iterator MapNode::begin()
{
	if (children == NULL) Init();
	return children->begin();
}

MapNode::const_iterator MapNode::end() const
{
	if (children == NULL) Init();
	return children->end();
}

MapNode::iterator MapNode::end()
{
	if (children == NULL) Init();
	return children->end();
}
