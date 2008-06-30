
/* yaml/NodeFactory.h
	Header for yaml::NodeFactory. */

#pragma once

#include "Node.h"

namespace yaml
{
	/// Generates the appropriate wrapper for LibYAML nodes.
	class NodeFactory
	{
		public:
			static Node *MakeNode(yaml_document_t *doc, yaml_node_t *node);
	};
}
