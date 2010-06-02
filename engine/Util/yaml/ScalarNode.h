
// yaml/ScalarNode.h
// Header for yaml::ScalarNode.
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

#pragma once

#include <limits.h>

#include "../OS.h"

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
			HoverRace::Util::OS::path_t AsPath() const;

		private:
			char *value;
	};
}
