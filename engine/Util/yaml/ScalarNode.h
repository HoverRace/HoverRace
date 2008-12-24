
/* yaml/ScalarNode.h
	Header for yaml::ScalarNode. */

#pragma once

#include <limits.h>

#include "Node.h"

namespace yaml
{
	/// Wrapper for LibYAML scalar nodes.
	class ScalarNode : public Node
	{
		typedef Node SUPER;

		public:
			ScalarNode(yaml_document_t *doc, yaml_node_t *node);
			virtual ~ScalarNode();

		public:
			std::string AsString() const;
			bool AsBool(bool def) const;
			double AsDouble(double def, double min, double max) const;
			float AsFloat(float def, float min, float max) const;
			int AsInt(int def, int min=INT_MIN, int max=INT_MAX) const;

		private:
			char *value;
	};
}
