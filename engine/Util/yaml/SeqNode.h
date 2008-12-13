
/* yaml/SeqNode.h
	Header for yaml::SeqNode. */

#pragma once

#include "Node.h"

#include <vector>

namespace yaml
{
	class SeqNode : public Node
	{
		typedef Node SUPER;

		public:
			SeqNode(yaml_document_t *doc, yaml_node_t *node);
			virtual ~SeqNode();

		private:
			void Init();

		public:
			typedef std::vector<Node*> children_t;
			children_t *GetChildren();
			
		private:
			children_t *children;
	};
}
