
/* yaml/MapNode.h
	Header for yaml::MapNode. */

#pragma once

#include <map>

#include "Node.h"

namespace yaml
{
	class MapNode : public Node
	{
		typedef Node SUPER;

		public:
			MapNode(yaml_document_t *doc, yaml_node_t *node);
			virtual ~MapNode();

		private:
			void Init();

		public:
			Node *Get(const std::string &key);

		private:
			typedef std::map<std::string,Node*> children_t;
			children_t *children;
	};
}
