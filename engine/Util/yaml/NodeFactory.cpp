
/* yaml/NodeFactory.cpp
	Generates wrapper instances for LibYAML nodes. */

#include "stdafx.h"

#include "Node.h"
#include "MapNode.h"
#include "ScalarNode.h"
#include "SeqNode.h"

#include "NodeFactory.h"

using namespace yaml;

/**
 * Wrap a LibYAML node in a new yaml::Node instance.
 * @param doc The LibYAML document which contains this node.
 * @param node The LibYAML node (may not be NULL).
 * @return A new instance (never NULL).
 *         It is up to the caller to delete this instance.
 */
Node *NodeFactory::MakeNode(yaml_document_t *doc, yaml_node_t *node)
{
	switch (node->type)
	{
		case YAML_SCALAR_NODE:
			return new ScalarNode(doc, node);
		case YAML_MAPPING_NODE:
			return new MapNode(doc, node);
		case YAML_SEQUENCE_NODE:
			return new SeqNode(doc, node);
		default:
			return new Node(doc, node);
	}
}
