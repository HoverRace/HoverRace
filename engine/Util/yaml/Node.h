
/* yaml/Node.h
	Header for yaml::Node. */

#pragma once

#include <yaml.h>

namespace yaml
{
	/// Base class for LibYAML document nodes.
	class Node
	{
		private:
			Node() : doc(NULL), node(NULL) { }
		public:
			Node(yaml_document_t *doc, yaml_node_t *node) :
				doc(doc), node(node) { };
			virtual ~Node() { }

		protected:
			yaml_document_t *GetDocument() const { return doc; }
			yaml_node_t *GetNode() const { return node; }

		private:
			yaml_document_t *doc;
			yaml_node_t *node;
	};
}
