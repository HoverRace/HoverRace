
// yaml/Node.h
// Header for yaml::Node.
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

#include <yaml.h>
#include <string>

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
