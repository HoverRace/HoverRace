
// yaml/NodeFactory.cpp
// Generates wrapper instances for LibYAML nodes.
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
